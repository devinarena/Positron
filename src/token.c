
/**
 * @file token.c
 * @author Devin Arena
 * @brief Contains token information for the lexer.
 * @since 1/6/2023
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

/**
 * @brief Creates a new token.
 *
 * @param type the type of the token
 * @param lexeme the lexeme of the token
 * @param line the line the token was found on
 */
Token token_new(enum TokenType type,
                const char* start,
                size_t length,
                int line) {
  Token token;
  token.type = type;
  token.start = start;
  token.length = length;
  token.line = line;
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
      printf(", ");
      token_print_lexeme(token);
      printf(", %d)", token->line);
      break;
    case TOKEN_EOF:
      printf("Token(TOKEN_EOF, \\0, %d)", token->line);
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
    case TOKEN_NONE:
      printf("TOKEN_NONE");
      break;
    case TOKEN_ERROR:
      printf("TOKEN_ERROR");
      break;
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
    case TOKEN_EXIT:
      printf("TOKEN_EXIT");
      break;
    case TOKEN_FALSE:
      printf("TOKEN_FALSE");
      break;
    case TOKEN_FOR:
      printf("TOKEN_FOR");
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
    case TOKEN_STR:
      printf("TOKEN_STR");
      break;
    case TOKEN_TRUE:
      printf("TOKEN_TRUE");
      break;
    case TOKEN_VOID:
      printf("TOKEN_VOID");
      break;
    case TOKEN_WHILE:
      printf("TOKEN_WHILE");
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
    case TOKEN_COMMA:
      printf("TOKEN_COMMA");
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
    case TOKEN_AND:
      printf("TOKEN_AND");
      break;
    case TOKEN_OR:
      printf("TOKEN_OR");
      break;
  }
}

/**
 * @brief Prints a token's name.
 *
 * @param type the token to print
 */
void token_print_lexeme(Token* token) {
  printf("%*.*s", token->length, token->length, token->start);
}