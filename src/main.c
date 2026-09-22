#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <tree_sitter/api.h>
#include <unistd.h>

#include "heap.h"
#include "ioqueue.h"
#include "lang.h"
#include "parse.h"
#include "tagvec.h"

#define QUEUE_CAPACITY 256

typedef enum {
  MERGE_KWAY,
  MERGE_EXTERNAL,
} MergeStrat;

typedef struct {
  const char* output;
  MergeStrat merge;
  long jobs; // 0 = auto
  char** paths;
  int path_count;
} Options;

const char* IGNORED_FILES[2] = {".git", "node_modules"};

static void usage(FILE* f) {
  fprintf(f, "Usage: tsag [options] [path...]\n"
             "\n"
             "Options:\n"
             "  -o, --output FILE    write tags to FILE instead of stdout\n"
             "  -j, --jobs N         worker thread count (default: cores-2, min 1)\n"
             "      --merge STRAT    merge strategy: kway (default) | external\n"
             "  -h, --help           show this help and exit\n");
}

static int parse_options(int argc, char** argv, Options* o) {
  static const struct option longs[] = {
      {"output", required_argument, NULL, 'o'},
      {"jobs", required_argument, NULL, 'j'},
      {"merge", required_argument, NULL, 1001},
      {"help", no_argument, NULL, 'h'},
      {NULL, 0, NULL, 0},
  };

  int c;
  while ((c = getopt_long(argc, argv, "o:j:h", longs, NULL)) != -1) {
    switch (c) {
    case 'o':
      o->output = optarg;
      break;
    case 'j': {
      char* end = NULL;
      errno = 0;
      long v = strtol(optarg, &end, 10);
      if (errno != 0 || !end || *end != '\0' || v < 1) {
        fprintf(stderr, "invalid --jobs value '%s': expected integer >= 1\n", optarg);
        return -1;
      }
      o->jobs = v;
      break;
    }
    case 1001:
      if (strcmp(optarg, "kway") == 0) {
        o->merge = MERGE_KWAY;
      } else if (strcmp(optarg, "external") == 0 || strcmp(optarg, "sort") == 0 ||
                 strcmp(optarg, "external-sort") == 0) {
        o->merge = MERGE_EXTERNAL;
      } else {
        fprintf(stderr, "invalid --merge value '%s': expected kway|external\n", optarg);
        return -1;
      }
      break;
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

typedef struct {
  IoQueue* q;
  IoQueue* outq;
  LangCache* cache;
} WorkerArg;

typedef struct {
  IoQueue* outq;
  int n;
  FILE* out;
} MergeArg;

static void* worker(void* arg) {
  WorkerArg* a = (WorkerArg*)arg;
  TSParser* parser = ts_parser_new();
  if (!parser) {
    fprintf(stderr, "Parser init failed\n");
    return NULL;
  }
  TSQueryCursor* cursor = ts_query_cursor_new();
  if (!cursor) {
    fprintf(stderr, "Cursor init failed\n");
    ts_parser_delete(parser);
    return NULL;
  }

  TagVec* vec = tag_vec_new(128);

  char* path;
  while ((path = (char*)io_queue_get(a->q)) != NULL) {
    parse_file(path, a->cache, parser, cursor, vec);
  }

  tag_vec_sort(vec);
  io_queue_put(a->outq, vec);

  ts_query_cursor_delete(cursor);
  ts_parser_delete(parser);
  return NULL;
}

static void* merge(void* arg) {
  MergeArg* a = (MergeArg*)arg;

  TagVec* batches[a->n];
  size_t batch_count = 0;

  TagVec* vec;
  while ((vec = (TagVec*)io_queue_get(a->outq)) != NULL) {
    batches[batch_count++] = vec;
  }

  // populate
  HeapEntry heap[a->n];
  size_t heap_size = 0;
  for (size_t i = 0; i < batch_count; ++i) {
    if (batches[i]->size > 0) {
      HeapEntry entry = {.batch = i, .idx = 0};
      heap_push(heap, &heap_size, entry, batches);
    }
  }

  // drain
  HeapEntry entry;
  while (heap_pop(heap, &heap_size, &entry, batches)) {
    Tag* tag = &batches[entry.batch]->tags[entry.idx];
    fprintf(a->out, "%s\t%s\t/^%s$/;\"\t%s\n", tag->name, tag->file, tag->pattern, tag->kind);
    if (entry.idx + 1 < batches[entry.batch]->size) {
      entry.idx++;
      heap_push(heap, &heap_size, entry, batches);
    }
  }

  for (size_t i = 0; i < batch_count; ++i) {
    tag_vec_free(batches[i]);
  }

  return NULL;
}

static bool is_ignored(const char* path) {
  for (size_t i = 0; i < 2; i++) {
    if (strstr(path, IGNORED_FILES[i]) != NULL) return true;
  }
  return false;
}

static void enqueue_path(IoQueue* queue, const char* path) {
  if (is_ignored(path)) return;
  struct stat info;
  if (lstat(path, &info) == -1) {
    fprintf(stderr, "failed to stat '%s': %s\n", path, strerror(errno));
    return;
  }
  if (S_ISDIR(info.st_mode)) { // is a directory
    DIR* dir = opendir(path);
    if (dir == NULL) {
      fprintf(stderr, "failed to open directory: %s\n", path);
      return;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
        size_t len = strlen(path) + 1 + strlen(entry->d_name) + 1;
        char* fullpath = malloc(len);
        if (strcmp(path, ".") == 0) {
          strcpy(fullpath, entry->d_name);
        } else {
          snprintf(fullpath, len, "%s/%s", path, entry->d_name);
        }
        enqueue_path(queue, fullpath);
        free(fullpath);
      }
    }
    closedir(dir);
  } else if (S_ISREG(info.st_mode)) { // is a regular file
    const char* ext = find_extension(path);
    if (!ext) return;
    const char* lang = ext_to_lang(ext);
    if (!lang) return;
    io_queue_put(queue, strdup(path));
  } // ignore links
}

int main(int argc, char** argv) {
  Options opts = {0};
  if (parse_options(argc, argv, &opts) != 0) {
    return 2;
  }
  if (opts.merge == MERGE_EXTERNAL) {
    fprintf(stderr, "--merge=external not yet implemented (kway is the current fallback)\n");
    return 2;
  }

  const char* grammars_dir = getenv("TSAG_GRAMMARS");
  if (!grammars_dir || !*grammars_dir) grammars_dir = "/home/davkk/.local/share/nvim/site/parser/";

  LangCache* cache = lang_cache_new(grammars_dir);
  if (!cache) {
    fprintf(stderr, "cache init failed\n");
    return 1;
  }

  long n = opts.jobs;
  if (n <= 0) {
    n = sysconf(_SC_NPROCESSORS_ONLN) - 2;
    if (n < 1) n = 1;
  }

  FILE* out = stdout;
  if (opts.output) {
    out = fopen(opts.output, "w");
    if (!out) {
      fprintf(stderr, "failed to open output '%s': %s\n", opts.output, strerror(errno));
      lang_cache_free(cache);
      return 1;
    }
  }

  IoQueue* queue = io_queue_new(QUEUE_CAPACITY);
  if (!queue) {
    fprintf(stderr, "io queue init failed\n");
    if (out != stdout) fclose(out);
    lang_cache_free(cache);
    return 1;
  }

  IoQueue* out_queue = io_queue_new((size_t)n);
  if (!out_queue) {
    fprintf(stderr, "io out queue init failed\n");
    io_queue_free(queue);
    if (out != stdout) fclose(out);
    lang_cache_free(cache);
    return 1;
  }

  pthread_t* threads = malloc((size_t)n * sizeof(*threads));
  if (!threads) {
    fprintf(stderr, "out of memory\n");
    io_queue_free(out_queue);
    io_queue_free(queue);
    if (out != stdout) fclose(out);
    lang_cache_free(cache);
    return 1;
  }
  WorkerArg arg = {queue, out_queue, cache};

  for (long i = 0; i < n; i++) {
    int rc = pthread_create(&threads[i], NULL, worker, &arg);
    if (rc) {
      fprintf(stderr, "Failed to create thread %ld\n", i);
      io_queue_close(queue);
      io_queue_close(out_queue);
      free(threads);
      io_queue_free(out_queue);
      io_queue_free(queue);
      if (out != stdout) fclose(out);
      lang_cache_free(cache);
      return 1;
    }
  }

  if (opts.path_count == 0) {
    enqueue_path(queue, ".");
  } else {
    for (int i = 0; i < opts.path_count; i++) {
      enqueue_path(queue, opts.paths[i]);
    }
  }
  io_queue_close(queue);

  MergeArg merge_arg = {out_queue, (int)n, out};
  pthread_t merge_thread;
  pthread_create(&merge_thread, NULL, merge, &merge_arg);

  for (long i = 0; i < n; i++) {
    int rc = pthread_join(threads[i], NULL);
    if (rc) {
      fprintf(stderr, "Failed to join thread %ld\n", i);
      return 1;
    }
  }
  io_queue_close(out_queue);
  pthread_join(merge_thread, NULL);

  free(threads);
  io_queue_free(out_queue);
  io_queue_free(queue);
  lang_cache_free(cache);
  if (out != stdout) {
    if (fclose(out) != 0) {
      fprintf(stderr, "failed to close output '%s': %s\n", opts.output, strerror(errno));
      return 1;
    }
  }
  return 0;
}
