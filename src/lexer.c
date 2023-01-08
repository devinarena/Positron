
/**
 * @file lexer.c
 * @author Devin Arena
 * @brief Lexes tokens to be parsed by the parser.
 * @since 1/6/2023
 **/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "token.h"

Lexer lexer;

/**
 * @brief Returns the current character in the source string.
 */
static char peek() {
  return lexer.input[lexer.index];
}

/**
 * @brief Returns the next character in the source string.
 */
static char peek_next() {
  // we should not be peeking past the end of the string
  if (lexer.input[lexer.index] == '\0')
    return '\0';
  return lexer.input[lexer.index + 1];
}

/**
 * @brief Initializes the lexer.
 *
 * @param input the input string to be lexed
 */
void lexer_init(const char* input) {
  lexer.input = input;
  lexer.line = 1;
  lexer.index = 0;
}

/**
 * @brief Skips whitespace in the source string.
 */
static void skip_whitespace() {
  char c = peek();

  // skip specific whitespace characters
  while (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
    // we still need to track the line number
    if (c == '\n')
      lexer.line++;
    lexer.index++;
    c = peek();
  }
}

/**
 * @brief Helper function for parsing a number token.
 *
 * @return Token* the token representing the number (floating point or integer)
 */
static Token* number() {
  char c;
  char* buffer = malloc(sizeof(char) * 40);
  int i = 0;
  enum TokenType type = TOKEN_LITERAL_INTEGER;

  while (isdigit(c = peek())) {
    buffer[i++] = c;
    lexer.index++;
  }

  if (peek() == '.' && isdigit(peek_next())) {
    type = TOKEN_LITERAL_FLOATING;
    buffer[i++] = '.';
    lexer.index++;

    while (isdigit(c = peek())) {
      buffer[i++] = c;
      lexer.index++;
    }
  }

  buffer[i] = '\0';

  return token_new(type, buffer, lexer.line);
}

/**
 * @brief Scans the next token and returns it.
 */
Token* lexer_next_token() {
  skip_whitespace();

  char c = peek();

  // handle the case for number literals (floating point or integer)
  if (isdigit(c)) {
    return number();
  }

  switch (c) {
    case '-': {
      char* buffer = malloc(sizeof(char) * 2);
      buffer[0] = '-';
      buffer[1] = '\0';
      lexer.index++;
      return token_new(TOKEN_MINUS, buffer, lexer.line);
    }
    case '\0': {
      char* buffer = malloc(sizeof(char));
      buffer[0] = '\0';
      return token_new(TOKEN_EOF, buffer, lexer.line);
    }
    default:
      printf("Unexpected character: %c", c);
      exit(1);
  }
}