#include "context.h"
#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: separate some concerns.
// File state: tag, tag_len, state <- these should refresh on every file.
// Global state: outputbuf, submodule dir <- these should persist.
int context_init(ParserContext *ctx, char *submodule_dir) {
  ctx->ob.buf = malloc(KB * 4);
  if (!ctx->ob.buf) {
    perror("malloc");
    return -1;
  }

  ctx->ob.len = 0;
  ctx->ob.capacity = KB * 4;

  ctx->submodule_dir = strdup(submodule_dir);
  if (!ctx->submodule_dir) {
    perror("strdup");
    free(ctx->ob.buf);
    return -1;
  }

  return 0;
}

int output_buf_resize(ParserContext *ctx) {
  size_t new_capacity = ctx->ob.capacity + KB * 4;
  char *new_buf = realloc(ctx->ob.buf, new_capacity);
  if (!new_buf) {
    perror("realloc");
    return -1;
  }

  ctx->ob.capacity = new_capacity;
  ctx->ob.buf = new_buf;
  return 0;
}

int output_buf_append_char(ParserContext *ctx, char c) {
  if (ctx->ob.capacity - ctx->ob.len - 1 <= 0 && output_buf_resize(ctx) < 0) {
    return -1;
  }

  ctx->ob.buf[ctx->ob.len++] = c;
  ctx->ob.buf[ctx->ob.len] = '\0';
  return 0;
}

int output_buf_append_str(ParserContext *ctx, char *str) {
  size_t str_len = strlen(str);
  if (ctx->ob.capacity - ctx->ob.len - 1 < str_len &&
      output_buf_resize(ctx) < 0) {
    return -1;
  }

  memcpy(ctx->ob.buf + ctx->ob.len, str, str_len);
  ctx->ob.len += str_len;
  ctx->ob.buf[ctx->ob.len] = '\0';
  return 0;
}
