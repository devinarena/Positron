
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
  if (parser.current.type == TOKEN_EOF)
    return;
  if (parser.previous.type != TOKEN_NONE)
    token_free(&parser.previous);
  parser.previous = parser.current;
  parser.current = lexer_next_token();
#ifdef POSITRON_DEBUG
    token_print(&parser.previous);
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
  if (parser.current.type == type) {
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
  if (parser.current.type == type) {
    advance();
    return;
  }
  // TODO: create a better error message
  parse_error("Unexpected token ");
  token_type_print(parser.current.type);
  printf("\n");
}

/**
 * @brief Produces a parse error and triggers synchronization.
 *
 * @param message The error message to print.
 */
void parse_error(const char* format, ...) {
  printf("[line %d] at '%s': ", parser.previous.line, parser.previous.lexeme);
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
  parser.scope = 0;
  parser.local_count = 0;
  parser.current = token_new(TOKEN_NONE, "", 0);
  parser.previous = token_new(TOKEN_NONE, "", 0);
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
  while (parser.current.type != TOKEN_EOF) {
    if (parser.previous.type == TOKEN_SEMICOLON) {
      return;
    }

    switch (parser.current.type) {
      case TOKEN_PRINT:
      case TOKEN_I32:
      case TOKEN_BOOL:
      case TOKEN_IF:
        return;
      default:
        break;
    }

    advance();
  }
}

/**
 * @brief Pops all locals with depths greater than the current off the stack.
 */
static void pop_locals() {
  for (int i = parser.local_count - 1; i >= 0; i--) {
    // break if we've reached the end of the current scope
    if (parser.locals[i].depth < parser.scope)
      break;

    block_new_opcode(parser.block, OP_POP);
    parser.local_count--;
  }
}

/**
 * @brief Gets the index of a local variable on the local stack or -1 if it
 * doesn't exist.
 *
 * @param name the name of the local
 * @return int the index of the local
 */
static int get_local(Token* name) {
  for (int i = parser.local_count - 1; i >= 0; i--) {
    // break if we've reached the end of the current scope
    if (parser.locals[i].depth < parser.scope)
      break;

    // check for duplicate local names
    if (strcmp(parser.locals[i].name->lexeme, name->lexeme) == 0) {
      return i;
    }
  }
  return -1;
}

/**
 * @brief Initializes a new local variable on the local stack.
 *
 * @param name the name of the local
 * @return int the index of the local
 */
static int new_local(Token* name) {
  if (parser.local_count == UINT8_MAX) {
    parse_error("Too many local variables in scope\n");
    return -1;
  }
  if (get_local(name) != -1) {
    parse_error("Redefinition of variable with name '%s'\n", name->lexeme);
    return -1;
  }
  parser.locals[parser.local_count].name = name;
  parser.locals[parser.local_count].depth = parser.scope;
  parser.local_count++;
  return parser.local_count - 1;
}

/**
 * @brief Parses a variable.
 */
static Value variable() {
  Token* token = &parser.previous;
  Value* val;
  if (!parser.scope) {
    val = hash_table_get(&parser.globals, token->lexeme);
    if (val == NULL) {
      parse_error("Undefined variable '%s'\n", token->lexeme);
      return value_new_null();
    }

    uint8_t index = block_new_constant(
        parser.block,
        &value_new_object((PObject*)p_object_string_new(token->lexeme)));

    block_new_opcodes(parser.block, OP_CONSTANT, index);
    block_new_opcode(parser.block, OP_GLOBAL_GET);
  } else {
    // iterate over all locals in the current scope
    for (int i = parser.local_count - 1; i >= 0; i--) {
      // break if we've reached the end of the current scope
      if (parser.locals[i].depth < parser.scope)
        break;

      // check all potentials for a match and emit the appropriate opcode
      if (strcmp(parser.locals[i].name->lexeme, token->lexeme) == 0) {
        block_new_opcodes(parser.block, OP_LOCAL_GET, i);
        return value_new_null();
      }
    }
  }
  return *val;
}

