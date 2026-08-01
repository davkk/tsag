#ifndef LANG_H
#define LANG_H

#include <pthread.h>
#include <tree_sitter/api.h>

#define MAX_LANGS 10

typedef struct LangEntry {
  char* name;
  void* dl_handle;
  TSLanguage* lang;
  TSQuery* query;
} LangEntry;

typedef struct LangCache {
  pthread_mutex_t lock;
  char* parser_dir;
  size_t entry_count;
  LangEntry entries[MAX_LANGS];
} LangCache;

LangCache* lang_cache_new(const char* parser_dir);
void lang_cache_free(LangCache* cache);
const LangEntry* lang_cache_get(LangCache* cache, const char* ext);

#endif
