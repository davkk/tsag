#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdio.h>

typedef struct {
  const char* output;
  long jobs; // 0 = auto
  char** paths;
  int path_count;
} Options;

void usage(FILE* f);
int parse_options(int argc, char** argv, Options* o);

#endif
