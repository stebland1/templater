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

char *build_submodule_path(char *buf, size_t buf_len, ParserContext *pctx,
                           FileContext *fctx) {
  // here we know the output buffer is only MAX_TAG_LEN
  // this needs to create a new tag, and then release it once done.
  // because the submodule path needs to be built without spaces.
  // and if failure, we need to restore the tag text as it was. With the
  // spaces.
  trim(fctx->tag, is_whitespace);
  size_t n =
      snprintf(buf, buf_len, "%s/%s.html", pctx->submodule_dir, fctx->tag);
  if (n < 0 || n >= PATH_MAX) {
    fprintf(stderr, "The submodule path is too long, exceeding %d bytes\n",
            PATH_MAX);
    return NULL;
  }

  return buf;
}

FILE *open_submodule_file(FILE *fp, ParserContext *pctx, FileContext *fctx) {
  // here we open the submodule file and return back the file pointer.
  char submodule_path[PATH_MAX];
  if (build_submodule_path(submodule_path, PATH_MAX, pctx, fctx) == NULL) {
    return NULL;
  }

  fp = fopen(submodule_path, "r");
  return fp;
}

// 0 == SUCCESS
// -1 == CANNOT RECOVER, ABANDON SHIP.
// 1 == RECOVER BY PUSHING TAG TO OB.
int resolve_tag(ParserContext *pctx, FileContext *fctx) {
  char submodule_path[PATH_MAX];
  if (build_submodule_path(submodule_path, PATH_MAX, pctx, fctx) == NULL) {
    // recover by inserting the tag back.
    return 1;
  }

  FILE *fp = NULL;
  if ((fp = open_submodule_file(fp, pctx, fctx)) == NULL) {
    if (flush_tag_to_output_buf(pctx, fctx) < 0) {
      fprintf(stderr, "Failure flushing tag to out buf\n");
      // memory error, we can't recover.
      return -1;
    }

    // recover by inserting the tag back.
    // we'll need to insert the }}
    return 1;
  }

  if (parse_file(pctx, fp) < 0) {
    // TODO: safely recover by dumping the tag buffer into the OB.
    // this time I'll need the suffix.
  }

  fclose(fp);
  fctx->state = CTX_SCANNING;
  return 0;
}

char *handle_parse_tag(ParserContext *pctx, FileContext *fctx, char *p) {
  if (is_opening_tag(p)) {
    fctx->tag[0] = '\0';
    fctx->tag_len = 0;
    return p + 2;
  }

  if (is_closing_tag(p)) {
    return resolve_tag(pctx, fctx) == 0 ? p + 2 : NULL;
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
  trim(buf, is_whitespace);

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
