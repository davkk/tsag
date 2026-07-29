#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>

#define TSAG_MAX_LANGS 10

typedef struct TSAGLangEntry {
  char* name;
  void* dl_handle;
  TSLanguage* lang;
  TSQuery* query;
} TSAGLangEntry;

typedef struct TSAGLangCache {
  pthread_mutex_t lock;
  char* parser_dir;
  size_t entry_count;
  TSAGLangEntry entries[TSAG_MAX_LANGS];
} TSAGLangCache;

static const struct {
  const char* ext;
  const char* name;
} EXT_LANG[] = {
    {"c", "c"}, {"h", "c"}, {"cpp", "cpp"}, {"cc", "cpp"}, {"hpp", "cpp"}, {"py", "python"},
};

static char* read_file(const char* path, size_t* out_len) {
  FILE* f = fopen(path, "rb");
  if (!f) return NULL;
  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return NULL;
  }
  long n = ftell(f);
  if (n < 0) {
    fclose(f);
    return NULL;
  }
  fseek(f, 0, SEEK_SET);
  char* buf = malloc((size_t)n + 1);
  if (!buf) {
    fclose(f);
    return NULL;
  }
  size_t got = fread(buf, 1, (size_t)n, f);
  fclose(f);
  buf[got] = '\0';
  if (out_len) *out_len = got;
  return buf;
}

static const char* ext_to_lang(const char* ext) {
  for (size_t i = 0; i < sizeof(EXT_LANG) / sizeof(EXT_LANG[0]); i++) {
    if (strcmp(ext, EXT_LANG[i].ext) == 0) {
      return EXT_LANG[i].name;
    }
  }
  return NULL;
}

static TSAGLangEntry* load_lang(const char* dir, const char* lang) {
  TSAGLangEntry* entry = malloc(sizeof(TSAGLangEntry));
  entry->dl_handle = NULL;
  entry->lang = NULL;
  entry->query = NULL;
  entry->name = NULL;

  char so_path[64], q_path[64], sym[64];
  void* lib = NULL;
  size_t q_len = 0;
  char* q_src = NULL;
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

  snprintf(q_path, sizeof(q_path), "queries/%s.scm", lang);
  q_src = read_file(q_path, &q_len);
  if (!q_src) {
    fprintf(stderr, "Failed to read query %s\n", q_path);
    goto cleanup;
  }

  uint32_t error_offset = 0;
  TSQueryError error_type = TSQueryErrorNone;
  query = ts_query_new(entry->lang, q_src, (uint32_t)q_len, &error_offset, &error_type);
  free(q_src);
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
  if (q_src) free(q_src);
  if (lib) dlclose(lib);
  free(entry);
  return NULL;
}

