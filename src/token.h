
/**
 * @file token.h
 * @author Devin Arena
 * @brief 1/6/2023
 * @since Contains token information for the lexer.
 **/

#ifndef POSITRON_TOKEN_H
#define POSITRON_TOKEN_H

#define IS_NUM(c) (c - '0' >= 0 && c - '0' <= 9)

enum TokenType {
  // literals
  TOKEN_LITERAL_INTEGER,
  TOKEN_LITERAL_FLOATING,
  
  // keywords
  TOKEN_BOOL,
  TOKEN_FALSE,
  TOKEN_I32,
  TOKEN_IF,
  TOKEN_NULL,
  TOKEN_PRINT,
  TOKEN_TRUE,

  TOKEN_IDENTIFIER,

  // single character tokens
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_EQUAL,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  // Thinking semi-colons will be optional for now
  TOKEN_SEMICOLON,
  TOKEN_EOF
};

typedef struct Token {
  enum TokenType type;
  const char* lexeme;
  int line;
} Token;

// allocates a new token
Token* token_new(enum TokenType type, const char* lexeme, int line);
// prints a token's data
void token_print(Token* token);
// prints a token's type
void token_type_print(enum TokenType type);
// frees a token
void token_free(Token* token);

#endif