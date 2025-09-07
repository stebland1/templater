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

int parse_context_init(ParserContext *pctx, char *submodule_dir);
int ob_append_str(ParserContext *pctx, char *str);
int ob_append_char(ParserContext *pctx, char c);
int ob_resize(ParserContext *pctx);
void init_file_context(FileContext *fctx);

#endif
