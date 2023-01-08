
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
#include "value.h"

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
static bool check(enum TokenType type) {
  if (parser.current->type == type) {
    return true;
  }
  return false;
}

/**
 * @brief Checks if the current token matches the given token type and advances
 * if so.
 *
 * @param type The token type to match.
 * @return true If the current token matches the given token type.
 * @return false If the current token does not match the given token type.
 */
static bool match(enum TokenType type) {
  if (check(type)) {
    advance();
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
static Value* literal() {
  if (match(TOKEN_LITERAL_INTEGER)) {
    Value* val = value_new_int_32(atoi(parser.previous->lexeme));
    uint8_t index = block_new_constant(parser.block, val);
    block_new_opcodes(parser.block, OP_CONSTANT_INTEGER_32, index);
    return val;
  } else {
    printf("Expected literal but got token of type ");
    token_type_print(parser.current->type);
    printf(" on line %d\n", parser.current->line);
    exit(1);
  }
  return NULL;
}

Value* expression();

/**
 * @brief Handles factor level parsing, including parenthesized expressions and
 * literals.
 *
 * @return Value* The value of the parsed expression.
 */
static Value* factor() {
  if (match(TOKEN_LPAREN)) {
    Value* val = expression();
    consume(TOKEN_RPAREN);
    return val;
  } else {
    return literal();
  }
}

/**
 * @brief Handles term level parsing, including multiplication and division.
 *
 * @return Value*
 */
static Value* term() {
  Value* val = factor();

  while (match(TOKEN_STAR) || match(TOKEN_SLASH)) {
    enum TokenType op = parser.previous->type;
    val = term();
    if (op == TOKEN_STAR) {
      block_new_opcode(parser.block, OP_MULTIPLY_INTEGER_32);
    } else if (op == TOKEN_SLASH) {
      block_new_opcode(parser.block, OP_DIVIDE_INTEGER_32);
    }
  }

  return val;
}

/**
 * @brief Parses a unary expression.
 */
static Value* unary() {
  Value* val = NULL;
  enum TokenType prev = parser.previous->type;
  if (prev == TOKEN_MINUS) {
    val = literal();
    block_new_opcode(parser.block, OP_NEGATE_INTEGER_32);
  }
  return val;
}

/**
 * @brief Parses an expression.
 */
Value* expression() {
  Value* val = NULL;
  // TODO: clean this up
  // prefix operators
  if (match(TOKEN_MINUS)) {
    val = unary();
  } else {
    val = term();
  }

  // infix operators
  while (match(TOKEN_PLUS) || match(TOKEN_MINUS)) {
    enum TokenType op = parser.previous->type;
    val = term();
    if (op == TOKEN_PLUS) {
      block_new_opcode(parser.block, OP_ADD_INTEGER_32);
    } else if (op == TOKEN_MINUS) {
      block_new_opcode(parser.block, OP_SUBTRACT_INTEGER_32);
    }
  }

  return val;
}