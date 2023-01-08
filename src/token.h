
/**
 * @file token.h
 * @author Devin Arena
 * @brief 1/6/2023
 * @since Contains token information for the lexer.
 **/

#ifndef TOKEN_H
#define TOKEN_H

#define IS_NUM(c) (c - '0' >= 0 && c - '0' <= 9)

enum TokenType { TOKEN_LITERAL_INTEGER, TOKEN_LITERAL_FLOATING, TOKEN_EOF };

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