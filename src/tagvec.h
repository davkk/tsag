#ifndef TAGVEC_H
#define TAGVEC_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char* name; // owned
  const char* file;
  char* pattern; // owned
  const char* kind;
} Tag;

typedef struct {
  size_t size;
  size_t capacity;
  Tag* tags;
} TagVec;

static inline TagVec* tag_vec_new(size_t cap) {
  assert(cap > 0 && "capacity must be greater than 0");
  TagVec* vec = calloc(1, sizeof(TagVec));
  if (!vec) return NULL;
  vec->size = 0;
  vec->capacity = cap;
  vec->tags = calloc(cap, sizeof(Tag));
  if (!vec->tags) {
    free(vec);
    return NULL;
  }
  return vec;
}

static inline void tag_vec_push(TagVec* vec, const Tag* tag) {
  assert(vec && "vec is NULL");
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    vec->tags = realloc(vec->tags, vec->capacity * sizeof(Tag));
  }
  vec->tags[vec->size++] = *tag;
}

static int compare_tags(const void* a, const void* b) {
  const Tag* ta = a;
  const Tag* tb = b;
  int cmp = strcmp(ta->name, tb->name);
  return cmp || strcmp(ta->file, tb->file);
}

static inline void tag_vec_sort(TagVec* vec) {
  assert(vec && "vec is NULL");
  qsort(vec->tags, vec->size, sizeof(Tag), compare_tags);
}

static inline void tag_vec_free(TagVec* vec) {
  assert(vec && "vec is NULL");
  for (size_t i = 0; i < vec->size; i++) {
    free(vec->tags[i].name);
    free(vec->tags[i].pattern);
  }
  free(vec->tags);
  free(vec);
}

#endif
