#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdio.h>

typedef enum {
  CTX_SCANNING,
  CTX_PARSING,
} ParseState;

int parse_file(FILE *fp, char *submodule_dir);

#endif
