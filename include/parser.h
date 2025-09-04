#ifndef PARSER_H
#define PARSER_H

#include "context.h"
#include <stddef.h>
#include <stdio.h>

#define MAX_FILE_PATH 64

int parse_file(ParserContext *pctx, FILE *fp);

#endif
