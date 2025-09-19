#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdio.h>

#define KB 1024

typedef int (*TrimPredicate)(char c);

char *readfile(FILE *fp, size_t *out_len);
char *trim(char *str, TrimPredicate should_trim);
void trim_in_place(char *str, TrimPredicate should_trim);

#endif
