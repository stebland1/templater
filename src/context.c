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
  ctx->tag[0] = '\0';
  ctx->tag_len = 0;
  ctx->state = CTX_SCANNING;
  ctx->outputbuf.buf = malloc(KB * 4);
  if (!ctx->outputbuf.buf) {
    perror("malloc");
    return -1;
  }

  ctx->outputbuf.len = 0;
  ctx->outputbuf.capacity = KB * 4;

  ctx->submodule_dir = strdup(submodule_dir);
  if (!ctx->submodule_dir) {
    perror("strdup");
    free(ctx->outputbuf.buf);
    return -1;
  }

  return 0;
}

int output_buf_resize(ParserContext *ctx) {
  size_t new_capacity = ctx->outputbuf.capacity + KB * 4;
  char *new_buf = realloc(ctx->outputbuf.buf, new_capacity);
  if (!new_buf) {
    perror("realloc");
    return -1;
  }

  ctx->outputbuf.capacity = new_capacity;
  ctx->outputbuf.buf = new_buf;
  return 0;
}

int output_buf_append_char(ParserContext *ctx, char c) {
  if (ctx->outputbuf.capacity - ctx->outputbuf.len - 1 <= 0 &&
      output_buf_resize(ctx) < 0) {
    return -1;
  }

  ctx->outputbuf.buf[ctx->outputbuf.len++] = c;
  ctx->outputbuf.buf[ctx->outputbuf.len] = '\0';
  return 0;
}

int output_buf_append_str(ParserContext *ctx, char *str) {
  size_t str_len = strlen(str);
  if (ctx->outputbuf.capacity - ctx->outputbuf.len - 1 < str_len &&
      output_buf_resize(ctx) < 0) {
    return -1;
  }

  memcpy(ctx->outputbuf.buf + ctx->outputbuf.len, str, str_len);
  ctx->outputbuf.len += str_len;
  ctx->outputbuf.buf[ctx->outputbuf.len] = '\0';
  return 0;
}
