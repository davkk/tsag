#include "merge.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "heap.h"
#include "linevec.h"
#include "spill.h"
#include "worker.h"

// Fast path: every worker sent an in-memory tail, nothing touched disk.
static void merge_tails(LineVec** tails, size_t n_tails, FILE* out) {
  if (n_tails == 0) return;
  HeapEntry* heap = malloc(n_tails * sizeof(*heap));
  if (!heap) {
    fprintf(stderr, "merge out of memory\n");
    return;
  }
  size_t heap_size = 0;
  for (size_t i = 0; i < n_tails; ++i) {
    if (tails[i]->len > 0) {
      heap_push(heap, &heap_size, (HeapEntry){.batch = i, .idx = 0}, tails);
    }
  }

  HeapEntry entry;
  while (heap_pop(heap, &heap_size, &entry, tails)) {
    fputs(tails[entry.batch]->items[entry.idx], out);
    if (entry.idx + 1 < tails[entry.batch]->len) {
      entry.idx++;
      heap_push(heap, &heap_size, entry, tails);
    }
  }
  free(heap);
}

// Slow path: at least one run file exists. Stream-merge pre-sorted runs
// with sort -m (no re-sort, no pipe loop: sort reads inputs itself).
// Returns 0 on success.
static int merge_runs(char** runs, size_t n_runs, FILE* out, const char* out_path, const char* cache_dir) {
  fprintf(stderr, "tsag: merging %zu spill runs with sort -m\n", n_runs);

  char tmp[PATH_MAX];
  tmp[0] = '\0';
  if (out_path) {
    int n = snprintf(tmp, sizeof(tmp), "%s.tmp.%ld", out_path, (long)getpid());
    if (n < 0 || (size_t)n >= sizeof(tmp)) {
      fprintf(stderr, "spill output temp path too long\n");
      return 1;
    }
  }

  pid_t pid = fork();
  if (pid == -1) {
    fprintf(stderr, "fork for sort -m: %s\n", strerror(errno));
    return 1;
  }

  if (pid == 0) { // child
    setenv("LC_ALL", "C", 1);
    if (!out_path) {
      if (out != stdout) {
        fflush(out);
        if (dup2(fileno(out), STDOUT_FILENO) == -1) _exit(127);
      }
    }
    size_t fixed = out_path ? 8 : 6; // sort -m -u -T dir [-o tmp] runs NULL
    char** argv = malloc((fixed + n_runs) * sizeof(*argv));
    if (!argv) _exit(127);
    size_t k = 0;
    argv[k++] = "sort";
    argv[k++] = "-m";
    argv[k++] = "-u";
    argv[k++] = "-T";
    argv[k++] = (char*)cache_dir;
    if (out_path) {
      argv[k++] = "-o";
      argv[k++] = tmp;
    }
    for (size_t i = 0; i < n_runs; i++) argv[k++] = runs[i];
    argv[k] = NULL;
    execvp("sort", argv);
    _exit(127); // exec failed
  }

  int status = 0;
  while (waitpid(pid, &status, 0) == -1 && errno == EINTR) {
  }
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    fprintf(stderr, "sort -m failed (status %d)\n", WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    if (tmp[0]) unlink(tmp);
    return 1;
  }
  if (out_path && rename(tmp, out_path) != 0) {
    fprintf(stderr, "rename spill output: %s\n", strerror(errno));
    unlink(tmp);
    return 1;
  }
  return 0;
}

void* merge(void* arg) {
  MergeArg* a = (MergeArg*)arg;

  WorkerOut* outs[a->n];
  size_t n_outs = 0;
  size_t total_runs = 0;

  WorkerOut* o;
  while ((o = (WorkerOut*)io_queue_get(a->outq)) != NULL) {
    total_runs += o->n_runs;
    outs[n_outs++] = o;
  }

  if (total_runs == 0) {
    LineVec* tails[a->n];
    size_t n_tails = 0;
    for (size_t i = 0; i < n_outs; ++i) {
      if (outs[i]->tail) tails[n_tails++] = outs[i]->tail;
    }
    merge_tails(tails, n_tails, a->out);
    for (size_t i = 0; i < n_outs; ++i) worker_out_free(outs[i]);
    return NULL;
  }

  // Collect every run path plus materialized tails. Run strings move
  // into runs[] (sources freed as an array right after); tails that
  // fail to materialize stay in memory and fold in after sort output.
  size_t cap = total_runs + n_outs;
  char** runs = malloc(cap * sizeof(*runs));
  size_t n_runs = 0;
  for (size_t i = 0; i < n_outs; ++i) {
    for (size_t r = 0; r < outs[i]->n_runs; r++) runs[n_runs++] = outs[i]->runs[r];
    free(outs[i]->runs);
    outs[i]->runs = NULL;
    outs[i]->n_runs = 0;
    if (outs[i]->tail && outs[i]->tail->len > 0) {
      char* p = spill_write_run(a->cache_dir, "tail", outs[i]->tail);
      if (p) {
        runs[n_runs++] = p;
        line_vec_free(outs[i]->tail);
        outs[i]->tail = NULL;
      }
      // On write failure the tail stays; merge_tails folds leftovers below.
    }
  }

  int rc = merge_runs(runs, n_runs, a->out, a->out_path, a->cache_dir);

  // Fold any unmaterialized tails (write failures only) in directly.
  // NOTE: these append after sort output, so order breaks — but only
  // when disk writes are already failing. Tags over ordering.
  LineVec* leftovers[a->n];
  size_t n_left = 0;
  for (size_t i = 0; i < n_outs; ++i) {
    if (outs[i]->tail && outs[i]->tail->len > 0) leftovers[n_left++] = outs[i]->tail;
  }
  if (n_left > 0) merge_tails(leftovers, n_left, a->out);

  for (size_t i = 0; i < n_runs; i++) {
    unlink(runs[i]); // FIXME: runs litter cache_path on kill/crash, as before
    free(runs[i]);
  }
  free(runs);
  for (size_t i = 0; i < n_outs; ++i) {
    if (outs[i]->tail) line_vec_free(outs[i]->tail);
    outs[i]->tail = NULL;
    worker_out_free(outs[i]);
  }
  if (rc != 0) fprintf(stderr, "tsag: merge failed, output may be incomplete\n");
  return NULL;
}
