#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>
#include <unistd.h>

#include "ioqueue.h"
#include "lang.h"
#include "parse.h"
#include "tagvec.h"

typedef struct {
  IoQueue* q;
  LangCache* cache;
  int* exit_code;
} WorkerArg;

static void* worker(void* arg) {
  WorkerArg* a = (WorkerArg*)arg;
  TSParser* parser = ts_parser_new();
  if (!parser) {
    printf("Parser init failed\n");
    return NULL;
  }
  TSQueryCursor* cursor = ts_query_cursor_new();
  if (!cursor) {
    printf("Cursor init failed\n");
    ts_parser_delete(parser);
    return NULL;
  }

  TagVec* vec = tag_vec_new(128);

  const char* path;
  while ((path = io_queue_get(a->q)) != NULL)
    if (parse_file(path, a->cache, parser, cursor, vec) != 0) *a->exit_code = 1;

  tag_vec_sort(vec);
  for (size_t i = 0; i < vec->size; ++i) {
    Tag* tag = &vec->tags[i];
    printf("%s\t%s\t/^%s$/;\"\t%s\n", tag->name, tag->file, tag->pattern, tag->kind);
  }

  tag_vec_free(vec);
  ts_query_cursor_delete(cursor);
  ts_parser_delete(parser);
  return NULL;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
    return 1;
  }

  const char* dir = getenv("TSAG_GRAMMARS");
  if (!dir || !*dir) dir = "/home/davkk/.local/share/nvim/site/parser/";

  LangCache* cache = lang_cache_new(dir);
  if (!cache) {
    fprintf(stderr, "cache init failed\n");
    return 1;
  }

  IoQueue* queue = io_queue_new(argc - 1);
  if (!queue) {
    fprintf(stderr, "io queue init failed\n");
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    const char* filepath = argv[i];
    io_queue_put(queue, filepath);
  }
  io_queue_close(queue);

  int n = sysconf(_SC_NPROCESSORS_ONLN) - 1;
  if (n < 1) n = 1;

  pthread_t threads[n];
  int exit_code = 0;
  WorkerArg arg = {queue, cache, &exit_code};

  for (int i = 0; i < n; i++) {
    int rc = pthread_create(&threads[i], NULL, worker, &arg);
    if (rc) {
      fprintf(stderr, "Failed to create thread %d\n", i);
      return 1;
    }
  }
  for (int i = 0; i < n; i++) {
    int rc = pthread_join(threads[i], NULL);
    if (rc) {
      fprintf(stderr, "Failed to join thread %d\n", i);
      return 1;
    }
  }

  io_queue_free(queue);
  lang_cache_free(cache);
  return exit_code;
}
