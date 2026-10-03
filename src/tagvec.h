#ifndef TAGVEC_H
#define TAGVEC_H

#include <stddef.h>
#include <stdint.h>

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
  char** paths;
  size_t path_count;
  size_t path_cap;
} TagVec;

// Per-file dedup: same byte range keeps the higher pattern_index.
#define TAG_DEDUP_CAP 1024

typedef struct {
  uint32_t start;
  uint32_t end;
  uint32_t pattern;
  size_t idx;
} TagSeen;

typedef struct {
  TagSeen entries[TAG_DEDUP_CAP];
  size_t count;
} TagDedup;

TagVec* tag_vec_new(size_t cap);
void tag_vec_add_path(TagVec* vec, char* path);
void tag_vec_push(TagVec* vec, const Tag* tag);
void tag_dedup_init(TagDedup* d);

// Insert tag, deduping on (start, end): higher pattern_index wins.
// Takes ownership of tag->name/pattern in all cases: pushes, replaces,
// or frees them when the incoming match loses. Returns 1 if kept, 0 if dropped.
int tag_vec_upsert(TagVec* vec, TagDedup* d, Tag* tag, uint32_t start, uint32_t end,
                   uint32_t pattern);
void tag_vec_sort(TagVec* vec);
void tag_vec_free(TagVec* vec);

#endif
