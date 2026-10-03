#ifndef TSAG_HEAP_H
#define TSAG_HEAP_H

#include <stdbool.h>
#include <stddef.h>

#include "linevec.h"

typedef struct {
  size_t batch;
  size_t idx;
} HeapEntry;

void heap_push(HeapEntry heap[], size_t* size, HeapEntry entry, LineVec** batches);
bool heap_pop(HeapEntry heap[], size_t* size, HeapEntry* out, LineVec** batches);

#endif
