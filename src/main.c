#include "context.h"
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

  ParserContext ctx;
  if (parse_context_init(&ctx, argv[2]) < 0) {
    fprintf(stderr, "Failed to init parser context\n");
    return -1;
  }

  if (parse_file(&ctx, fp) < 0) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
