#ifndef WORKER_H
#define WORKER_H

#include "ioqueue.h"
#include "lang.h"

typedef struct {
  IoQueue* q;
  IoQueue* outq;
  LangCache* lang_cache;
  // Reserved for spill runs dir (XDG_CACHE_HOME/tsag/<project>).
  // See refactor.qf LineVec + sort -m plan; currently unused.
  char* cache_path;
} WorkerArg;

void* worker(void* arg);

#endif
