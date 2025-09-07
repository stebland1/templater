#ifndef PARSER_H
#define PARSER_H

#include "context.h"
#include <stddef.h>
#include <stdio.h>

#define MAX_FILE_PATH 64

int is_opening_tag(char *p);
int is_closing_tag(char *p);
char *handle_scan(ParserContext *pctx, FileContext *fctx, char *p);
char *handle_parse(ParserContext *pctx, FileContext *fctx, char *p);
int flush_tag_to_output_buf(ParserContext *ctx, FileContext *fctx);
int parse_file(ParserContext *pctx, FILE *fp);

#endif
