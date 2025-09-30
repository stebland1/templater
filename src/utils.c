#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Reads from the passed in file descriptor and puts into an allocated buffer.
 *
 * Returns said buffer, responsibility of freeing this resource is left up to
 * the caller.
 */
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
      size_t newbuf_size = buf_size + KB;
      char *newbuf = realloc(buf, newbuf_size);
      if (!newbuf) {
        free(buf);
        return NULL;
      }

      buf = newbuf;
      buf_size = newbuf_size;
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

char *trim(char *str, TrimPredicate should_trim) {
  if (!str) {
    return NULL;
  }

  char *start = str;
  while (should_trim(*start)) {
    start++;
  }

  if (*start == '\0') {
    char *empty = malloc(1);
    if (!empty) {
      perror("malloc");
      return NULL;
    }
    empty[0] = '\0';
    return empty;
  }

  char *end = start + strlen(start) - 1;
  while (should_trim(*end)) {
    end--;
  }

  size_t len = end - start + 1;
  char *out = malloc(len + 1);
  if (!out) {
    perror("malloc");
    return NULL;
  }

  memcpy(out, start, len);
  out[len] = '\0';
  return out;
}

void trim_in_place(char *str, TrimPredicate should_trim) {
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
