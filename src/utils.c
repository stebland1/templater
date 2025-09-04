#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

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
