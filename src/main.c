#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "discover.h"
#include "ioqueue.h"
#include "lang.h"
#include "merge.h"
#include "options.h"
#include "worker.h"

#define DEFAULT_OUTPUT_FILEPATH "tags"

int main(int argc, char** argv) {
  Options opts = {0};
  if (parse_options(argc, argv, &opts) != 0) {
    return 2;
  }

  char* parsers_dir = getenv("TSAG_PARSERS");
  if (!parsers_dir) {
    const char* base_dir = getenv("XDG_DATA_HOME");
    assert(base_dir);
    static char buf[PATH_MAX];
    snprintf(buf, sizeof(buf), "%s/tsag", base_dir);
    parsers_dir = buf;
  }
  assert(parsers_dir);

  LangCache* lang_cache = lang_cache_new(parsers_dir);
  if (!lang_cache) {
    fprintf(stderr, "lang_cache init failed\n");
    return 1;
  }

  long jobs = opts.jobs;
  if (jobs <= 0) {
    jobs = sysconf(_SC_NPROCESSORS_ONLN) - 2;
    if (jobs < 1) jobs = 1;
  }

  FILE* out;
  if (opts.output && strcmp(opts.output, "-") == 0) {
    out = stdout;
  } else {
    out = fopen(opts.output ? opts.output : DEFAULT_OUTPUT_FILEPATH, "w");
  }
  if (!out) {
    fprintf(stderr, "failed to open output '%s': %s\n", opts.output, strerror(errno));
    lang_cache_free(lang_cache);
    return 1;
  }

  IoQueue* queue = io_queue_new(QUEUE_CAPACITY);
  if (!queue) {
    fprintf(stderr, "io queue init failed\n");
    if (out != stdout) fclose(out);
    lang_cache_free(lang_cache);
    return 1;
  }

  IoQueue* out_queue = io_queue_new((size_t)jobs);
  if (!out_queue) {
    fprintf(stderr, "io out queue init failed\n");
    io_queue_free(queue);
    if (out != stdout) fclose(out);
    lang_cache_free(lang_cache);
    return 1;
  }

  pthread_t* threads = malloc((size_t)jobs * sizeof(*threads));
  if (!threads) {
    fprintf(stderr, "out of memory\n");
    io_queue_free(out_queue);
    io_queue_free(queue);
    if (out != stdout) fclose(out);
    lang_cache_free(lang_cache);
    return 1;
  }

  WorkerArg worker_arg = {queue, out_queue, lang_cache};

  for (long i = 0; i < jobs; i++) {
    int rc = pthread_create(&threads[i], NULL, worker, &worker_arg);
    if (rc) {
      fprintf(stderr, "Failed to create thread %ld\n", i);
      io_queue_close(queue);
      io_queue_close(out_queue);
      free(threads);
      io_queue_free(out_queue);
      io_queue_free(queue);
      if (out != stdout) fclose(out);
      lang_cache_free(lang_cache);
      return 1;
    }
  }

  MergeArg merge_arg = {out_queue, (int)jobs, out};
  pthread_t merge_thread;
  int merge_rc = pthread_create(&merge_thread, NULL, merge, &merge_arg);
  if (merge_rc) {
    fprintf(stderr, "Failed to create merge thread\n");
    io_queue_close(queue);
    for (long i = 0; i < jobs; i++) {
      pthread_join(threads[i], NULL);
    }
    io_queue_close(out_queue);
    free(threads);
    io_queue_free(out_queue);
    io_queue_free(queue);
    if (out != stdout) fclose(out);
    lang_cache_free(lang_cache);
    return 1;
  }

  if (opts.path_count == 0) {
    enqueue_path(queue, ".");
  } else {
    for (int i = 0; i < opts.path_count; i++) {
      enqueue_path(queue, opts.paths[i]);
    }
  }
  io_queue_close(queue);

  for (long i = 0; i < jobs; i++) {
    int rc = pthread_join(threads[i], NULL);
    if (rc) {
      fprintf(stderr, "Failed to join thread %ld\n", i);
      return 1;
    }
  }
  io_queue_close(out_queue);
  pthread_join(merge_thread, NULL);

  free(threads);
  io_queue_free(out_queue);
  io_queue_free(queue);
  lang_cache_free(lang_cache);
  if (out != stdout) {
    if (fclose(out) != 0) {
      fprintf(stderr, "failed to close output '%s': %s\n", opts.output, strerror(errno));
      return 1;
    }
  }
  return 0;
}
