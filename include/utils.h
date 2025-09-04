#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdio.h>

#define KB 1024

char *readfile(FILE *fp, size_t *out_len);
void trim_whitespace(char *str);

#endif
