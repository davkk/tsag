#include "lang.h"
#include "queries.h"

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const struct {
  const char* ext;
  const char* name;
} EXT_LANG[] = {
    {"c", "c"}, {"h", "c"}, {"cpp", "cpp"}, {"cc", "cpp"}, {"hpp", "cpp"}, {"py", "python"},
};

static const char* ext_to_lang(const char* ext) {
  for (size_t i = 0; i < sizeof(EXT_LANG) / sizeof(EXT_LANG[0]); i++) {
    if (strcmp(ext, EXT_LANG[i].ext) == 0) {
      return EXT_LANG[i].name;
    }
  }
  return NULL;
}

static LangEntry* load_lang(const char* dir, const char* lang) {
  LangEntry* entry = malloc(sizeof(LangEntry));
  entry->dl_handle = NULL;
  entry->lang = NULL;
  entry->query = NULL;
  entry->name = NULL;

  char so_path[64], sym[64];
  void* lib = NULL;
  const char* q_src = NULL;
  size_t q_len = 0;
  TSQuery* query = NULL;

  snprintf(so_path, sizeof(so_path), "%s/%s.so", dir, lang);

  lib = dlopen(so_path, RTLD_NOW);
  if (!lib) {
    fprintf(stderr, "Failed to load library: %s\n", dlerror());
    goto cleanup;
  }
  entry->dl_handle = lib;

  snprintf(sym, sizeof(sym), "tree_sitter_%s", lang);
  __extension__ TSLanguage* (*tree_sitter_sym)(void) = (TSLanguage * (*)(void)) dlsym(lib, sym);
  if (!tree_sitter_sym) {
    fprintf(stderr, "Failed to find symbol: %s\n", dlerror());
    goto cleanup;
  }
  entry->lang = tree_sitter_sym();

  for (size_t i = 0; QUERIES[i].lang; i++) {
    if (strcmp(QUERIES[i].lang, lang) == 0) {
      q_src = QUERIES[i].source;
      q_len = QUERIES[i].source_len;
      break;
    }
  }
  if (!q_src) {
    fprintf(stderr, "No embedded query for %s\n", lang);
    goto cleanup;
  }

  uint32_t error_offset = 0;
  TSQueryError error_type = TSQueryErrorNone;
  query = ts_query_new(entry->lang, q_src, (uint32_t)q_len, &error_offset, &error_type);
  if (!query) {
    fprintf(stderr, "Query compile failed at offset %u (error %d)\n", error_offset,
            (int)error_type);
    goto cleanup;
  }
  entry->query = query;

  entry->name = strdup(lang);
  if (!entry->name) goto cleanup;

  return entry;

cleanup:
  free(entry->name);
  ts_query_delete(query);
  if (lib) dlclose(lib);
  free(entry);
  return NULL;
}

LangCache* lang_cache_new(const char* parser_dir) {
  LangCache* cache = calloc(1, sizeof(LangCache));
  if (!cache) return NULL;

  cache->parser_dir = strdup(parser_dir);
  if (!cache->parser_dir) {
    free(cache);
    return NULL;
  }

  int rc = pthread_mutex_init(&cache->lock, NULL);
  if (rc != 0) {
    fprintf(stderr, "pthread_mutex_init failed: %s\n", strerror(rc));
    free(cache->parser_dir);
    free(cache);
    return NULL;
  }

  return cache;
}

void lang_cache_free(LangCache* cache) {
  if (!cache) return;
  for (size_t i = 0; i < cache->entry_count; ++i) {
    LangEntry* entry = &cache->entries[i];
    ts_query_delete(entry->query);
    dlclose(entry->dl_handle);
    free(entry->name);
  }
  pthread_mutex_destroy(&cache->lock);
  free(cache->parser_dir);
  free(cache);
}

const LangEntry* lang_cache_get(LangCache* cache, const char* ext) {
  const char* name = ext_to_lang(ext);
  if (!name) return NULL;

  pthread_mutex_lock(&cache->lock);
  for (size_t i = 0; i < cache->entry_count; ++i) {
    if (strcmp(cache->entries[i].name, name) == 0) {
      pthread_mutex_unlock(&cache->lock);
      return &cache->entries[i];
    }
  }
  if (cache->entry_count == MAX_LANGS) {
    pthread_mutex_unlock(&cache->lock);
    fprintf(stderr, "Maximum number of languages reached\n");
    return NULL;
  }

  LangEntry* tmp = load_lang(cache->parser_dir, name);
  if (!tmp) {
    pthread_mutex_unlock(&cache->lock);
    return NULL;
  }
  cache->entries[cache->entry_count++] = *tmp;
  free(tmp);
  pthread_mutex_unlock(&cache->lock);
  return &cache->entries[cache->entry_count - 1];
}
