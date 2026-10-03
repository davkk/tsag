#ifndef WORKER_H
#define WORKER_H

#include "ioqueue.h"
#include "lang.h"
#include "linevec.h"

typedef struct {
  IoQueue* q;
  IoQueue* outq;
  LangCache* lang_cache;
  char* cache_path; // spill runs dir (XDG_CACHE_HOME/tsag/<project>)
  int worker_id;
} WorkerArg;

// What a worker hands to the merge: either an in-memory sorted tail
// (common case, nothing ever spilled) or run files on disk (tail
// included as the last run). Never both missing.
typedef struct {
  LineVec* tail; // NULL when fully spilled
  char** runs;   // owned run paths
  size_t n_runs;
} WorkerOut;

void worker_out_free(WorkerOut* o);

void* worker(void* arg);

#endif
