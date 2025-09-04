#include "parser.h"
#include "context.h"
#include "utils.h"
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_opening_tag(char *p) {
  return *p && *p == '{' && *(p + 1) && *(p + 1) == '{';
}

int is_closing_tag(char *p) {
  return *p && *p == '}' && *(p + 1) && *(p + 1) == '}';
}

char *handle_scan(ParserContext *ctx, char *p) {
  if (is_opening_tag(p)) {
    ctx->state = CTX_PARSING;
    p += 2;
  } else {
    if (output_buf_append_char(ctx, *p) < 0) {
      fprintf(stderr, "Failure to append character to output buf\n");
      return NULL;
    }
    p++;
  }

  return p;
}

int flush_tag_to_output_buf(ParserContext *ctx) {
  // TODO: there may be times where we want the suffix too.
  // Might need a rethink later. maybe a couple of bool flags.
  if (output_buf_append_str(ctx, "{{") < 0 ||
      output_buf_append_str(ctx, ctx->tag) < 0) {
    return -1;
  }

  ctx->tag[0] = '\0';
  ctx->tag_len = 0;
  return 0;
}

char *handle_parse(ParserContext *ctx, char *p) {
  if (is_opening_tag(p)) {
    ctx->tag[0] = '\0';
    ctx->tag_len = 0;
    return p + 2;
  }

  if (is_closing_tag(p)) {
    printf("Finding submodule %s\n", ctx->tag);
    if (output_buf_append_str(ctx, "TAG GOES HERE!") < 0) {
      return NULL;
    }

    char submodule_path[PATH_MAX];
    trim_whitespace(ctx->tag);
    size_t n = snprintf(submodule_path, sizeof(submodule_path), "%s/%s.html",
                        ctx->submodule_dir, ctx->tag);
    if (n < 0 || n >= PATH_MAX) {
      fprintf(stderr, "The submodule path is too long, exceeding %d bytes\n",
              PATH_MAX);
      return NULL;
    }

    FILE *fp = fopen(submodule_path, "r");
    if (!fp) {
      perror("fopen");
      fprintf(stderr, "Failure to open file at %s\n", submodule_path);
      return NULL;
    }

    printf("Opened file at %s!\n", submodule_path);
    return p + 2;
  }

  ctx->tag[ctx->tag_len++] = *p;
  if (ctx->tag_len >= TAG_CAPACITY - 1) {
    if (flush_tag_to_output_buf(ctx) == -1) {
      return NULL;
    }

    ctx->state = CTX_SCANNING;
  }

  ctx->tag[ctx->tag_len] = '\0';
  return p + 1;
}

int parse_file(FILE *fp, char *submodule_dir) {
  size_t buf_len;
  char *buf = readfile(fp, &buf_len);
  if (!buf) {
    return -1;
  }

  ParserContext ctx;
  if (context_init(&ctx, submodule_dir) < 0) {
    fprintf(stderr, "Failed to init context\n");
    return -1;
  }

  char *p = buf;
  while (*p) {
    switch (ctx.state) {
    case CTX_SCANNING:
      p = handle_scan(&ctx, p);
      if (!p) {
        return -1;
      }
      break;
    case CTX_PARSING:
      p = handle_parse(&ctx, p);
      if (!p) {
        return -1;
      }
      break;
    }
  }

  printf("Parsing file...\n");
  printf("Output: %s\n", ctx.outputbuf.buf);
  fclose(fp);
  return 0;
}
