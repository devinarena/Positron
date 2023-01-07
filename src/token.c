
/**
 * @file token.c
 * @author Devin Arena
 * @brief Contains token information for the lexer.
 * @since 1/6/2023
 **/

#include <stdio.h>
#include <stdlib.h>

#include "token.h"

/**
 * @brief Allocates a new token.
 *
 * @param type the type of the token
 * @param lexeme the lexeme of the token
 * @param line the line the token was found on
 */
Token* token_new(enum TokenType type, const char* lexeme, int line) {
  Token* token = malloc(sizeof(Token));
  token->type = type;
  token->lexeme = lexeme;
  token->line = line;
  return token;
}

/**
 * @brief Prints a token's data including the enum type.
 *
 * @param token the token to print
 */
void token_print(Token* token) {
  switch (token->type) {
    case TOKEN_LITERAL_INTEGER:
      printf("Token(TOKEN_LITERAL_INTEGER, %s, %d)\n", token->lexeme, token->line);
      break;
    case TOKEN_LITERAL_FLOATING:
      printf("Token(TOKEN_LITERAL_FLOATING, %s, %d)\n", token->lexeme, token->line);
      break;
    case TOKEN_EOF:
      printf("Token(TOKEN_EOF, \\0, %d)\n", token->line);
      return;
  }
}

/**
 * @brief Frees a token.
 *
 * @param token the token to free
 */
void token_free(Token* token) {
  free((void*)token->lexeme);
  free((void*)token);
}