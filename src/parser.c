#include "parser.h"
#include "context.h"
#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

    return p + 2;
  }

  ctx->tag[ctx->tag_len++] = *p;
  if (ctx->tag_len >= TAG_CAPACITY - 1) {
    if (flush_tag_to_output_buf(ctx) == -1) {
      return NULL;
    }

    ctx->state = CTX_SCANNING;
  }

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
