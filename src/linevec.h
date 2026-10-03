#ifndef LINEVEC_H
#define LINEVEC_H

#include <stddef.h>
#include <stdint.h>

// One owned "name\tfile\t/^pattern$/;"\tkind" line per tag; nothing borrowed.
typedef struct {
  char** items; // owned lines
  size_t len;
  size_t cap;
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
int line_vec_push(LineVec* vec, char* line);
void line_vec_replace(LineVec* vec, size_t idx, char* line);
void line_vec_sort(LineVec* vec);
void line_vec_uniq(LineVec* vec);
void line_vec_clear(LineVec* vec);
void line_vec_free(LineVec* vec);

void dedup_init(Dedup* d);

// 0 = drop, 1 = survive (*slot set); pushes must dedup_track afterwards.
int dedup_claim(Dedup* d, uint32_t start, uint32_t end, uint32_t pattern, size_t vec_len, size_t* slot);
void dedup_track(Dedup* d, uint32_t start, uint32_t end, uint32_t pattern, size_t idx);

#endif
