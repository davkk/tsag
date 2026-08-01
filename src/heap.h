#ifndef TSAG_HEAP_H
#define TSAG_HEAP_H

#include "tagvec.h"

typedef struct {
  size_t batch;
  size_t idx;
} HeapEntry;

static inline bool heap_less(TagVec** batches, const HeapEntry* a, const HeapEntry* b) {
  TagVec* ab = batches[a->batch];
  TagVec* bb = batches[b->batch];
  Tag* at = &ab->tags[a->idx];
  Tag* bt = &bb->tags[b->idx];
  int cmp = strcmp(at->name, bt->name);
  if (cmp != 0) return cmp < 0;
  return strcmp(at->file, bt->file) < 0;
}

static void heap_heapify_up(HeapEntry heap[], size_t idx, TagVec** batches) {
  if (idx == 0) return;
  size_t p = (idx - 1) / 2;
  if (heap_less(batches, &heap[idx], &heap[p])) {
    HeapEntry tmp = heap[p];
    heap[p] = heap[idx];
    heap[idx] = tmp;
    heap_heapify_up(heap, p, batches);
  }
}

static void heap_heapify_down(HeapEntry heap[], size_t idx, size_t* size, TagVec** batches) {
  size_t smallest = idx;
  size_t l = idx * 2 + 1;
  size_t r = idx * 2 + 2;
  if (l < *size && heap_less(batches, &heap[l], &heap[smallest])) smallest = l;
  if (r < *size && heap_less(batches, &heap[r], &heap[smallest])) smallest = r;
  if (smallest != idx) {
    HeapEntry tmp = heap[smallest];
    heap[smallest] = heap[idx];
    heap[idx] = tmp;
    heap_heapify_down(heap, smallest, size, batches);
  }
}

static inline void heap_push(HeapEntry heap[], size_t* size, HeapEntry entry, TagVec** batches) {
  heap[*size] = entry;
  heap_heapify_up(heap, *size, batches);
  (*size)++;
}

static inline bool heap_pop(HeapEntry heap[], size_t* size, HeapEntry* out, TagVec** batches) {
  if (*size == 0) return false;
  *out = heap[0];
  heap[0] = heap[*size - 1];
  (*size)--;
  heap_heapify_down(heap, 0, size, batches);
  return true;
}

#endif
