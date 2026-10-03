#ifndef MERGE_H
#define MERGE_H

#include <stdio.h>

#include "ioqueue.h"

typedef struct {
  IoQueue* outq;
  int n;
  FILE* out;
  const char* out_path;  // NULL when writing to stdout ("-o -")
  const char* cache_dir; // spill runs live here
} MergeArg;

void* merge(void* arg);

#endif
