#ifndef LANG_H
#define LANG_H

#include <pthread.h>
#include <tree_sitter/api.h>

#define MAX_LANGS 32

typedef struct LangEntry {
  char* name;
  void* dl_handle;
  TSLanguage* lang;
  TSQuery* query;
} LangEntry;

typedef struct LangCache {
  pthread_mutex_t lock;
  pthread_cond_t loaded; // broadcast when entries[] or loading[] changes
  char* parser_dir;
  size_t entry_count;
  LangEntry entries[MAX_LANGS];
  const char* loading[MAX_LANGS]; // EXT_LANG names with load in flight
  size_t loading_count;
} LangCache;

const char* find_extension(const char* path);
const char* ext_to_lang(const char* ext);
LangCache* lang_cache_new(const char* parser_dir);
void lang_cache_free(LangCache* cache);
const LangEntry* lang_cache_get(LangCache* cache, const char* ext);

#endif
