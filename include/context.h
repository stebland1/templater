#ifndef CONTEXT_H
#define CONTEXT_H

#include "parser.h"
#include <stddef.h>

#define TAG_CAPACITY 64

typedef struct {
  char *buf;
  size_t len;
  size_t capacity;
} Outbuf;

typedef struct {
  char tag[TAG_CAPACITY];
  size_t tag_len;
  ParseState state;
  Outbuf outputbuf;
  char *submodule_dir;
} ParserContext;

int context_init(ParserContext *ctx, char *submodule_dir);
int output_buf_append_str(ParserContext *ctx, char *str);
int output_buf_append_char(ParserContext *ctx, char c);

#endif
