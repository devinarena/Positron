

/**
 * @file main.c
 * @author Devin Arena
 * @brief Entrypoint for the program.
 * @since 1/5/2023
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interpreter.h"
#include "lexer.h"
#include "memory.h"
#include "parser.h"

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    printf("Usage: %s <file>", argv[0]);
    return 1;
  }

  const char* source = read_file(argv[1]);
  char* name = malloc(strlen(argv[1]));
  strcpy(name, argv[1]);

  lexer_init(source);
  Block* block = block_new(name);
  parser_init(block);

  if (parse()) {
#ifdef POSITRON_DEBUG
    block_print(block);
#endif

    interpreter_init();

    interpret(block);

    interpreter_free();
  }

  parser_free();
  block_free(block);
  free((void*)source);

  return 0;
}