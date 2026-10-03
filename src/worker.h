#ifndef WORKER_H
#define WORKER_H

#include "ioqueue.h"
#include "lang.h"

typedef struct {
  IoQueue* q;
  IoQueue* outq;
  LangCache* lang_cache;
  char* cache_path;
} WorkerArg;

void* worker(void* arg);

#endif
