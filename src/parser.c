
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

static bool match(enum TokenType type) {
  if (parser.current->type == type) {
    parser.previous = parser.current;
    parser.current = lexer_next_token();
    return true;
  }
  return false;
}

void parser_init(Block* block) {
  parser.block = block;
  parser.current = NULL;
  parser.previous = NULL;

  advance();
}

void expression() {
  if (match(TOKEN_LITERAL_INTEGER)) {
    Value* value = value_new_int_32(atoi(parser.current->lexeme));
    uint8_t index = block_new_constant(parser.block, value);
    block_new_opcodes(parser.block, OP_CONSTANT_INTEGER_32, index);
  }
}