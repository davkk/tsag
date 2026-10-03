#include "worker.h"

#include <stdio.h>
#include <tree_sitter/api.h>

#include "parse.h"
#include "tagvec.h"

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

  TagVec* vec = tag_vec_new(128);

  char* path;
  while ((path = (char*)io_queue_get(a->q)) != NULL) {
    parse_file(path, a->lang_cache, parser, cursor, vec);
  }

  tag_vec_sort(vec);
  io_queue_put(a->outq, vec);

  ts_query_cursor_delete(cursor);
  ts_parser_delete(parser);
  return NULL;
}
