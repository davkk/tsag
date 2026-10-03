#include "discover.h"

#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "lang.h"
#include "strstack.h"

static const char* IGNORED_FILES[] = {".git", "build", "dist"};
// NOTE: node_modules, .venv, vendor/ are indexed on purpose (library defs).

static bool is_ignored(const char* path) {
  // TODO: --exclude flag; matching is whole-components only (not substring).
  const char* p = path;
  for (;;) {
    while (*p == '/') p++;
    if (*p == '\0') break;
    const char* end = strchr(p, '/');
    size_t len = end != NULL ? (size_t)(end - p) : strlen(p);
    if (!(len == 1 && p[0] == '.')) {
      for (size_t i = 0; i < sizeof(IGNORED_FILES) / sizeof(IGNORED_FILES[0]); i++) {
        size_t ilen = strlen(IGNORED_FILES[i]);
        if (len == ilen && strncmp(p, IGNORED_FILES[i], len) == 0) return true;
      }
    }
    if (end == NULL) break;
    p = end + 1;
  }
  return false;
}

void enqueue_path(IoQueue* out_queue, const char* root) {
  StrStack stack;
  strstack_init(&stack);

  char* root_copy = strdup(root);
  if (!root_copy) {
    fprintf(stderr, "out of memory\n");
    strstack_free(&stack);
    return;
  }
  if (strstack_push(&stack, root_copy) != 0) {
    fprintf(stderr, "out of memory\n");
    free(root_copy);
    strstack_free(&stack);
    return;
  }

  char* path;
  while ((path = strstack_pop(&stack)) != NULL) {
    if (is_ignored(path)) {
      free(path);
      continue;
    }

    struct stat info;
    if (lstat(path, &info) == -1) {
      fprintf(stderr, "failed to stat '%s': %s\n", path, strerror(errno));
      free(path);
      continue;
    }

    if (S_ISDIR(info.st_mode)) { // is a directory
      DIR* dir = opendir(path);
      if (!dir) {
        fprintf(stderr, "failed to open directory '%s': %s\n", path, strerror(errno));
        free(path);
        continue;
      }
      struct dirent* entry;
      while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
          size_t flen = strlen(path) + 1 + strlen(entry->d_name) + 1;
          char* fullpath = malloc(flen);
          if (!fullpath) {
            fprintf(stderr, "out of memory\n");
            continue;
          }
          if (strcmp(path, ".") == 0) {
            strcpy(fullpath, entry->d_name);
          } else {
            snprintf(fullpath, flen, "%s/%s", path, entry->d_name);
          }
          if (strstack_push(&stack, fullpath) != 0) {
            fprintf(stderr, "out of memory\n");
            free(fullpath);
          }
        }
      }
      closedir(dir);
      free(path);
    } else if (S_ISREG(info.st_mode)) {
      const char* ext = find_extension(path);
      if (!ext) {
        free(path);
        continue;
      }
      const char* lang = ext_to_lang(ext);
      if (!lang) {
        free(path);
        continue;
      }
      io_queue_put(out_queue, path); // transfer ownership
    } else {
      free(path); // ignore links, sockets, etc.
    }
  }
  strstack_free(&stack);
}
