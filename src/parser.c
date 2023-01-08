
/**
 * @file parser.c
 * @author Devin Arena
 * @brief Parses tokens to be interpreted by the interpreter.
 * @since 1/6/2023
 **/

#include <stdbool.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"
#include "token.h"

Parser parser;

/**
 * @brief Advances the parser to the next token, tracking the previous token.
 */
static void advance() {
  if (parser.current && parser.current->type == TOKEN_EOF)
    return;
  parser.previous = parser.current;
  parser.current = lexer_next_token();
  token_print(parser.current);
}

/**
 * @brief Checks if the current token matches the given token type.
 *
 * @param type The token type to match.
 * @return true If the current token matches the given token type.
 * @return false If the current token does not match the given token type.
 */
static bool match(enum TokenType type) {
  if (parser.current->type == type) {
    return true;
  }
  return false;
}

/**
 * @brief Consumes the current token if it matches the given token type.
 *
 * @param type The token type to consume.
 */
static void consume(enum TokenType type) {
  if (parser.current->type == type) {
    advance();
    return;
  }
  printf("Expected token of type ");
  token_type_print(type);
  printf(" but got token of type ");
  token_type_print(parser.current->type);
  printf(" on line %d\n", parser.current->line);
  exit(1);
}

/**
 * @brief Initializes the parser with the given block.
 *
 * @param block The block to generate opcodes for.
 */
void parser_init(Block* block) {
  parser.block = block;
  parser.current = NULL;
  parser.previous = NULL;

  advance();
}

/**
 * @brief Parses a literal.
 */
static void literal() {
  if (match(TOKEN_LITERAL_INTEGER)) {
    Value* val = value_new_int_32(atoi(parser.current->lexeme));
    uint8_t index = block_new_constant(parser.block, val);
    block_new_opcodes(parser.block, OP_CONSTANT_INTEGER_32, index);
  }
}

/**
 * @brief Parses an expression.
 */
void expression() {

  literal();
}