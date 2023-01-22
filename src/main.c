

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
#include "positron.h"

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    printf("Usage: %s <file>", argv[0]);
    return 1;
  }

  char* path = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      DEBUG_MODE = true;
    } else if (strcmp(argv[i], "-h") == 0) {
      printf("Usage: %s <file>", argv[0]);
      exit(0);
    } else {
      if (path) {
        printf("usage: %s <file>", argv[0]);
        exit(1);
      }
      path = strdup(argv[i]);
    }
  }

  if (!path) {
    printf("Usage: %s <file>", argv[0]);
    exit(1);
  }

  const char* source = read_file(path);

  lexer_init(source);
  parser_init(path);

  InterpretResult result;

  PFunction* script = parse_script(path);

  if (script) {
    interpreter_init();

    result = interpret(script);

    interpreter_free();
  }

  parser_free();
  free((void*)source);

  return (int)result;
}