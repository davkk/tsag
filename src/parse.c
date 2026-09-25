#include "parse.h"
#include "tagvec.h"

#include <ctype.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_LINE_LEN 512
#define MAX_FILE_SIZE (2UL * 1024 * 1024) // 2 MiB: skip huge/generated files

static char* read_file(const char* path, size_t* out_len) {
  int fd = open(path, O_RDONLY);
  if (fd == -1) return NULL;
  struct stat sb;
  if (fstat(fd, &sb) != 0) {
    close(fd);
    return NULL;
  }
  if (!S_ISREG(sb.st_mode) || (size_t)sb.st_size > MAX_FILE_SIZE) {
    close(fd);
    return NULL;
  }
  FILE* f = fdopen(fd, "rb");
  if (!f) {
    close(fd);
    return NULL;
  }
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

static void line_range(const char* src, size_t src_len, uint32_t start, const char** out, size_t* out_len) {
  size_t s = start;
  size_t back_limit = (start > MAX_LINE_LEN) ? start - MAX_LINE_LEN : 0;
  while (s > back_limit && src[s - 1] != '\n') s--;
  size_t e = start;
  size_t fwd_limit = (src_len - start > MAX_LINE_LEN) ? start + MAX_LINE_LEN : src_len;
  while (e < fwd_limit && src[e] != '\n') e++;
  *out = src + s;
  *out_len = e - s;
}

static char* escape_pattern(const char* src, size_t len) {
  size_t w = 0;
  for (size_t i = 0; i < len; i++) {
    char c = src[i];
    if (c == '\n' || c == '\r') break;
    if (c == '\t' || c == '\\' || c == '/' || (c == '$' && i + 1 == len))
      w += 2;
    else
      w += 1;
  }
  char* dst = malloc(w + 1);
  if (!dst) return NULL;
  size_t out = 0;
  for (size_t i = 0; i < len; i++) {
    char c = src[i];
    if (c == '\n' || c == '\r') break;
    if (c == '\t') {
      dst[out++] = '\\';
      dst[out++] = 't';
      continue;
    }
    if (c == '\\' || c == '/') {
      dst[out++] = '\\';
      dst[out++] = c;
      continue;
    }
    if (c == '$' && i + 1 == len) {
      dst[out++] = '\\';
      dst[out++] = '$';
      continue;
    }
    dst[out++] = c;
  }
  dst[w] = '\0';
  return dst;
}

int parse_file(char* filepath, LangCache* cache, TSParser* parser, TSQueryCursor* cursor, FILE* out) {
  const char* ext = find_extension(filepath);
  if (!ext) return 1;

  const LangEntry* entry = lang_cache_get(cache, ext);
  if (!entry) return 1;

  if (!ts_parser_set_language(parser, entry->lang)) {
    fprintf(stderr, "Failed to set language for %s\n", filepath);
    return 1;
  }

  size_t src_len = 0;
  char* source = read_file(filepath, &src_len);
  if (!source) return 1;

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
    uint32_t name_start = 0;
    uint32_t name_end = 0;

    for (uint16_t i = 0; i < match.capture_count; i++) {
      TSQueryCapture cap = match.captures[i];
      uint32_t cn_len = 0;
      const char* cn = ts_query_capture_name_for_id(entry->query, cap.index, &cn_len);

      if (cn_len == 4 && memcmp(cn, "name", 4) == 0) {
        name_start = ts_node_start_byte(cap.node);
        name_end = ts_node_end_byte(cap.node);
        name = source + name_start;
        name_len = name_end - name_start;
      } else if (cn_len > 5 && memcmp(cn, "kind.", 5) == 0) {
        kind = cn + 5;
      }
    }

    if (name_len == 0 || !kind[0]) continue;

    const char* line;
    size_t ll;
    line_range(source, src_len, name_start, &line, &ll);
    content = line;
    content_len = ll;

    // TODO: do we need this?
    bool bad_name = false;
    for (size_t k = 0; k < name_len; k++) {
      if (!isprint((unsigned char)name[k])) {
        bad_name = true;
        break;
      }
    }
    if (bad_name) continue;

    char* pattern = escape_pattern(content, content_len);
    if (!pattern) continue;

    fprintf(out, "%.*s\t%s\t/^%s$/;\"\t%s\n", (int)name_len, name, filepath, pattern, kind);
    free(pattern);
  }

  ts_tree_delete(tree);
  free(source);
  return 0;
}
