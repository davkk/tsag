#ifndef STRSTACK_H
#define STRSTACK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
  char** items;
  size_t len;
  size_t cap;
} StrStack;

static inline void strstack_init(StrStack* s) {
  s->items = NULL;
  s->len = 0;
  s->cap = 0;
}

static inline void strstack_free(StrStack* s) {
  free(s->items);
  s->items = NULL;
  s->len = 0;
  s->cap = 0;
}

static inline bool strstack_empty(const StrStack* s) {
  return s->len == 0;
}

static inline int strstack_push(StrStack* s, char* str) {
  if (s->len == s->cap) {
    size_t ncap = s->cap ? s->cap * 2 : 256;
    char** nitems = realloc(s->items, ncap * sizeof(*nitems));
    if (!nitems) return -1;
    s->items = nitems;
    s->cap = ncap;
  }
  s->items[s->len++] = str;
  return 0;
}

static inline char* strstack_pop(StrStack* s) {
  if (s->len == 0) return NULL;
  return s->items[--s->len];
}

#endif
