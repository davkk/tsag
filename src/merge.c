#include "merge.h"

#include <stdio.h>

#include "heap.h"
#include "tagvec.h"

void* merge(void* arg) {
  MergeArg* a = (MergeArg*)arg;

  TagVec* batches[a->n];
  size_t batch_count = 0;

  TagVec* vec;
  while ((vec = (TagVec*)io_queue_get(a->outq)) != NULL) {
    batches[batch_count++] = vec;
  }

  // populate
  HeapEntry heap[a->n];
  size_t heap_size = 0;
  for (size_t i = 0; i < batch_count; ++i) {
    if (batches[i]->size > 0) {
      HeapEntry entry = {.batch = i, .idx = 0};
      heap_push(heap, &heap_size, entry, batches);
    }
  }

  // drain
  HeapEntry entry;
  while (heap_pop(heap, &heap_size, &entry, batches)) {
    Tag* tag = &batches[entry.batch]->tags[entry.idx];
    fprintf(a->out, "%s\t%s\t/^%s$/;\"\t%s\n", tag->name, tag->file, tag->pattern, tag->kind);
    if (entry.idx + 1 < batches[entry.batch]->size) {
      entry.idx++;
      heap_push(heap, &heap_size, entry, batches);
    }
  }

  for (size_t i = 0; i < batch_count; ++i) {
    tag_vec_free(batches[i]);
  }

  return NULL;
}
