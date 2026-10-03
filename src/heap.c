#include "heap.h"

#include <string.h>

static bool heap_less(LineVec** batches, const HeapEntry* a, const HeapEntry* b) {
  return strcmp(batches[a->batch]->items[a->idx], batches[b->batch]->items[b->idx]) < 0;
}

static void heap_heapify_up(HeapEntry heap[], size_t idx, LineVec** batches) {
  if (idx == 0) return;
  size_t p = (idx - 1) / 2;
  if (heap_less(batches, &heap[idx], &heap[p])) {
    HeapEntry tmp = heap[p];
    heap[p] = heap[idx];
    heap[idx] = tmp;
    heap_heapify_up(heap, p, batches);
  }
}

static void heap_heapify_down(HeapEntry heap[], size_t idx, size_t* size, LineVec** batches) {
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

void heap_push(HeapEntry heap[], size_t* size, HeapEntry entry, LineVec** batches) {
  heap[*size] = entry;
  heap_heapify_up(heap, *size, batches);
  (*size)++;
}

bool heap_pop(HeapEntry heap[], size_t* size, HeapEntry* out, LineVec** batches) {
  if (*size == 0) return false;
  *out = heap[0];
  heap[0] = heap[*size - 1];
  (*size)--;
  heap_heapify_down(heap, 0, size, batches);
  return true;
}
