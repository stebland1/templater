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

  ParserContext pctx;
  if (parse_context_init(&pctx, argv[2]) < 0) {
    fprintf(stderr, "Failed to init parser context\n");
    return -1;
  }

  if (parse_file(&pctx, fp) < 0) {
    parse_context_destroy(&pctx);
    return EXIT_FAILURE;
  }

  printf("%s\n", pctx.ob.buf);
  parse_context_destroy(&pctx);
  return EXIT_SUCCESS;
}
