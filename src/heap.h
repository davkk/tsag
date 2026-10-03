#ifndef TSAG_HEAP_H
#define TSAG_HEAP_H

#include <stdbool.h>
#include <stddef.h>

#include "tagvec.h"

typedef struct {
  size_t batch;
  size_t idx;
} HeapEntry;

void heap_push(HeapEntry heap[], size_t* size, HeapEntry entry, TagVec** batches);
bool heap_pop(HeapEntry heap[], size_t* size, HeapEntry* out, TagVec** batches);

#endif