/**
 * @brief Parses a literal.
 */
static Value literal() {
  if (match(TOKEN_LITERAL_INTEGER)) {
    Value val = value_new_int_32(atoi(parser.previous.lexeme));
    uint8_t index = block_new_constant(parser.block, &val);
    block_new_opcodes(parser.block, OP_CONSTANT, index);
    return val;
  } else if (match(TOKEN_LITERAL_STRING)) {
    Value val = value_new_object(
        (PObject*)p_object_string_new(parser.previous.lexeme));
    uint8_t index = block_new_constant(parser.block, &val);
    block_new_opcodes(parser.block, OP_CONSTANT, index);
    return val;
  } else if (match(TOKEN_NULL)) {
    Value val = value_new_null();
    uint8_t index = block_new_constant(parser.block, &val);
    block_new_opcodes(parser.block, OP_CONSTANT, index);
    return val;
  } else if (match(TOKEN_TRUE)) {
    Value val = value_new_boolean(true);
    uint8_t index = block_new_constant(parser.block, &val);
    block_new_opcodes(parser.block, OP_CONSTANT, index);
    return val;
  } else if (match(TOKEN_FALSE)) {
    Value val = value_new_boolean(false);
    uint8_t index = block_new_constant(parser.block, &val);
    block_new_opcodes(parser.block, OP_CONSTANT, index);
    return val;
  } else if (match(TOKEN_IDENTIFIER)) {
    return variable();
  } else {
    // TODO: create a better error message
    parse_error("Expected literal but got token of type ");
    token_type_print(parser.current.type);
    printf("\n");
  }
  return value_new_null();
}

static Value expression(enum Precedence prec);

/**
 * @brief Handles primary level parsing, including parenthesized expressions and
 * literals.
 *
 * @return Value* The value of the parsed expression.
 */
static Value primary() {
  if (match(TOKEN_LPAREN)) {
    Value val = expression(PREC_ASSIGNMENT);
    consume(TOKEN_RPAREN);
    return val;
  } else {
    return literal();
  }
}

/**
 * @brief Handles conditional parsing, e.g. ==, !=
 *
 * @return Value the result of the condition
 */
static Value condition(Value* lhs) {
  if (!match(TOKEN_EQUAL_EQUAL) && !match(TOKEN_NOT_EQUAL) &&
      !match(TOKEN_GREATER) && !match(TOKEN_GREATER_EQUAL) &&
      !match(TOKEN_LESS) && !match(TOKEN_LESS_EQUAL)) {
    parse_error("Expected condition operator but got token of type ");
    token_type_print(parser.current.type);
    return value_new_null();
  }
  enum TokenType op = parser.previous.type;
  Value rhs = expression(PREC_COMPARISON);
  if (lhs->type == VAL_INTEGER_32 && rhs.type == VAL_INTEGER_32) {
    if (op == TOKEN_EQUAL_EQUAL)
      block_new_opcode(parser.block, OP_COMPARE_INTEGER_32);
    else if (op == TOKEN_GREATER)
      block_new_opcode(parser.block, OP_GREATER_INTEGER_32);
    else if (op == TOKEN_LESS)
      block_new_opcode(parser.block, OP_LESS_INTEGER_32);
    else if (op == TOKEN_GREATER_EQUAL)
      block_new_opcode(parser.block, OP_GREATER_EQUAL_INTEGER_32);
    else if (op == TOKEN_LESS_EQUAL)
      block_new_opcode(parser.block, OP_LESS_EQUAL_INTEGER_32);
  } else if (lhs->type == VAL_BOOL && rhs.type == VAL_BOOL) {
    if (op == TOKEN_EQUAL_EQUAL)
      block_new_opcode(parser.block, OP_COMPARE_BOOLEAN);
    else {
      parse_error("Invalid operator for boolean comparison");
      return value_new_null();
    }
  } else {
    parse_error("Cannot compare values of type ");
    value_type_print(lhs->type);
    printf(" and ");
    value_type_print(rhs.type);
    printf("\n");
    return value_new_null();
  }
  if (op == TOKEN_NOT_EQUAL) {
    block_new_opcode(parser.block, OP_NOT);
  }
  return value_new_boolean(true);
}

