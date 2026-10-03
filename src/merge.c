#include "merge.h"

#include <stdio.h>

#include "heap.h"
#include "linevec.h"

void* merge(void* arg) {
  MergeArg* a = (MergeArg*)arg;

  LineVec* batches[a->n];
  size_t batch_count = 0;

  LineVec* vec;
  while ((vec = (LineVec*)io_queue_get(a->outq)) != NULL) {
    batches[batch_count++] = vec;
  }

  // populate
  HeapEntry heap[a->n];
  size_t heap_size = 0;
  for (size_t i = 0; i < batch_count; ++i) {
    if (batches[i]->len > 0) {
      HeapEntry entry = {.batch = i, .idx = 0};
      heap_push(heap, &heap_size, entry, batches);
    }
  }

  // drain
  HeapEntry entry;
  while (heap_pop(heap, &heap_size, &entry, batches)) {
    fputs(batches[entry.batch]->items[entry.idx], a->out);
    if (entry.idx + 1 < batches[entry.batch]->len) {
      entry.idx++;
      heap_push(heap, &heap_size, entry, batches);
    }
  }

  for (size_t i = 0; i < batch_count; ++i) {
    line_vec_free(batches[i]);
  }

  return NULL;
}
