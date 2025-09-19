#include "context.h"
#include "utils.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_whitespace_or_slash(char c) { return isspace(c) || c == '/'; }

// TODO: separate some concerns.
// File state: tag, tag_len, state <- these should refresh on every file.
// Global state: outputbuf, submodule dir <- these should persist.
int parse_context_init(ParserContext *pctx, char *submodule_dir) {
  pctx->ob.buf = malloc(KB * 4);
  if (!pctx->ob.buf) {
    perror("malloc");
    return -1;
  }

  pctx->ob.len = 0;
  pctx->ob.capacity = KB * 4;

  trim_in_place(submodule_dir, is_whitespace_or_slash);
  pctx->submodule_dir = strdup(submodule_dir);
  if (!pctx->submodule_dir) {
    perror("strdup");
    free(pctx->ob.buf);
    return -1;
  }

  return 0;
}

int ob_resize(ParserContext *pctx) {
  size_t new_capacity = pctx->ob.capacity + KB * 4;
  char *new_buf = realloc(pctx->ob.buf, new_capacity);
  if (!new_buf) {
    perror("realloc");
    return -1;
  }

  pctx->ob.capacity = new_capacity;
  pctx->ob.buf = new_buf;
  return 0;
}

int ob_append_char(ParserContext *pctx, char c) {
  if (pctx->ob.capacity - pctx->ob.len - 1 <= 0 && ob_resize(pctx) < 0) {
    return -1;
  }

  pctx->ob.buf[pctx->ob.len++] = c;
  pctx->ob.buf[pctx->ob.len] = '\0';
  return 0;
}

int ob_append_str(ParserContext *pctx, char *str) {
  size_t str_len = strlen(str);
  if (pctx->ob.capacity - pctx->ob.len - 1 < str_len && ob_resize(pctx) < 0) {
    return -1;
  }

  memcpy(pctx->ob.buf + pctx->ob.len, str, str_len);
  pctx->ob.len += str_len;
  pctx->ob.buf[pctx->ob.len] = '\0';
  return 0;
}

void init_file_context(FileContext *fctx) {
  fctx->tag[0] = '\0';
  fctx->tag_len = 0;
  fctx->state = CTX_SCANNING;
}
