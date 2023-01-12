
/**
 * @file lexer.c
 * @author Devin Arena
 * @brief Lexes tokens to be parsed by the parser.
 * @since 1/6/2023
 **/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  enum TokenType type = TOKEN_LITERAL_INTEGER;
  int start = lexer.index;
  while (isdigit(peek()))
    lexer.index++;

  // check for a decimal point
  if (peek() == '.' && isdigit(peek_next())) {
    type = TOKEN_LITERAL_FLOATING;
    // consume the decimal point
    lexer.index++;

    while (isdigit(peek()))
      lexer.index++;
  }

  int length = lexer.index - start;
  char* buffer = malloc(sizeof(char) * (length + 1));
  memcpy(buffer, &lexer.input[start], length);
  buffer[length] = '\0';

  return token_new(type, buffer, lexer.line);
}

static Token* identifier() {
  int start = lexer.index;
  lexer.index++;
  while (isalnum(peek()))
    lexer.index++;

  int length = lexer.index - start;
  char* buffer = malloc(sizeof(char) * (length + 1));
  memcpy(buffer, &lexer.input[start], length);
  buffer[length] = '\0';

  if (strcmp(buffer, "bool") == 0) {
    return token_new(TOKEN_BOOL, buffer, lexer.line);
  } else if (strcmp(buffer, "false") == 0) {
    return token_new(TOKEN_FALSE, buffer, lexer.line);
  } else if (strcmp(buffer, "i32") == 0) {
    return token_new(TOKEN_I32, buffer, lexer.line);
  } else if (strcmp(buffer, "if") == 0) {
    return token_new(TOKEN_IF, buffer, lexer.line);
  } else if (strcmp(buffer, "null") == 0) {
    return token_new(TOKEN_NULL, buffer, lexer.line);
  } else if (strcmp(buffer, "print") == 0) {
    return token_new(TOKEN_PRINT, buffer, lexer.line);
  } else if (strcmp(buffer, "true") == 0) {
    return token_new(TOKEN_TRUE, buffer, lexer.line);
  }

  return token_new(TOKEN_IDENTIFIER, buffer, lexer.line);
}

/**
 * @brief Helper for generating tokens from the given string.
 *
 * @param type the type of token to generate
 * @param c the character to generate the token from
 * @return Token* the token generated
 */
static Token* make_token(enum TokenType type, const char* str, int length) {
  char* buffer = malloc(sizeof(char) * (length + 1));
  strcpy(buffer, str);
  buffer[length] = '\0';
  return token_new(type, buffer, lexer.line);
}

static Token* single_char_token(enum TokenType type, char c) {
  lexer.index++;
  return make_token(type, &c, 1);
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

  if (isalpha(c)) {
    return identifier();
  }

  switch (c) {
    case '!': {
      if (peek_next() == '=') {
        lexer.index += 2;
        return make_token(TOKEN_NOT_EQUAL, "!=", 2);
      }
      return single_char_token(TOKEN_EXCLAMATION, c);
    }
    case '+':
      return single_char_token(TOKEN_PLUS, c);
    case '-':
      return single_char_token(TOKEN_MINUS, c);
    case '*':
      return single_char_token(TOKEN_STAR, c);
    case '/':
      return single_char_token(TOKEN_SLASH, c);
    case '=': {
      if (peek_next() == '=') {
        lexer.index += 2;
        return make_token(TOKEN_EQUAL_EQUAL, "==", 2);
      }
      return single_char_token(TOKEN_EQUAL, c);
    }
    case '(':
      return single_char_token(TOKEN_LPAREN, c);
    case ')':
      return single_char_token(TOKEN_RPAREN, c);
    case '{':
      return single_char_token(TOKEN_LBRACE, c);
    case '}':
      return single_char_token(TOKEN_RBRACE, c);
    case ';': {
      return single_char_token(TOKEN_SEMICOLON, c);
    }
    case '>': {
      if (peek_next() == '=') {
        lexer.index += 2;
        return make_token(TOKEN_GREATER_EQUAL, ">=", 2);
      }
      return single_char_token(TOKEN_GREATER, c);
    }
    case '<': {
      if (peek_next() == '=') {
        lexer.index += 2;
        return make_token(TOKEN_LESS_EQUAL, "<=", 2);
      }
      return single_char_token(TOKEN_LESS, c);
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