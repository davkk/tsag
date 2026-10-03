#include "worker.h"

#include <stdio.h>
#include <stdlib.h>
#include <tree_sitter/api.h>

#include "linevec.h"
#include "parse.h"

void* worker(void* arg) {
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

  LineVec* vec = line_vec_new(128);
  if (!vec) {
    fprintf(stderr, "LineVec init failed\n");
    char* path;
    while ((path = (char*)io_queue_get(a->q)) != NULL) free(path);
    ts_query_cursor_delete(cursor);
    ts_parser_delete(parser);
    return NULL;
  }

  char* path;
  while ((path = (char*)io_queue_get(a->q)) != NULL) {
    parse_file_lines(path, a->lang_cache, parser, cursor, vec);
    free(path); // lines copy what they need; nothing borrowed
  }

  line_vec_sort(vec);
  line_vec_uniq(vec);
  io_queue_put(a->outq, vec);

  ts_query_cursor_delete(cursor);
  ts_parser_delete(parser);
  return NULL;
}