/**
 * @brief Handles factor level parsing, including multiplication and division.
 *
 * @return Value The value of the parsed expression.
 */
static Value factor() {
  Value val = primary();

  while (match(TOKEN_STAR) || match(TOKEN_SLASH)) {
    enum TokenType op = parser.previous.type;
    Value rhs = primary();
    if (op == TOKEN_STAR) {
      if (val.type == VAL_INTEGER_32 && rhs.type == VAL_INTEGER_32) {
        block_new_opcode(parser.block, OP_MULTIPLY_INTEGER_32);
      } else {
        // TODO: create a better error message
        parse_error("Cannot multiply values of type ");
        value_type_print(val.type);
        printf(" and ");
        value_type_print(rhs.type);
        printf("\n");
      }
    } else if (op == TOKEN_SLASH) {
      if (val.type == VAL_INTEGER_32 && rhs.type == VAL_INTEGER_32) {
        block_new_opcode(parser.block, OP_DIVIDE_INTEGER_32);
      } else {
        // TODO: create a better error message
        parse_error("Cannot divide values of type ");
        value_type_print(val.type);
        printf(" and ");
        value_type_print(rhs.type);
        printf("\n");
      }
    }
    val = rhs;
  }

  return val;
}

/**
 * @brief Handles term-level parsing (addition and subtraction)
 */
static Value term(Value* lhs) {
  enum TokenType op = parser.previous.type;
  
  Value rhs = factor();
  if (lhs->type == VAL_NULL) return rhs;

  if (op == TOKEN_PLUS) {
    if (lhs->type == VAL_INTEGER_32 && rhs.type == VAL_INTEGER_32) {
      block_new_opcode(parser.block, OP_ADD_INTEGER_32);
    } else {
      // TODO: create a better error message
      parse_error("Cannot add values of type ");
      value_type_print(lhs->type);
      printf(" and ");
      value_type_print(rhs.type);
      printf("\n");
    }
  } else if (op == TOKEN_MINUS) {
    if (lhs->type == VAL_INTEGER_32 && rhs.type == VAL_INTEGER_32) {
      block_new_opcode(parser.block, OP_SUBTRACT_INTEGER_32);
    } else {
      // TODO: create a better error message
      parse_error("Cannot subtract values of type ");
      value_type_print(lhs->type);
      printf(" and ");
      value_type_print(rhs.type);
      printf("\n");
    }
  }
  return rhs;
}

/**
 * @brief Parses a logical expression (and and or).
 */
static Value logical(Value* lhs) {
  if (lhs->type != VAL_BOOL) {
    parse_error("Expected boolean value but got ");
    value_type_print(lhs->type);
    printf("\n");
    return value_new_null();
  }
  enum TokenType op = parser.previous.type;
  if (op == TOKEN_AND) {
    block_new_opcode(parser.block, OP_DUPE);
    block_new_opcodes_3(parser.block, OP_CJUMPF, 0, 0);
    int start = parser.block->opcodes->size;
    Value rhs = expression(PREC_AND);
    if (rhs.type != VAL_BOOL) {
      parse_error("Expected boolean value but got ");
      value_type_print(rhs.type);
      printf("\n");
      return value_new_null();
    }
    int end = parser.block->opcodes->size;
    uint16_t dist = end - start + 1;
    (*(uint8_t*)parser.block->opcodes->data[start - 2]) = (dist >> 8) & 0xFF;
    (*(uint8_t*)parser.block->opcodes->data[start - 1]) = dist & 0xFF;
    return rhs;
  } else if (op == TOKEN_OR) {
    block_new_opcode(parser.block, OP_DUPE);
    block_new_opcodes_3(parser.block, OP_CJUMPT, 0, 0);
    int start = parser.block->opcodes->size;
    Value rhs = expression(PREC_OR);
    if (rhs.type != VAL_BOOL) {
      parse_error("Expected boolean value but got ");
      value_type_print(rhs.type);
      printf("\n");
      return value_new_null();
    }
    int end = parser.block->opcodes->size;
    uint16_t dist = end - start + 1;
    (*(uint8_t*)parser.block->opcodes->data[start - 2]) = (dist >> 8) & 0xFF;
    (*(uint8_t*)parser.block->opcodes->data[start - 1]) = dist & 0xFF;
    return rhs;
  }
  return value_new_null();
}

