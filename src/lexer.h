
/**
 * @file lexer.h
 * @author Devin Arena
 * @brief Lexes an input string and returns a list of tokens
 * @since 1/6/2023
 **/

#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef struct Lexer {
  const char* input;
  int line;
  int index;
} Lexer;

// inits the lexer
void lexer_init(const char* input);
// scans the next token and returns it
Token* lexer_next_token();

#endif