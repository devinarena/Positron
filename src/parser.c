
/**
 * @file parser.c
 * @author Devin Arena
 * @brief Parses tokens to be interpreted by the interpreter.
 * @since 1/6/2023
 **/

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
#ifdef POSITRON_DEBUG
  token_print(parser.current);
#endif
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
  // TODO: create a better error message
  parse_error("Unexpected token ");
  token_type_print(parser.current->type);
  printf("\n");
}

/**
 * @brief Produces a parse error and triggers synchronization.
 *
 * @param message The error message to print.
 */
void parse_error(const char* format, ...) {
  printf("[line %d] at '%s': ", parser.previous->line, parser.previous->lexeme);
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  parser.had_error = true;
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
  hash_table_init(&parser.globals);

  advance();
}

/**
 * @brief Frees the parser's memory.
 */
void parser_free() {
  hash_table_free(&parser.globals);
}

/**
 * @brief Syncs the parser to the next statement.
 */
static void synchronize() {
  // break on a statement end (semicolon) or begin
  while (parser.current->type != TOKEN_EOF) {
    if (parser.previous->type == TOKEN_SEMICOLON) {
      return;
    }

    switch (parser.current->type) {
      case TOKEN_PRINT:
      case TOKEN_I32:
        return;
      default:
        break;
    }

    advance();
  }
}

/**
 * @brief Parses a variable.
 */
static Value* variable() {
  Token* token = parser.previous;
  Value* val = hash_table_get(&parser.globals, token->lexeme);
  if (val == NULL) {
    printf("Undefined variable '%s' on line %d", token->lexeme, token->line);
    exit(1);
  }

  uint8_t index = block_new_constant(
      parser.block,
      value_new_object((PObject*)p_object_string_new(token->lexeme)));

  block_new_opcodes(parser.block, OP_CONSTANT, index);
  block_new_opcode(parser.block, OP_GLOBAL_GET);
  return val;
}

/**
 * @brief Parses a literal.
 */
static Value* literal() {
  if (match(TOKEN_LITERAL_INTEGER)) {
    Value* val = value_new_int_32(atoi(parser.previous->lexeme));
    uint8_t index = block_new_constant(parser.block, val);
    block_new_opcodes(parser.block, OP_CONSTANT, index);
    return val;
  } else if (match(TOKEN_NULL)) {
    Value* val = value_new_null();
    uint8_t index = block_new_constant(parser.block, val);
    block_new_opcodes(parser.block, OP_CONSTANT, index);
    return val;
  } else if (match(TOKEN_TRUE)) {
    Value* val = value_new_boolean(true);
    uint8_t index = block_new_constant(parser.block, val);
    block_new_opcodes(parser.block, OP_CONSTANT, index);
    return val;
  } else if (match(TOKEN_FALSE)) {
    Value* val = value_new_boolean(false);
    uint8_t index = block_new_constant(parser.block, val);
    block_new_opcodes(parser.block, OP_CONSTANT, index);
    return val;
  } else if (match(TOKEN_IDENTIFIER)) {
    return variable();
  } else {
    // TODO: create a better error message
    parse_error("Expected literal but got token of type ");
    token_type_print(parser.current->type);
    printf("\n");
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
    Value* rhs = factor();
    if (op == TOKEN_STAR) {
      if (val->type == VAL_INTEGER_32 && rhs->type == VAL_INTEGER_32) {
        block_new_opcode(parser.block, OP_MULTIPLY_INTEGER_32);
      } else {
        // TODO: create a better error message
        parse_error("Cannot multiply values of type ");
        value_type_print(val->type);
        printf(" and ");
        value_type_print(rhs->type);
        printf("\n");
      }
    } else if (op == TOKEN_SLASH) {
      if (val->type == VAL_INTEGER_32 && rhs->type == VAL_INTEGER_32) {
        block_new_opcode(parser.block, OP_DIVIDE_INTEGER_32);
      } else {
        // TODO: create a better error message
        parse_error("Cannot divide values of type ");
        value_type_print(val->type);
        printf(" and ");
        value_type_print(rhs->type);
        printf("\n");
      }
    }
    val = rhs;
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
    Value* rhs = term();
    if (op == TOKEN_PLUS) {
      if (val->type == VAL_INTEGER_32 && rhs->type == VAL_INTEGER_32) {
        block_new_opcode(parser.block, OP_ADD_INTEGER_32);
      } else {
        // TODO: create a better error message
        parse_error("Cannot add values of type ");
        value_type_print(val->type);
        printf(" and ");
        value_type_print(rhs->type);
      }
    } else if (op == TOKEN_MINUS) {
      if (val->type == VAL_INTEGER_32 && rhs->type == VAL_INTEGER_32) {
        block_new_opcode(parser.block, OP_SUBTRACT_INTEGER_32);
      } else {
        // TODO: create a better error message
        parse_error("Cannot subtract values of type ");
        value_type_print(val->type);
        printf(" and ");
        value_type_print(rhs->type);
        printf("\n");
      }
    }
    val = rhs;
  }

  return val;
}

/**
 * @brief Parses a statement.
 */
void statement() {
  if (match(TOKEN_PRINT)) {
    expression();
    block_new_opcode(parser.block, OP_PRINT);
  } else if (match(TOKEN_I32) || match(TOKEN_BOOL)) {
    enum ValueType type = VAL_NULL;
    if (parser.previous->type == TOKEN_I32) {
      type = VAL_INTEGER_32;
    } else if (parser.previous->type == TOKEN_BOOL) {
      type = VAL_BOOL;
    }

    consume(TOKEN_IDENTIFIER);

    const char* name = parser.previous->lexeme;
    PString* pstr = p_object_string_new(name);
    uint8_t index =
        block_new_constant(parser.block, value_new_object((PObject*)pstr));

    block_new_opcodes(parser.block, OP_CONSTANT, index);
    block_new_opcode(parser.block, OP_GLOBAL_DEFINE);

    consume(TOKEN_EQUAL);

    Value* val = expression();
    hash_table_set(&parser.globals, name, val);

    block_new_opcodes(parser.block, OP_CONSTANT, index);
    block_new_opcode(parser.block, OP_GLOBAL_SET);
  } else {
    expression();
  }
  match(TOKEN_SEMICOLON);

  if (parser.had_error) {
    synchronize();
  }
}

bool parse() {
  while (!match(TOKEN_EOF)) {
    statement();
  }

  return !parser.had_error;
}