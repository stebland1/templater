#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *readfile(FILE *fp, size_t *out_len) {
  size_t buf_size = KB;
  char *buf = malloc(buf_size);
  if (!buf) {
    return NULL;
  }

  size_t n;
  size_t total_read = 0;
  while ((n = fread(buf + total_read, sizeof(*buf), buf_size - total_read,
                    fp)) > 0) {
    total_read += n;

    if (total_read == buf_size) {
      char *newbuf = realloc(buf, buf_size + KB);
      if (!newbuf) {
        free(buf);
        return NULL;
      }
    }
  }

  if (ferror(fp)) {
    free(buf);
    return NULL;
  }

  *out_len = total_read;
  buf[total_read] = '\0';
  return buf;
}

void trim(char *str, TrimPredicate should_trim) {
  if (str == NULL) {
    return;
  }

  char *start = str;
  char *end = str + strlen(str) - 1;

  while (should_trim(*start)) {
    start++;
  }

  while (should_trim(*end)) {
    end--;
  }

  if (start > end) {
    *str = '\0';
    return;
  }

  *(end + 1) = '\0';
  size_t n = ((end + 1) - start) + 1;
  if (str != start) {
    memmove(str, start, n);
  }
}
