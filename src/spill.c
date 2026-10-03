#include "spill.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

size_t spill_default_limit(void) {
  const char* e = getenv("TSAG_SPILL_LIMIT");
  if (e && e[0]) {
    errno = 0;
    char* end = NULL;
    unsigned long long v = strtoull(e, &end, 10);
    if (errno == 0 && end && *end == '\0' && v > 0 && v <= SIZE_MAX) return (size_t)v;
    fprintf(stderr, "ignoring invalid TSAG_SPILL_LIMIT '%s'\n", e);
  }
  return SPILL_DEFAULT_LIMIT;
}

void spill_init(Spill* s, const char* dir, int worker_id, size_t limit) {
  s->dir = dir;
  s->worker_id = worker_id;
  s->seq = 0;
  s->runs = NULL;
  s->n_runs = 0;
  s->cap = 0;
  s->limit = limit > 0 ? limit : SPILL_DEFAULT_LIMIT;
}

// mkdir -p of exactly one level: main/creates the base, we own the leaf.
static bool ensure_dir(const char* dir) {
  if (mkdir(dir, 0755) == -1 && errno != EEXIST) {
    fprintf(stderr, "spill dir '%s' creation failed: %s\n", dir, strerror(errno));
    return false;
  }
  return true;
}

static bool record_run(Spill* s, char* path) {
  if (s->n_runs == s->cap) {
    size_t ncap = s->cap ? s->cap * 2 : 8;
    char** items = realloc(s->runs, ncap * sizeof(*items));
    if (!items) return false;
    s->runs = items;
    s->cap = ncap;
  }
  s->runs[s->n_runs++] = path;
  return true;
}

char* spill_write_run(const char* dir, const char* prefix, LineVec* vec) {
  line_vec_sort(vec);
  line_vec_uniq(vec);

  if (!ensure_dir(dir)) return NULL;

  char path[PATH_MAX];
  int n = snprintf(path, sizeof(path), "%s/%s-XXXXXX", dir, prefix);
  if (n < 0 || (size_t)n >= sizeof(path)) {
    fprintf(stderr, "spill path too long\n");
    return NULL;
  }
  int fd = mkstemp(path);
  if (fd == -1) {
    fprintf(stderr, "spill file creation failed: %s\n", strerror(errno));
    return NULL;
  }
  FILE* f = fdopen(fd, "w");
  if (!f) {
    fprintf(stderr, "spill file open failed: %s\n", strerror(errno));
    close(fd);
    unlink(path);
    return NULL;
  }
  for (size_t i = 0; i < vec->len; i++) {
    if (fputs(vec->items[i], f) == EOF) {
      fprintf(stderr, "spill write failed: %s\n", strerror(errno));
      fclose(f);
      unlink(path);
      return NULL;
    }
  }
  if (fclose(f) != 0) {
    fprintf(stderr, "spill close failed: %s\n", strerror(errno));
    unlink(path);
    return NULL;
  }
  return strdup(path);
}

bool spill_flush(Spill* s, LineVec* vec) {
  if (vec->len == 0) return true;
  char prefix[64];
  snprintf(prefix, sizeof(prefix), "run-%d-%u", s->worker_id, s->seq);
  char* path = spill_write_run(s->dir, prefix, vec);
  if (!path) return false;
  if (!record_run(s, path)) {
    free(path);
    return false;
  }
  s->seq++;
  line_vec_clear(vec);
  return true;
}

void spill_dispose(Spill* s) {
  free(s->runs);
  s->runs = NULL;
  s->n_runs = 0;
  s->cap = 0;
}
