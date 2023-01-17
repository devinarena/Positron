

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
  size_t len = strlen(argv[1]);
  char* name = malloc(len + 1);
  strcpy(name, argv[1]);
  name[len] = '\0';

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
    printf("1");
  }

  parser_free();
  printf("2");
  // block_free(block);
  printf("3");
  free((void*)source);
  printf("4");

  return 0;
}