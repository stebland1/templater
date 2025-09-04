#ifndef CONTEXT_H
#define CONTEXT_H

#include <stddef.h>

#define TAG_CAPACITY 64

typedef enum {
  CTX_SCANNING,
  CTX_PARSING,
} ParseState;

typedef struct {
  char *buf;
  size_t len;
  size_t capacity;
} OutBuffer;

typedef struct {
  char tag[TAG_CAPACITY];
  size_t tag_len;
  ParseState state;
} FileContext;

typedef struct {
  OutBuffer ob;
  char *submodule_dir;
} ParserContext;

int context_init(ParserContext *ctx, char *submodule_dir);
int output_buf_append_str(ParserContext *ctx, char *str);
int output_buf_append_char(ParserContext *ctx, char c);

#endif
