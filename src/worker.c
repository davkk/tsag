#include "worker.h"

#include <stdio.h>
#include <stdlib.h>
#include <tree_sitter/api.h>
#include <unistd.h>

#include "parse.h"
#include "spill.h"

void worker_out_free(WorkerOut* o) {
  if (!o) return;
  if (o->tail) line_vec_free(o->tail);
  for (size_t i = 0; i < o->n_runs; i++) {
    free(o->runs[i]);
  }
  free(o->runs);
  free(o);
}

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
  Spill spill;
  spill_init(&spill, a->cache_path, a->worker_id, spill_default_limit());

  char* path;
  while ((path = (char*)io_queue_get(a->q)) != NULL) {
    parse_file_lines(path, a->lang_cache, parser, cursor, vec, &spill);
    free(path); // lines copy what they need; nothing borrowed
  }

  WorkerOut* out = calloc(1, sizeof(WorkerOut));
  if (!out) {
    fprintf(stderr, "WorkerOut init failed\n");
    line_vec_free(vec);
    for (size_t i = 0; i < spill.n_runs; i++) {
      unlink(spill.runs[i]);
      free(spill.runs[i]);
    }
    spill_dispose(&spill);
    ts_query_cursor_delete(cursor);
    ts_parser_delete(parser);
    return NULL;
  }
  if (spill.n_runs > 0) {
    // Slow path armed: fold the tail in as the last run so the merge
    // sees only sorted run files. A failed tail write falls back to
    // sending the tail in memory alongside the runs.
    if (vec->len > 0 && spill_flush(&spill, vec)) {
      line_vec_free(vec);
      vec = NULL;
    }
  } else {
    line_vec_sort(vec);
    line_vec_uniq(vec);
  }
  out->tail = vec;
  // Ownership of the run path array moves to WorkerOut; detach it before
  // dispose (dispose frees the array it still holds).
  out->runs = spill.runs;
  out->n_runs = spill.n_runs;
  spill.runs = NULL;
  spill.n_runs = 0;
  spill_dispose(&spill);
  io_queue_put(a->outq, out);

  ts_query_cursor_delete(cursor);
  ts_parser_delete(parser);
  return NULL;
}