static TSAGLangCache* lang_cache_new(const char* parser_dir) {
  TSAGLangCache* cache = calloc(1, sizeof(TSAGLangCache));
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

static void lang_cache_free(TSAGLangCache* cache) {
  if (!cache) return;
  for (size_t i = 0; i < cache->entry_count; ++i) {
    TSAGLangEntry* entry = &cache->entries[i];
    ts_query_delete(entry->query);
    dlclose(entry->dl_handle);
    free(entry->name);
  }
  pthread_mutex_destroy(&cache->lock);
  free(cache->parser_dir);
  free(cache);
}

static const TSAGLangEntry* lang_cache_get(TSAGLangCache* cache, const char* ext) {
  const char* name = ext_to_lang(ext);
  if (!name) return NULL;
  pthread_mutex_lock(&cache->lock);
  size_t i = 0;
  for (i = 0; i < cache->entry_count; ++i) {
    if (strcmp(cache->entries[i].name, name) == 0) break;
  }
  if (i < cache->entry_count) {
    pthread_mutex_unlock(&cache->lock);
    return &cache->entries[i];
  }
  if (cache->entry_count == TSAG_MAX_LANGS) {
    pthread_mutex_unlock(&cache->lock);
    fprintf(stderr, "Maximum number of languages reached\n");
    return NULL;
  }

  TSAGLangEntry* tmp = load_lang(cache->parser_dir, name);
  if (!tmp) {
    pthread_mutex_unlock(&cache->lock);
    return NULL;
  }

  cache->entries[cache->entry_count++] = *tmp;
  free(tmp);

  pthread_mutex_unlock(&cache->lock);
  return &cache->entries[cache->entry_count - 1];
}

static void line_range(const char* src, size_t src_len, uint32_t start, const char** out,
                       size_t* out_len) {
  size_t s = start;
  while (s > 0 && src[s - 1] != '\n') s--;
  size_t e = start;
  while (e < src_len && src[e] != '\n') e++;
  *out = src + s;
  *out_len = e - s;
}

static int parse_file(const char* filepath, TSAGLangCache* cache, TSParser* parser, TSQueryCursor* cursor) {
  const char* dot = strrchr(filepath, '.');
  if (!dot) {
    fprintf(stderr, "File does not have an extension: %s\n", filepath);
    return 1;
  }
  const char* ext = dot + 1;

  const TSAGLangEntry* entry = lang_cache_get(cache, ext);
  if (!entry) {
    fprintf(stderr, "No language for %s\n", filepath);
    return 1;
  }

  if (!ts_parser_set_language(parser, entry->lang)) {
    fprintf(stderr, "Failed to set language for %s\n", filepath);
    return 1;
  }

  size_t src_len = 0;
  char* source = read_file(filepath, &src_len);
  if (!source) {
    fprintf(stderr, "Failed to read %s\n", filepath);
    return 1;
  }

  TSTree* tree = ts_parser_parse_string(parser, NULL, source, (uint32_t)src_len);
  if (!tree) {
    fprintf(stderr, "Parsing failed for %s\n", filepath);
    free(source);
    return 1;
  }

  TSNode root = ts_tree_root_node(tree);
  ts_query_cursor_set_max_start_depth(cursor, 1);
  ts_query_cursor_exec(cursor, entry->query, root);

  TSQueryMatch match;
  while (ts_query_cursor_next_match(cursor, &match)) {
    const char* name = "";
    size_t name_len = 0;
    const char* kind = "";
    size_t kind_len = 0;
    const char* content = "";
    size_t content_len = 0;

    for (uint16_t i = 0; i < match.capture_count; i++) {
      TSQueryCapture cap = match.captures[i];
      uint32_t cn_len = 0;
      const char* cn = ts_query_capture_name_for_id(entry->query, cap.index, &cn_len);

      if (cn_len == 4 && memcmp(cn, "name", 4) == 0) {
        uint32_t a = ts_node_start_byte(cap.node);
        uint32_t b = ts_node_end_byte(cap.node);
        name = source + a;
        name_len = b - a;
      } else if (cn_len > 5 && memcmp(cn, "kind.", 5) == 0) {
        kind = cn + 5;
        kind_len = cn_len - 5;
        const char* line;
        size_t ll;
        line_range(source, src_len, ts_node_start_byte(cap.node), &line, &ll);
        content = line;
        content_len = ll;
      }
    }

    printf("%.*s\t%s\t/^%.*s$/;\"\t%.*s\n", (int)name_len, name, filepath, (int)content_len,
           content, (int)kind_len, kind);
  }

  ts_tree_delete(tree);
  free(source);
  return 0;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
    return 1;
  }

  const char* dir = getenv("TSAG_GRAMMARS");
  if (!dir || !*dir) dir = "/home/davkk/.local/share/nvim/site/parser/";

  TSAGLangCache* cache = lang_cache_new(dir);
  if (!cache) {
    fprintf(stderr, "cache init failed\n");
    return 1;
  }

  TSParser* parser = ts_parser_new();
  if (!parser) {
    fprintf(stderr, "Parser init failed\n");
    return 1;
  }

  TSQueryCursor* cursor = ts_query_cursor_new();
  if (!cursor) {
    fprintf(stderr, "Cursor init failed\n");
    ts_parser_delete(parser);
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    const char* filepath = argv[i];
    fprintf(stderr, "Processing %s\n", filepath);
    int rc = parse_file(filepath, cache, parser, cursor);
    if (rc) fprintf(stderr, "Failed to parse %s\n", filepath);
  }

  ts_query_cursor_delete(cursor);
  ts_parser_delete(parser);
  lang_cache_free(cache);
  return 0;
}
