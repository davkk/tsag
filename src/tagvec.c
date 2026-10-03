#include "tagvec.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

TagVec* tag_vec_new(size_t cap) {
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
  vec->path_cap = 64;
  vec->paths = calloc(vec->path_cap, sizeof(char*));
  if (!vec->paths) {
    free(vec->tags);
    free(vec);
    return NULL;
  }
  return vec;
}

void tag_vec_add_path(TagVec* vec, char* path) {
  if (vec->path_count == vec->path_cap) {
    vec->path_cap *= 2;
    vec->paths = realloc(vec->paths, vec->path_cap * sizeof(char*));
  }
  vec->paths[vec->path_count++] = path;
}

void tag_vec_push(TagVec* vec, const Tag* tag) {
  assert(vec && "vec is NULL");
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    vec->tags = realloc(vec->tags, vec->capacity * sizeof(Tag));
  }
  vec->tags[vec->size++] = *tag;
}

void tag_dedup_init(TagDedup* d) {
  d->count = 0;
}

int tag_vec_upsert(TagVec* vec, TagDedup* d, Tag* tag, uint32_t start, uint32_t end,
                   uint32_t pattern) {
  size_t slot = vec->size;
  for (size_t k = 0; k < d->count; k++) {
    if (d->entries[k].start == start && d->entries[k].end == end) {
      if (pattern <= d->entries[k].pattern) {
        free(tag->name);
        free(tag->pattern);
        return 0;
      }
      slot = d->entries[k].idx;
      d->entries[k].pattern = pattern;
      break;
    }
  }

  if (slot == vec->size) {
    if (d->count < TAG_DEDUP_CAP) {
      d->entries[d->count++] = (TagSeen){start, end, pattern, vec->size};
    }
    tag_vec_push(vec, tag);
    return 1;
  }

  free(vec->tags[slot].name);
  free(vec->tags[slot].pattern);
  vec->tags[slot] = *tag;
  return 1;
}

static int compare_tags(const void* a, const void* b) {
  const Tag* ta = a;
  const Tag* tb = b;
  int cmp = strcmp(ta->name, tb->name);
  if (cmp != 0) return cmp;
  return strcmp(ta->file, tb->file);
}

void tag_vec_sort(TagVec* vec) {
  assert(vec && "vec is NULL");
  qsort(vec->tags, vec->size, sizeof(Tag), compare_tags);
}

void tag_vec_free(TagVec* vec) {
  assert(vec && "vec is NULL");
  for (size_t i = 0; i < vec->size; i++) {
    free(vec->tags[i].name);
    free(vec->tags[i].pattern);
  }
  for (size_t i = 0; i < vec->path_count; i++) {
    free(vec->paths[i]);
  }
  free(vec->paths);
  free(vec->tags);
  free(vec);
}
