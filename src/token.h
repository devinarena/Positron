
/**
 * @file token.h
 * @author Devin Arena
 * @brief 1/6/2023
 * @since Contains token information for the lexer.
 **/

#ifndef POSITRON_TOKEN_H
#define POSITRON_TOKEN_H

enum TokenType {
  TOKEN_NONE,
  TOKEN_ERROR,

  // literals
  TOKEN_LITERAL_INTEGER,
  TOKEN_LITERAL_FLOATING,
  TOKEN_LITERAL_STRING,
  
  // keywords
  TOKEN_BOOL,
  TOKEN_ELSE,
  TOKEN_EXIT,
  TOKEN_FALSE,
  TOKEN_FUN,
  TOKEN_FOR,
  TOKEN_IF,
  TOKEN_LET,
  TOKEN_NULL,
  TOKEN_PRINT,
  TOKEN_RETURN,
  TOKEN_STRUCT,
  TOKEN_TRUE,
  TOKEN_WHILE,

  TOKEN_IDENTIFIER,

  // single character tokens
  TOKEN_EXCLAMATION,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_EQUAL,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_LBRACKET,
  TOKEN_RBRACKET,
  TOKEN_GREATER,
  TOKEN_LESS,
  TOKEN_COMMA,
  TOKEN_DOT,
  TOKEN_COLON,
  // Thinking semi-colons will be optional for now
  TOKEN_SEMICOLON,

  // double character tokens
  TOKEN_EQUAL_EQUAL,
  TOKEN_NOT_EQUAL,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS_EQUAL,
  TOKEN_AND,
  TOKEN_OR,

  TOKEN_EOF
};

typedef struct Token {
  enum TokenType type;
  const char* start;
  int length;
  int line;
} Token;

// allocates a new token
Token token_new(enum TokenType type, const char* start, size_t length, int line);
// prints a token's data
void token_print(Token* token);
// prints a token's type
void token_type_print(enum TokenType type);
void token_print_lexeme(Token* token);

#endif