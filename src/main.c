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
#include <sys/wait.h>
#include <tree_sitter/api.h>
#include <unistd.h>

#include "heap.h"
#include "ioqueue.h"
#include "lang.h"
#include "parse.h"
#include "tagvec.h"

#define QUEUE_CAPACITY 256
#define DEFAULT_OUTPUT_FILEPATH "tags"

typedef struct {
  const char* output;
  long jobs; // 0 = auto
  char** paths;
  int path_count;
} Options;

const char* IGNORED_FILES[3] = {".git", "node_modules", "build"};

static void usage(FILE* f) {
  fprintf(f, "Usage: tsag [options] [path...]\n"
             "\n"
             "Options:\n"
             "  -o FILE    write tags to FILE\n"
             "  -j N       worker thread count (default: cores-2, min 1)\n"
             "  -h         show this help and exit\n");
}

static int parse_options(int argc, char** argv, Options* o) {
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

  // TODO: add directory/project name to the template for easier debugging
  // TODO: disk quota exceeded - I might have to use .cache dir instead of /tmp
  // TODO: tmp files are not removed if the process is killed/ctrl-c
  char tmp_path[] = "/tmp/tsag.XXXXXX"; // NOTE: the X will be replaced with a real path by mkstemp
  int tmp_fd = mkstemp(tmp_path);
  if (tmp_fd == -1) {
    fprintf(stderr, "mkstemp: %s\n", strerror(errno));
    return NULL;
  }

  FILE* tmp_file = fdopen(tmp_fd, "w");
  if (!tmp_file) {
    close(tmp_fd);
    unlink(tmp_path);
    return NULL;
  }

  char* path;
  while ((path = (char*)io_queue_get(a->q)) != NULL) {
    parse_file(path, a->cache, parser, cursor, tmp_file);
    free(path);
  }

  if (fflush(tmp_file) != 0 || ferror(tmp_file)) {
    fprintf(stderr, "spill write failed: %s\n", strerror(errno));
    fclose(tmp_file);
    unlink(tmp_path);
    return NULL;
  }
  if (fclose(tmp_file) != 0) {
    fprintf(stderr, "spill close failed: %s\n", strerror(errno));
    unlink(tmp_path);
    return NULL;
  }

  // TODO: check strdup status
  io_queue_put(a->outq, strdup(tmp_path));

  ts_query_cursor_delete(cursor);
  ts_parser_delete(parser);
  return NULL;
}

static void* merge(void* arg) {
  MergeArg* a = (MergeArg*)arg;

  if (a->n <= 0) {
    fprintf(stderr, "merge: invalid worker count\n");
    return (void*)(intptr_t)1;
  }

  int pipefd[2];
  if (pipe(pipefd) == -1) {
    fprintf(stderr, "pipe: %s\n", strerror(errno));
    return (void*)(intptr_t)1;
  }

  pid_t pid = fork();
  if (pid == -1) {
    fprintf(stderr, "fork: %s\n", strerror(errno));
    close(pipefd[0]);
    close(pipefd[1]);
    return (void*)(intptr_t)1;
  }

  if (pid == 0) { // child
    close(pipefd[1]);
    if (dup2(pipefd[0], STDIN_FILENO) == -1) { // point child stdin to pipe read end
      fprintf(stderr, "dup2 stdin: %s\n", strerror(errno));
      _exit(127);
    }
    close(pipefd[0]);

    setenv("LC_ALL", "C", 1);
    setenv("LC_COLLATE", "C", 1);

    if (a->out != stdout) { // point child stdout to output file
      fflush(a->out);
      if (dup2(fileno(a->out), STDOUT_FILENO) == -1) {
        fprintf(stderr, "dup2 stdout: %s\n", strerror(errno));
        _exit(127);
      }
    }

    char* sort_argv[] = {"sort", "-u", NULL};
    // FIXME: resolve absolute path to sort binary at compile time
    execvp("sort", sort_argv);
    fprintf(stderr, "exec sort: %s\n", strerror(errno));
    _exit(127); // exit child
    return NULL;
  }

  // parent
  close(pipefd[0]);
  signal(SIGPIPE, SIG_IGN); // avoid dying if sort exits before we finish writing

  char* path;
  while ((path = io_queue_get(a->outq)) != NULL) {
    FILE* f = fopen(path, "r");
    if (!f) {
      fprintf(stderr, "fopen %s: %s\n", path, strerror(errno));
      free(path);
      continue;
    }
    char buf[65536];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
      if (write(pipefd[1], buf, n) != (ssize_t)n) {
        fprintf(stderr, "write to sort: %s\n", strerror(errno));
        break;
      }
    }
    fclose(f);
    unlink(path);
    free(path);
  }
  close(pipefd[1]);

  int status = 0;
  while (waitpid(pid, &status, 0) == -1 && errno == EINTR) {
  }

  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) { // check for child exit status
    fprintf(stderr, "sort failed (status %d)\n", WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    return (void*)(intptr_t)1;
  }

  return NULL;
}

static bool is_ignored(const char* path) {
  // TODO: add --exclude flag and add more dirs
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

  FILE* out = fopen(opts.output ? opts.output : DEFAULT_OUTPUT_FILEPATH, "w");
  if (!out) {
    fprintf(stderr, "failed to open output '%s': %s\n", opts.output, strerror(errno));
    lang_cache_free(cache);
    return 1;
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
