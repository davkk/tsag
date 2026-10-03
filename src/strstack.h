#ifndef STRSTACK_H
#define STRSTACK_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  char** items;
  size_t len;
  size_t cap;
} StrStack;

void strstack_init(StrStack* s);
void strstack_free(StrStack* s);
bool strstack_empty(const StrStack* s);
int strstack_push(StrStack* s, char* str);
char* strstack_pop(StrStack* s);

#endif
