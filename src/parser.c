#include "parser.h"
#include "context.h"
#include "utils.h"
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
  if (ob_append_str(pctx, "{{") < 0 || ob_append_str(pctx, fctx->tag) < 0) {
    return -1;
  }

  fctx->tag[0] = '\0';
  fctx->tag_len = 0;
  return 0;
}

int build_submodule_path(char *buf, size_t buf_len, ParserContext *pctx,
                         char *trimmed_tag) {
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

/*
 * Here, a potential tag has been found.
 * Try to open the file associated, and parse it.
 */
int resolve_tag(ParserContext *pctx, FileContext *fctx) {
  fctx->tag[fctx->tag_len] = '\0';

  char *trimmed_tag = trim(fctx->tag, is_whitespace);
  if (!trimmed_tag) {
    return -1;
  }

  FILE *fp = NULL;
  if (strcmp(trimmed_tag, BODY_TAG) == 0) {
    if (isatty(STDIN_FILENO)) {
      fprintf(stderr, "stdin is a terminal — no piped input detected.\n");
      return -1;
    }
    fp = stdin;
    goto parse;
  }

  char submodule_path[PATH_MAX];
  if (build_submodule_path(submodule_path, PATH_MAX, pctx, trimmed_tag) < 0) {
    return -1;
  }

  fp = fopen(submodule_path, "r");
  if (!fp) {
    fprintf(stderr, "Failed to open file %s\n", submodule_path);
    return -1;
  }

parse:
  if (parse_file(pctx, fp) < 0) {
    return -1;
  }

  fclose(fp);
  fctx->state = CTX_SCANNING;
  fctx->tag[0] = '\0';
  fctx->tag_len = 0;
  return TR_SUCCESS;
}

/*
 * Empties out the tag currently being parsed.
 *
 * Useful if we encounter another opening tag sequence `{{`.
 * This means we need to scrap what we currently have.
 */
char *reset_tag(char *p, FileContext *fctx) {
  fctx->tag[0] = '\0';
  fctx->tag_len = 0;
  return p + 2;
}

char *handle_closing_tag(char *p, ParserContext *pctx, FileContext *fctx) {
  if (resolve_tag(pctx, fctx) < 0) {
    fprintf(stderr, "Failure to resolve tag `%s`, Attempting to proceed...\n",
            fctx->tag);

    if (flush_tag_to_output_buf(pctx, fctx) < 0) {
      fprintf(stderr, "Failure to flush tag `%s` to output buffer\n",
              fctx->tag);
      return NULL;
    }

    fctx->state = CTX_SCANNING;
    return p;
  }

  fctx->state = CTX_SCANNING;
  /* We've successfully resolved the tag, skip over the closing tag */
  return p + 2;
}

char *handle_parse_tag_char(char *p, ParserContext *pctx, FileContext *fctx) {
  if (fctx->tag_len >= TAG_CAPACITY - 1) {
    printf("We've exceeded the tag capacity with tag `%s`, len of %zu\n",
           fctx->tag, fctx->tag_len);
    if (flush_tag_to_output_buf(pctx, fctx) == -1) {
      fprintf(stderr, "Failure flushing tag to out buf\n");
      return NULL;
    }

    fctx->state = CTX_SCANNING;
    return p;
  }

  fctx->tag[fctx->tag_len++] = *p;
  fctx->tag[fctx->tag_len] = '\0';
  return p + 1;
}

/*
 * When parsing, there's 3 cases.
 *
 * 1. We're on an opening tag {{.
 * 2. We're on a closing tag }}.
 * 3. It's a regular character, and part of the tag being built.
 */
char *handle_parse_tag(ParserContext *pctx, FileContext *fctx, char *p) {
  if (is_opening_tag(p)) {
    return reset_tag(p, fctx);
  }

  if (is_closing_tag(p)) {
    return handle_closing_tag(p, pctx, fctx);
  }

  return handle_parse_tag_char(p, pctx, fctx);
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

  free(buf);
  fclose(fp);
  return 0;
}
