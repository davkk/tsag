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
void tag_vec_replace(TagVec* vec, size_t idx, const Tag* tag);
void tag_dedup_init(TagDedup* d);

// Reserve a slot for (start, end, pattern) without allocating strings.
// Returns 0 if this match loses and must be skipped, 1 if it survives.
// On survive, *slot is vec_size (push) or the existing index (replace;
// stored pattern already updated). Push path must call tag_dedup_track
// after a successful push to record the new key.
int tag_dedup_claim(TagDedup* d, uint32_t start, uint32_t end, uint32_t pattern, size_t vec_size, size_t* slot);
void tag_dedup_track(TagDedup* d, uint32_t start, uint32_t end, uint32_t pattern, size_t idx);
void tag_vec_sort(TagVec* vec);
void tag_vec_free(TagVec* vec);

#endif
