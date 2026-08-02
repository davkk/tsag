#include "parse.h"
#include "tagvec.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void line_range(const char* src, size_t src_len, uint32_t start, const char** out,
                       size_t* out_len) {
  size_t s = start;
  while (s > 0 && src[s - 1] != '\n') s--;
  size_t e = start;
  while (e < src_len && src[e] != '\n') e++;
  *out = src + s;
  *out_len = e - s;
}

static char* escape_pattern(const char* src, size_t len) {
  char* dst = malloc(len * 2 + 1);
  if (!dst) return NULL;
  size_t w = 0;
  for (size_t i = 0; i < len; i++) {
    char c = src[i];
    if (c == '\n' || c == '\r') break;
    if (c == '\t') {
      dst[w++] = '\\';
      dst[w++] = 't';
      continue;
    }
    if (c == '\\' || c == '/') {
      dst[w++] = '\\';
      dst[w++] = c;
      continue;
    }
    if (c == '$' && i + 1 == len) {
      dst[w++] = '\\';
      dst[w++] = '$';
      continue;
    }
    dst[w++] = c;
  }
  dst[w] = '\0';
  return dst;
}

int parse_file(const char* filepath, LangCache* cache, TSParser* parser, TSQueryCursor* cursor,
               TagVec* vec) {
  const char* dot = strrchr(filepath, '.');
  if (!dot) {
    return 1;
  }
  const char* ext = dot + 1;

  const LangEntry* entry = lang_cache_get(cache, ext);
  if (!entry) {
    return 1;
  }

  if (!ts_parser_set_language(parser, entry->lang)) {
    fprintf(stderr, "Failed to set language for %s\n", filepath);
    return 1;
  }

  size_t src_len = 0;
  char* source = read_file(filepath, &src_len);
  if (!source) {
    return 1;
  }

  TSTree* tree = ts_parser_parse_string(parser, NULL, source, (uint32_t)src_len);
  if (!tree) {
    fprintf(stderr, "Parsing failed for %s\n", filepath);
    free(source);
    return 1;
  }

  TSNode root = ts_tree_root_node(tree);
  ts_query_cursor_exec(cursor, entry->query, root);

  TSQueryMatch match;
  while (ts_query_cursor_next_match(cursor, &match)) {
    const char* name = "";
    size_t name_len = 0;
    const char* kind = "";
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
        const char* line;
        size_t ll;
        line_range(source, src_len, ts_node_start_byte(cap.node), &line, &ll);
        content = line;
        content_len = ll;
      }
    }

    if (name_len == 0 || content_len == 0 || !kind[0]) continue;

    bool bad_name = false;
    for (size_t k = 0; k < name_len; k++) {
      if (!isprint((unsigned char)name[k])) { bad_name = true; break; }
    }
    if (bad_name) continue;

    char* pattern = escape_pattern(content, content_len);
    if (!pattern) continue;

    Tag tag = {
        .name = strndup(name, name_len),
        .file = filepath,
        .pattern = pattern,
        .kind = kind,
    };
    tag_vec_push(vec, &tag);
  }

  ts_tree_delete(tree);
  free(source);
  return 0;
}
