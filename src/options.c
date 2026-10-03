#include "options.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

void usage(FILE* f) {
  fprintf(f, "Usage: tsag [options] [path...]\n"
             "\n"
             "Options:\n"
             "  -o FILE    write tags to FILE\n"
             "  -j N       worker thread count (default: cores-2, min 1)\n"
             "  -h         show this help and exit\n");
}

int parse_options(int argc, char** argv, Options* o) {
  int c;
  while ((c = getopt(argc, argv, "o:j:h")) != -1) {
    switch (c) {
    case 'o':
      o->output = optarg;
      break;
    case 'j': {
      char* end = NULL;
      errno = 0;
      long v = strtol(optarg, &end, 10);
      if (errno != 0 || !end || *end != '\0' || v < 1) {
        fprintf(stderr, "invalid jobs value '%s': expected integer >= 1\n", optarg);
        return -1;
      }
      o->jobs = v;
      break;
    }
    case 'h':
      usage(stdout);
      exit(0);
    default:
      usage(stderr);
      return -1;
    }
  }

  o->paths = &argv[optind];
  o->path_count = argc - optind;
  return 0;
}
