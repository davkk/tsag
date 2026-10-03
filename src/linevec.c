#include "linevec.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

LineVec* line_vec_new(size_t cap) {
  assert(cap > 0 && "capacity must be greater than 0");
  LineVec* vec = calloc(1, sizeof(LineVec));
  if (!vec) return NULL;
  vec->items = calloc(cap, sizeof(char*));
  if (!vec->items) {
    free(vec);
    return NULL;
  }
  vec->cap = cap;
  return vec;
}

int line_vec_push(LineVec* vec, char* line) {
  assert(vec && "vec is NULL");
  if (vec->len == vec->cap) {
    if (vec->cap > SIZE_MAX / (2 * sizeof(char*))) return 0;
    size_t new_cap = vec->cap * 2;
    char** items = realloc(vec->items, new_cap * sizeof(char*));
    if (!items) return 0;
    vec->items = items;
    vec->cap = new_cap;
  }
  vec->bytes += strlen(line);
  vec->items[vec->len++] = line;
  return 1;
}

void line_vec_replace(LineVec* vec, size_t idx, char* line) {
  assert(vec && "vec is NULL");
  assert(idx < vec->len && "replace index out of range");
  vec->bytes -= strlen(vec->items[idx]);
  free(vec->items[idx]);
  vec->bytes += strlen(line);
  vec->items[idx] = line;
}

static int compare_lines(const void* a, const void* b) {
  return strcmp(*(char* const*)a, *(char* const*)b);
}

void line_vec_sort(LineVec* vec) {
  assert(vec && "vec is NULL");
  qsort(vec->items, vec->len, sizeof(char*), compare_lines);
}

// Drop adjacent exact duplicates. Call after sort.
void line_vec_uniq(LineVec* vec) {
  assert(vec && "vec is NULL");
  size_t w = 0;
  for (size_t r = 0; r < vec->len; r++) {
    if (w > 0 && strcmp(vec->items[r], vec->items[w - 1]) == 0) {
      vec->bytes -= strlen(vec->items[r]);
      free(vec->items[r]);
      continue;
    }
    vec->items[w++] = vec->items[r];
  }
  vec->len = w;
}

void line_vec_clear(LineVec* vec) {
  assert(vec && "vec is NULL");
  for (size_t i = 0; i < vec->len; i++) {
    free(vec->items[i]);
  }
  vec->len = 0;
  vec->bytes = 0;
}

void line_vec_free(LineVec* vec) {
  assert(vec && "vec is NULL");
  line_vec_clear(vec);
  free(vec->items);
  free(vec);
}

void dedup_init(Dedup* d) {
  d->count = 0;
}

int dedup_claim(Dedup* d, uint32_t start, uint32_t end, uint32_t pattern, size_t vec_len, size_t* slot) {
  for (size_t k = 0; k < d->count; k++) {
    if (d->entries[k].start == start && d->entries[k].end == end) {
      if (pattern <= d->entries[k].pattern) return 0;
      d->entries[k].pattern = pattern;
      *slot = d->entries[k].idx;
      return 1;
    }
  }
  *slot = vec_len;
  return 1;
}

void dedup_track(Dedup* d, uint32_t start, uint32_t end, uint32_t pattern, size_t idx) {
  if (d->count < DEDUP_CAP) {
    d->entries[d->count++] = (DedupEntry){start, end, pattern, idx};
  }
}
