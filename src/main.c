#include "parser.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <template path> <submodule dir>\n", argv[0]);
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");
  if (!fp) {
    perror("fopen");
    return EXIT_FAILURE;
  }

  if (parse_file(fp, argv[2]) < 0) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
