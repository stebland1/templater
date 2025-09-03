#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <template path> <submodule dir>\n", argv[0]);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
