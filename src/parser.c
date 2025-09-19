#include "parser.h"
#include "context.h"
#include "utils.h"
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_opening_tag(char *p) { return *p == '{' && *(p + 1) == '{'; }

int is_closing_tag(char *p) { return *p == '}' && *(p + 1) == '}'; }

int is_whitespace(char c) { return isspace(c); }

/*
 * Handles the scanning of the file for opening tags `{{`.
 */
char *handle_scan(ParserContext *pctx, FileContext *fctx, char *p) {
  if (is_opening_tag(p)) {
    fctx->state = CTX_PARSING;
    p += 2;
  } else {
    if (ob_append_char(pctx, *p) < 0) {
      fprintf(stderr, "Failure to append character to output buf\n");
      return NULL;
    }
    p++;
  }

  return p;
}

int flush_tag_to_output_buf(ParserContext *pctx, FileContext *fctx) {
  // TODO: there may be times where we want the suffix too.
  // Might need a rethink later. maybe a couple of bool flags.
  if (ob_append_str(pctx, "{{") < 0 || ob_append_str(pctx, fctx->tag) < 0) {
    // memory error, we can't recover.
    return -1;
  }

  fctx->tag[0] = '\0';
  fctx->tag_len = 0;
  return 0;
}

int build_submodule_path(char *buf, size_t buf_len, ParserContext *pctx,
                         FileContext *fctx) {
  char *trimmed_tag = trim(fctx->tag, is_whitespace);
  if (!trimmed_tag) {
    return -1;
  }

  size_t n =
      snprintf(buf, buf_len, "%s/%s.html", pctx->submodule_dir, trimmed_tag);
  free(trimmed_tag);
  if (n < 0 || n >= PATH_MAX) {
    fprintf(stderr, "The submodule path is too long, exceeding %d bytes\n",
            PATH_MAX);
    return -1;
  }

  return 0;
}

ResolveTagResult resolve_tag(ParserContext *pctx, FileContext *fctx) {
  char submodule_path[PATH_MAX];
  if (build_submodule_path(submodule_path, PATH_MAX, pctx, fctx) < 0) {
    return TR_RECOVER;
  }

  FILE *fp = fopen(submodule_path, "r");
  if (!fp) {
    if (flush_tag_to_output_buf(pctx, fctx) < 0) {
      fprintf(stderr, "Failure flushing tag to out buf\n");
      return TR_FAILURE;
    }

    fctx->state = CTX_SCANNING;
    return TR_RECOVER;
  }

  if (parse_file(pctx, fp) < 0) {
    // TODO: safely recover by dumping the tag buffer into the OB.
    // this time I'll need the suffix.
  }

  fclose(fp);
  fctx->state = CTX_SCANNING;
  return TR_SUCCESS;
}

char *handle_parse_tag(ParserContext *pctx, FileContext *fctx, char *p) {
  if (is_opening_tag(p)) {
    fctx->tag[0] = '\0';
    fctx->tag_len = 0;
    return p + 2;
  }

  if (is_closing_tag(p)) {
    return resolve_tag(pctx, fctx) == TR_FAILURE ? NULL : p + 2;
  }

  if (fctx->tag_len >= TAG_CAPACITY - 1) {
    if (flush_tag_to_output_buf(pctx, fctx) == -1) {
      return NULL;
    }

    fctx->state = CTX_SCANNING;
  }

  fctx->tag[fctx->tag_len++] = *p;
  fctx->tag[fctx->tag_len] = '\0';
  return p + 1;
}

/*
 * The parsing entry point.
 *
 * Handles the reading of the file to parse and looping over chars.
 */
int parse_file(ParserContext *pctx, FILE *fp) {
  size_t buf_len;
  char *buf = readfile(fp, &buf_len);
  if (!buf) {
    return -1;
  }

  // TODO: do this properly. trimming prefixed whitespace isn't necessary.
  trim_in_place(buf, is_whitespace);

  FileContext fctx;
  init_file_context(&fctx);

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
      p = handle_parse_tag(pctx, &fctx, p);
      if (!p) {
        return -1;
      }
      break;
    }
  }

  fclose(fp);
  return 0;
}