/**
 * @brief Parses a unary expression.
 */
static Value unary() {
  enum TokenType prev = parser.previous.type;
  Value val = expression(PREC_UNARY);
  switch (prev) {
    case TOKEN_MINUS: {
      if (val.type == VAL_INTEGER_32) {
        block_new_opcode(parser.block, OP_NEGATE_INTEGER_32);
      } else {
        parse_error("Cannot negate value of type ");
        value_type_print(val.type);
        printf("\n");
      }
      break;
    }
    case TOKEN_EXCLAMATION: {
      block_new_opcode(parser.block, OP_NOT);
      bool data = !value_is_truthy(&val);
      val.type = VAL_BOOL;
      val.data.boolean = data;
      return val;
    }
    default: {
      parse_error("Invalid unary operator");
      break;
    }
  }
  return val;
}

/**
 * @brief Parses an assignment expression.
 *
 */
static Value assignment() {
  Token* name = &parser.previous;
  match(TOKEN_EQUAL);
  block_new_opcode(parser.block, OP_POP);
  Value rhs = expression(PREC_ASSIGNMENT);
  if (!parser.scope) {
    PString* pname = p_object_string_new(name->lexeme);
    Value val = value_new_object((PObject*)name);
    block_new_opcodes(parser.block, OP_CONSTANT,
                      block_new_constant(parser.block, &val));
    block_new_opcode(parser.block, OP_GLOBAL_SET);
    Value* global = hash_table_get(&parser.globals, pname->value);
    if (!global) {
      parse_error("Undefined variable '%s'", name->lexeme);
      return value_new_null();
    } else if (global->type != rhs.type) {
      parse_error("Cannot assign value of type ");
      value_type_print(rhs.type);
      printf(" to variable of type ");
      value_type_print(global->type);
      printf("\n");
    }
    hash_table_set(&parser.globals, pname->value, &rhs);
  } else {
    int index = get_local(name);
    if (index == -1) {
      parse_error("Undefined variable '%s'", name->lexeme);
      return value_new_null();
    }
    block_new_opcodes(parser.block, OP_LOCAL_SET, index);
  }
  return rhs;
}

/**
 * @brief Parses an expression.
 */
static Value expression(enum Precedence prec) {
  Value val = value_new_null();
  // TODO: clean this up
  // prefix operators
  if (match(TOKEN_MINUS) || match(TOKEN_EXCLAMATION)) {
    val = unary();
  } else {
    val = term(&val);
  }

  // infix operators (by precedence)
  if (prec <= PREC_TERM) {
    while (match(TOKEN_PLUS) || match(TOKEN_MINUS)) {
      val = term(&val);
    }
  }
  if (prec <= PREC_COMPARISON) {
    while (check(TOKEN_EQUAL_EQUAL) || check(TOKEN_NOT_EQUAL) ||
           check(TOKEN_GREATER) || check(TOKEN_GREATER_EQUAL) ||
           check(TOKEN_LESS) || check(TOKEN_LESS_EQUAL)) {
      val = condition(&val);
    }
  }
  if (prec <= PREC_AND) {
    while (match(TOKEN_AND) || match(TOKEN_OR)) {
      val = logical(&val);
    }
  }
  if (prec <= PREC_ASSIGNMENT) {
    while (match(TOKEN_EQUAL)) {
      val = assignment();
    }
  }

  return val;
}

