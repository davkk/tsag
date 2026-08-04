#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>

#include "lang.h"

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

static void print_snippet(const char* src, uint32_t start, uint32_t end) {
  printf(" ");
  for (uint32_t i = start; i < end; i++) {
    char c = src[i];
    if (c == '\n') {
      printf("\\n");
      break;
    }
    if (c == '\t') {
      printf("\\t");
      continue;
    }
    putchar(c);
  }
}

static void dump_node(TSNode node, const char* src, int depth, const char* field) {
  if (ts_node_is_null(node)) return;

  for (int i = 0; i < depth; i++) printf("  ");
  if (field) printf("%s: ", field);

  uint32_t start = ts_node_start_byte(node);
  uint32_t end = ts_node_end_byte(node);
  printf("(%s [%u, %u]", ts_node_type(node), start, end);

  uint32_t child_count = ts_node_child_count(node);
  if (child_count > 0) {
    printf("\n");
    for (uint32_t i = 0; i < child_count; i++) {
      TSNode child = ts_node_child(node, i);
      const char* child_field = ts_node_field_name_for_child(node, i);
      dump_node(child, src, depth + 1, child_field);
    }
    for (int i = 0; i < depth; i++) printf("  ");
    printf(")\n");
  } else {
    print_snippet(src, start, end);
    printf(")\n");
  }
}

static void dump_tree(TSParser* parser, const TSLanguage* lang, const char* src,
                      uint32_t src_len) {
  ts_parser_set_language(parser, lang);
  TSTree* tree = ts_parser_parse_string(parser, NULL, src, src_len);
  if (!tree) {
    fprintf(stderr, "parse failed\n");
    return;
  }
  TSNode root = ts_tree_root_node(tree);
  dump_node(root, src, 0, NULL);
  ts_tree_delete(tree);
}

static void run_query(TSQueryCursor* cursor, const TSQuery* query, const TSLanguage* lang,
                      const char* src, uint32_t src_len) {
  TSParser* parser = ts_parser_new();
  ts_parser_set_language(parser, lang);
  TSTree* tree = ts_parser_parse_string(parser, NULL, src, src_len);

  ts_query_cursor_exec(cursor, query, ts_tree_root_node(tree));

  uint32_t match_count = 0;
  TSQueryMatch match;
  while (ts_query_cursor_next_match(cursor, &match)) {
    printf("match %u (pattern %u)\n", match_count++, match.pattern_index);
    for (uint16_t i = 0; i < match.capture_count; i++) {
      TSQueryCapture cap = match.captures[i];
      uint32_t name_len = 0;
      const char* name = ts_query_capture_name_for_id(query, cap.index, &name_len);
      uint32_t start = ts_node_start_byte(cap.node);
      uint32_t end = ts_node_end_byte(cap.node);
      printf("  %-20.*s [%u, %u]", (int)name_len, name, start, end);
      print_snippet(src, start, end);
      printf("\n");
    }
  }

  ts_tree_delete(tree);
  ts_parser_delete(parser);
}

int main(int argc, char** argv) {
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s <file> [query.scm]\n", argv[0]);
    return 1;
  }
  const char* filepath = argv[1];
  const char* query_file = argc == 3 ? argv[2] : NULL;

  const char* dir = getenv("TSAG_GRAMMARS");
  if (!dir || !*dir) dir = "/home/davkk/.local/share/nvim/site/parser/";

  LangCache* cache = lang_cache_new(dir);
  if (!cache) return 1;

  const char* dot = strrchr(filepath, '.');
  if (!dot) {
    fprintf(stderr, "no extension: %s\n", filepath);
    lang_cache_free(cache);
    return 1;
  }
  const char* ext = dot + 1;
  const LangEntry* entry = lang_cache_get(cache, ext);
  if (!entry) {
    fprintf(stderr, "no grammar for extension: %s\n", ext);
    lang_cache_free(cache);
    return 1;
  }

  size_t src_len = 0;
  char* src = read_file(filepath, &src_len);
  if (!src) {
    fprintf(stderr, "read failed: %s\n", filepath);
    lang_cache_free(cache);
    return 1;
  }

  TSParser* parser = ts_parser_new();

  if (query_file) {
    size_t q_len = 0;
    char* q_src = read_file(query_file, &q_len);
    if (!q_src) {
      fprintf(stderr, "read failed: %s\n", query_file);
      ts_parser_delete(parser);
      free(src);
      lang_cache_free(cache);
      return 1;
    }
    uint32_t error_offset = 0;
    TSQueryError error_type = TSQueryErrorNone;
    TSQuery* query =
        ts_query_new(entry->lang, q_src, (uint32_t)q_len, &error_offset, &error_type);
    if (!query) {
      fprintf(stderr, "query compile failed at offset %u (error %d)\n", error_offset,
              (int)error_type);
      free(q_src);
      ts_parser_delete(parser);
      free(src);
      lang_cache_free(cache);
      return 1;
    }
    TSQueryCursor* cursor = ts_query_cursor_new();
    run_query(cursor, query, entry->lang, src, (uint32_t)src_len);
    ts_query_cursor_delete(cursor);
    ts_query_delete(query);
    free(q_src);
  } else {
    dump_tree(parser, entry->lang, src, (uint32_t)src_len);
  }

  ts_parser_delete(parser);
  free(src);
  lang_cache_free(cache);
  return 0;
}
