#include "parser.h"
#include "context.h"
#include "utils.h"
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

char *handle_scan(ParserContext *pctx, FileContext *fctx, char *p) {
  if (is_opening_tag(p)) {
    fctx->state = CTX_PARSING;
    p += 2;
  } else {
    if (output_buf_append_char(pctx, *p) < 0) {
      fprintf(stderr, "Failure to append character to output buf\n");
      return NULL;
    }
    p++;
  }

  return p;
}

int flush_tag_to_output_buf(ParserContext *ctx, FileContext *fctx) {
  // TODO: there may be times where we want the suffix too.
  // Might need a rethink later. maybe a couple of bool flags.
  if (output_buf_append_str(ctx, "{{") < 0 ||
      output_buf_append_str(ctx, fctx->tag) < 0) {
    return -1;
  }

  fctx->tag[0] = '\0';
  fctx->tag_len = 0;
  return 0;
}

char *handle_parse(ParserContext *pctx, FileContext *fctx, char *p) {
  if (is_opening_tag(p)) {
    fctx->tag[0] = '\0';
    fctx->tag_len = 0;
    return p + 2;
  }

  if (is_closing_tag(p)) {
    if (output_buf_append_str(pctx, "TAG GOES HERE!") < 0) {
      return NULL;
    }

    char submodule_path[PATH_MAX];
    trim_whitespace(fctx->tag);
    size_t n = snprintf(submodule_path, sizeof(submodule_path), "%s/%s.html",
                        pctx->submodule_dir, fctx->tag);
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

  fctx->tag[fctx->tag_len++] = *p;
  if (fctx->tag_len >= TAG_CAPACITY - 1) {
    if (flush_tag_to_output_buf(pctx, fctx) == -1) {
      return NULL;
    }

    fctx->state = CTX_SCANNING;
  }

  fctx->tag[fctx->tag_len] = '\0';
  return p + 1;
}

int parse_file(ParserContext *pctx, FILE *fp) {
  size_t buf_len;
  char *buf = readfile(fp, &buf_len);
  if (!buf) {
    return -1;
  }

  FileContext fctx;
  fctx.tag[0] = '\0';
  fctx.tag_len = 0;
  fctx.state = CTX_SCANNING;

  char *p = buf;
  while (*p) {
    switch (fctx.state) {
    case CTX_SCANNING:
      p = handle_scan(pctx, &fctx, p);
      if (!p) {
        return -1;
      }
      break;
    case CTX_PARSING:
      p = handle_parse(pctx, &fctx, p);
      if (!p) {
        return -1;
      }
      break;
    }
  }

  printf("Output: %s\n", pctx->ob.buf);
  fclose(fp);
  return 0;
}