/**
 * @brief Parses an if statement.
 */
static void statement_if() {
  consume(TOKEN_LPAREN);
  Value condition = expression(PREC_ASSIGNMENT);
  if (condition.type != VAL_BOOL) {
    parse_error("Expected value type VAL_BOOL but got ");
    value_type_print(condition.type);
    printf("\n");
    return;
  }
  consume(TOKEN_RPAREN);

  block_new_opcodes_3(parser.block, OP_CJUMPF, 0xFF, 0xFF);
  size_t start = parser.block->opcodes->size - 2;
  statement();
  int jump = parser.block->opcodes->size - start - 1;
  uint16_t size = jump < UINT16_MAX ? jump : UINT16_MAX;
  (*(uint8_t*)parser.block->opcodes->data[start]) = (size >> 8) & 0xFF;
  (*(uint8_t*)parser.block->opcodes->data[start + 1]) = size & 0xFF;
}

/**
 * @brief Implementation of a local declaration.
 *
 * @param type
 */
static void declaration_local(enum ValueType type) {
  Token* name = &parser.previous;
  consume(TOKEN_EQUAL);
  Value val = expression(PREC_ASSIGNMENT);
  if (get_local(name) != -1) {
    parse_error("Variable '%s' already declared", name->lexeme);
  }
  if (val.type != type) {
    parse_error("Expected value type ");
    value_type_print(val.type);
    printf(" but got ");
    value_type_print(type);
    printf("\n");
    return;
  }

  block_new_opcodes(parser.block, OP_LOCAL_SET, new_local(name));
}

/**
 * @brief Implementation of a global declaration.
 */
static void declaration_global(enum ValueType type) {
  const char* name = parser.previous.lexeme;
  PString* pstr = p_object_string_new(name);
  Value vname = value_new_object((PObject*)pstr);
  uint8_t index = block_new_constant(parser.block, &vname);

  block_new_opcodes(parser.block, OP_CONSTANT, index);
  block_new_opcode(parser.block, OP_GLOBAL_DEFINE);

  consume(TOKEN_EQUAL);

  Value val = expression(PREC_ASSIGNMENT);

  if (val.type != type) {
    parse_error("Expected value type ");
    value_type_print(val.type);
    printf(" but got ");
    value_type_print(type);
    printf("\n");
    return;
  }


  hash_table_set(&parser.globals, pstr->value, &val);

  block_new_opcodes(parser.block, OP_CONSTANT, index);
  block_new_opcode(parser.block, OP_GLOBAL_SET);
}

/**
 * @brief Parses a declaration.
 */
static void statement_declaration() {
  enum ValueType type = VAL_NULL;
  if (parser.previous.type == TOKEN_I32) {
    type = VAL_INTEGER_32;
  } else if (parser.previous.type == TOKEN_BOOL) {
    type = VAL_BOOL;
  }

  consume(TOKEN_IDENTIFIER);

  if (parser.scope)
    declaration_local(type);
  else
    declaration_global(type);
}

/**
 * @brief Parses a block.
 *
 */
static void block() {
  parser.scope++;
  while (!match(TOKEN_RBRACE)) {
    statement();
  }
  parser.scope--;
  pop_locals();
}

/**
 * @brief Parses a statement.
 */
void statement() {
  if (match(TOKEN_PRINT)) {
    expression(PREC_ASSIGNMENT);
    block_new_opcode(parser.block, OP_PRINT);
  } else if (match(TOKEN_IF)) {
    statement_if();
  } else if (match(TOKEN_I32) || match(TOKEN_BOOL)) {
    statement_declaration();
  } else if (match(TOKEN_LBRACE)) {
    block();
  } else {
    expression(PREC_ASSIGNMENT);
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