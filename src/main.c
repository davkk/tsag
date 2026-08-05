#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <tree_sitter/api.h>
#include <unistd.h>

#include "heap.h"
#include "ioqueue.h"
#include "lang.h"
#include "parse.h"
#include "tagvec.h"

#define QUEUE_CAPACITY 256

typedef struct {
  IoQueue* q;
  IoQueue* outq;
  LangCache* cache;
} WorkerArg;

typedef struct {
  IoQueue* outq;
  int n;
} MergeArg;

static void* worker(void* arg) {
  WorkerArg* a = (WorkerArg*)arg;
  TSParser* parser = ts_parser_new();
  if (!parser) {
    fprintf(stderr, "Parser init failed\n");
    return NULL;
  }
  TSQueryCursor* cursor = ts_query_cursor_new();
  if (!cursor) {
    fprintf(stderr, "Cursor init failed\n");
    ts_parser_delete(parser);
    return NULL;
  }

  TagVec* vec = tag_vec_new(128);

  char* path;
  while ((path = (char*)io_queue_get(a->q)) != NULL) {
    parse_file(path, a->cache, parser, cursor, vec);
  }

  tag_vec_sort(vec);
  io_queue_put(a->outq, vec);

  ts_query_cursor_delete(cursor);
  ts_parser_delete(parser);
  return NULL;
}

static void* merge(void* arg) {
  MergeArg* a = (MergeArg*)arg;

  TagVec* batches[a->n];
  size_t batch_count = 0;

  TagVec* vec;
  while ((vec = (TagVec*)io_queue_get(a->outq)) != NULL) {
    batches[batch_count++] = vec;
  }

  // populate
  HeapEntry heap[a->n];
  size_t heap_size = 0;
  for (size_t i = 0; i < batch_count; ++i) {
    if (batches[i]->size > 0) {
      HeapEntry entry = {.batch = i, .idx = 0};
      heap_push(heap, &heap_size, entry, batches);
    }
  }

  // drain
  HeapEntry entry;
  while (heap_pop(heap, &heap_size, &entry, batches)) {
    Tag* tag = &batches[entry.batch]->tags[entry.idx];
    printf("%s\t%s\t/^%s$/;\"\t%s\n", tag->name, tag->file, tag->pattern, tag->kind);
    if (entry.idx + 1 < batches[entry.batch]->size) {
      entry.idx++;
      heap_push(heap, &heap_size, entry, batches);
    }
  }

  for (size_t i = 0; i < batch_count; ++i) {
    tag_vec_free(batches[i]);
  }

  return NULL;
}

static void enqueue_path(IoQueue* queue, const char* path) {
  struct stat info;
  if (lstat(path, &info) == -1) {
    fprintf(stderr, "failed to stat '%s': %s\n", path, strerror(errno));
    return;
  }
  if (S_ISDIR(info.st_mode)) {
    DIR* dir = opendir(path);
    if (dir == NULL) {
      fprintf(stderr, "failed to open directory: %s\n", path);
      return;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
        size_t len = strlen(path) + 1 + strlen(entry->d_name) + 1;
        char* fullpath = malloc(len);
        if (strcmp(path, ".") == 0) {
          strcpy(fullpath, entry->d_name);
        } else {
          snprintf(fullpath, len, "%s/%s", path, entry->d_name);
        }
        enqueue_path(queue, fullpath);
        free(fullpath);
      }
    }
    closedir(dir);
  } else if (S_ISREG(info.st_mode)) {
    io_queue_put(queue, strdup(path));
  }
}

int main(int argc, char** argv) {
  const char* grammars_dir = getenv("TSAG_GRAMMARS");
  if (!grammars_dir || !*grammars_dir) grammars_dir = "/home/davkk/.local/share/nvim/site/parser/";

  LangCache* cache = lang_cache_new(grammars_dir);
  if (!cache) {
    fprintf(stderr, "cache init failed\n");
    return 1;
  }

  int n = sysconf(_SC_NPROCESSORS_ONLN) - 2;
  if (n < 1) n = 1;

  IoQueue* queue = io_queue_new(QUEUE_CAPACITY);
  if (!queue) {
    fprintf(stderr, "io queue init failed\n");
    return 1;
  }

  IoQueue* out_queue = io_queue_new(n);
  if (!out_queue) {
    fprintf(stderr, "io out queue init failed\n");
    return 1;
  }

  pthread_t threads[n];
  WorkerArg arg = {queue, out_queue, cache};

  for (int i = 0; i < n; i++) {
    int rc = pthread_create(&threads[i], NULL, worker, &arg);
    if (rc) {
      fprintf(stderr, "Failed to create thread %d\n", i);
      return 1;
    }
  }

  if (argc == 1) {
    enqueue_path(queue, ".");
  } else {
    for (int i = 1; i < argc; i++) {
      enqueue_path(queue, argv[i]);
    }
  }
  io_queue_close(queue);

  MergeArg merge_arg = {out_queue, n};
  pthread_t merge_thread;
  pthread_create(&merge_thread, NULL, merge, &merge_arg);

  for (int i = 0; i < n; i++) {
    int rc = pthread_join(threads[i], NULL);
    if (rc) {
      fprintf(stderr, "Failed to join thread %d\n", i);
      return 1;
    }
  }
  io_queue_close(out_queue);
  pthread_join(merge_thread, NULL);

  io_queue_free(out_queue);
  io_queue_free(queue);
  lang_cache_free(cache);
  return 0;
}
