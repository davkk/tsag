#ifndef MERGE_H
#define MERGE_H

#include <stdio.h>

#include "ioqueue.h"

typedef struct {
  IoQueue* outq;
  int n;
  FILE* out;
} MergeArg;

void* merge(void* arg);

#endif
