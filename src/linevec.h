#ifndef LINEVEC_H
#define LINEVEC_H

#include <stddef.h>
#include <stdint.h>

// A tag is a fully formatted, owned output line:
//   "name\tfile\t/^pattern$/;\"\tkind\n"
// Single ownership (free each line) replaces Tag's mixed owned/borrowed
// fields, so a vec can be sorted, passed across threads, and later
// flushed to a spill run file as-is.
typedef struct {
  char** items; // owned lines
  size_t len;
  size_t cap;
  size_t bytes; // sum of strlen(items), excl. NULs; for spill budgeting
} LineVec;

// Per-file dedup: same byte range keeps the higher pattern_index.
#define DEDUP_CAP 1024

typedef struct {
  uint32_t start;
  uint32_t end;
  uint32_t pattern;
  size_t idx;
} DedupEntry;

typedef struct {
  DedupEntry entries[DEDUP_CAP];
  size_t count;
} Dedup;

LineVec* line_vec_new(size_t cap);
void line_vec_push(LineVec* vec, char* line);
void line_vec_replace(LineVec* vec, size_t idx, char* line);
void line_vec_sort(LineVec* vec);
void line_vec_uniq(LineVec* vec);
void line_vec_clear(LineVec* vec);
void line_vec_free(LineVec* vec);

void dedup_init(Dedup* d);

// Reserve a slot for (start, end, pattern) without allocating strings.
// Returns 0 if this match loses and must be skipped, 1 if it survives.
// On survive, *slot is vec_len (push) or the existing index (replace;
// stored pattern already updated). Push path must call dedup_track
// after a successful push to record the new key.
int dedup_claim(Dedup* d, uint32_t start, uint32_t end, uint32_t pattern, size_t vec_len, size_t* slot);
void dedup_track(Dedup* d, uint32_t start, uint32_t end, uint32_t pattern, size_t idx);

#endif
