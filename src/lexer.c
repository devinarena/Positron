
/**
 * @file lexer.c
 * @author Devin Arena
 * @brief Lexes tokens to be parsed by the parser.
 * @since 1/6/2023
 **/

#include <ctype.h>
#include <stdbool.h>
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
 * @brief Helper for generating tokens from the given string.
 *
 * @param type the type of token to generate
 * @param c the character to generate the token from
 * @return Token* the token generated
 */
static Token make_token(enum TokenType type, const char* start, size_t length) {
  return token_new(type, start, length, lexer.line);
}

/**
 * @brief Helper function for parsing a number token.
 *
 * @return Token* the token representing the number (floating point or integer)
 */
static Token number() {
  enum TokenType type = TOKEN_LITERAL_INTEGER;
  const char* schar = lexer.input + lexer.index;
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

  return make_token(type, schar, length);
}

static Token identifier() {
  const char* schar = lexer.input + lexer.index;
  int start = lexer.index;
  lexer.index++;
  while (isalnum(peek()))
    lexer.index++;

  int length = lexer.index - start;

  if (strncmp(schar, "bool", 4) == 0) {
    return make_token(TOKEN_BOOL, schar, length);
  } else if (strncmp(schar, "false", 5) == 0) {
    return make_token(TOKEN_FALSE, schar, length);
  } else if (strncmp(schar, "i32", 3) == 0) {
    return make_token(TOKEN_I32, schar, length);
  } else if (strncmp(schar, "if", 2) == 0) {
    return make_token(TOKEN_IF, schar, length);
  } else if (strncmp(schar, "null", 4) == 0) {
    return make_token(TOKEN_NULL, schar, length);
  } else if (strncmp(schar, "print", 5) == 0) {
    return make_token(TOKEN_PRINT, schar, length);
  } else if (strncmp(schar, "str", 3) == 0) {
    return make_token(TOKEN_STR, schar, length);
  } else if (strncmp(schar, "true", 4) == 0) {
    return make_token(TOKEN_TRUE, schar, length);
  } else if (strncmp(schar, "while", 5) == 0) {
    return make_token(TOKEN_WHILE, schar, length);
  }

  return make_token(TOKEN_IDENTIFIER, schar, length);
}

static Token string() {
  lexer.index++;
  const char* schar = lexer.input + lexer.index;
  int start = lexer.index;
  bool quote = false;
  while (peek() != '\0') {
    if (peek() == '\n')
      lexer.line++;
    else if (peek() == '"') {
      quote = true;
      lexer.index++;
      break;
    }
    lexer.index++;
  }
  if (peek() == '\0' && !quote) {
    printf("Error: unterminated string literal on line %d", lexer.line);
    exit(1);
  }
  int length = (lexer.index - start - 1);
  return make_token(TOKEN_LITERAL_STRING, schar, length);
}

/**
 * @brief Scans the next token and returns it.
 */
Token lexer_next_token() {
  skip_whitespace();

  const char* schar = lexer.input + lexer.index;
  char c = peek();

  // handle the case for number literals (floating point or integer)
  if (isdigit(c)) {
    return number();
  }

  if (isalpha(c)) {
    return identifier();
  }

  Token token = make_token(TOKEN_ERROR, schar, 0);

  switch (c) {
    case '!': {
      if (peek_next() == '=') {
        token = make_token(TOKEN_NOT_EQUAL, schar, 2);
        lexer.index += 2;
        break;
      }
      token = make_token(TOKEN_EXCLAMATION, schar, 1);
      lexer.index++;
      break;
    }
    case '+':
      token = make_token(TOKEN_PLUS, schar, 1);
      lexer.index++;
      break;
    case '-':
      token = make_token(TOKEN_MINUS, schar, 1);
      lexer.index++;
      break;
    case '*':
      token = make_token(TOKEN_STAR, schar, 1);
      lexer.index++;
      break;
    case '/':
      if (peek_next() == '/') {
        while (peek() != '\0' && peek() != '\n')
          lexer.index++;
        token = lexer_next_token();
      } else {
        token = make_token(TOKEN_SLASH, schar, 1);
        lexer.index++;
      }
      break;
    case '=': {
      if (peek_next() == '=') {
        token = make_token(TOKEN_EQUAL_EQUAL, schar, 2);
        lexer.index += 2;
        break;
      }
      token = make_token(TOKEN_EQUAL, schar, 1);
      lexer.index++;
      break;
    }
    case '(':
      token = make_token(TOKEN_LPAREN, schar, 1);
      lexer.index++;
      break;
    case ')':
      token = make_token(TOKEN_RPAREN, schar, 1);
      lexer.index++;
      break;
    case '{':
      token = make_token(TOKEN_LBRACE, schar, 1);
      lexer.index++;
      break;
    case '}':
      token = make_token(TOKEN_RBRACE, schar, 1);
      lexer.index++;
      break;
    case ';': {
      token = make_token(TOKEN_SEMICOLON, schar, 1);
      lexer.index++;
      break;
    }
    case '>': {
      if (peek_next() == '=') {
        token = make_token(TOKEN_GREATER_EQUAL, schar, 2);
        lexer.index += 2;
        break;
      }
      token = make_token(TOKEN_GREATER, schar, 1);
      lexer.index++;
      break;
    }
    case '<': {
      if (peek_next() == '=') {
        token = make_token(TOKEN_LESS_EQUAL, schar, 2);
        lexer.index += 2;
        break;
      }
      token = make_token(TOKEN_LESS, schar, 1);
      lexer.index++;
      break;
    }
    case '"': {
      token = string();
      break;
    }
    case '&': {
      if (peek_next() == '&') {
        token = make_token(TOKEN_AND, schar, 2);
        lexer.index += 2;
        break;
      }
      printf("Unexpected character: %c\n", c);
      exit(1);
    }
    case '|': {
      if (peek_next() == '|') {
        token = make_token(TOKEN_OR, schar, 2);
        lexer.index += 2;
        break;
      }
      printf("Unexpected character: %c\n", c);
      exit(1);
    }
    case '\0': {
      token = make_token(TOKEN_EOF, schar, 1);
      lexer.index++;
      break;
    }
    default:
      printf("Unexpected character: %c\n", c);
      exit(1);
  }

  return token;
}