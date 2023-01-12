
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
    default:
      printf("Token(");
      token_type_print(token->type);
      printf(", %s, %d)\n", token->lexeme, token->line);
      break;
    case TOKEN_EOF:
      printf("Token(TOKEN_EOF, \\0, %d)\n", token->line);
      return;
  }
}

/**
 * @brief Prints a token's type.
 *
 * @param type the type to print
 */
void token_type_print(enum TokenType type) {
  switch (type) {
    case TOKEN_LITERAL_FLOATING:
      printf("TOKEN_LITERAL_FLOATING");
      break;
    case TOKEN_LITERAL_INTEGER:
      printf("TOKEN_LITERAL_INTEGER");
      break;
    case TOKEN_LITERAL_STRING:
      printf("TOKEN_LITERAL_STRING");
      break;
    case TOKEN_BOOL:
      printf("TOKEN_BOOL");
      break;
    case TOKEN_FALSE:
      printf("TOKEN_FALSE");
      break;
    case TOKEN_I32:
      printf("TOKEN_I32");
      break;
    case TOKEN_IF:
      printf("TOKEN_IF");
      break;
    case TOKEN_NULL:
      printf("TOKEN_NULL");
      break;
    case TOKEN_PRINT:
      printf("TOKEN_PRINT");
      break;
    case TOKEN_TRUE:
      printf("TOKEN_TRUE");
      break;
    case TOKEN_IDENTIFIER:
      printf("TOKEN_IDENTIFIER");
      break;
    case TOKEN_EXCLAMATION:
      printf("TOKEN_EXCLAMATION");
      break;
    case TOKEN_PLUS:
      printf("TOKEN_PLUS");
      break;
    case TOKEN_MINUS:
      printf("TOKEN_MINUS");
      break;
    case TOKEN_STAR:
      printf("TOKEN_STAR");
      break;
    case TOKEN_SLASH:
      printf("TOKEN_SLASH");
      break;
    case TOKEN_EQUAL:
      printf("TOKEN_EQUAL");
      break;
    case TOKEN_LPAREN:
      printf("TOKEN_LPAREN");
      break;
    case TOKEN_RPAREN:
      printf("TOKEN_RPAREN");
      break;
    case TOKEN_LBRACE:
      printf("TOKEN_LBRACE");
      break;
    case TOKEN_RBRACE:
      printf("TOKEN_RBRACE");
      break;
    case TOKEN_GREATER:
      printf("TOKEN_GREATER");
      break;
    case TOKEN_LESS:
      printf("TOKEN_LESS");
      break;
    case TOKEN_SEMICOLON:
      printf("TOKEN_SEMICOLON");
      break;
    case TOKEN_EOF:
      printf("TOKEN_EOF");
      break;
    case TOKEN_EQUAL_EQUAL:
      printf("TOKEN_EQUAL_EQUAL");
      break;
    case TOKEN_NOT_EQUAL:
      printf("TOKEN_NOT_EQUAL");
      break;
    case TOKEN_GREATER_EQUAL:
      printf("TOKEN_GREATER_EQUAL");
      break;
    case TOKEN_LESS_EQUAL:
      printf("TOKEN_LESS_EQUAL");
      break;
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