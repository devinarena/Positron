
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
#include "positron.h"
#include "token.h"
#include "value.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

Parser parser;

/**
 * @brief Advances the parser to the next token, tracking the previous token.
 */
static void advance() {
  if (parser.current.type == TOKEN_EOF)
    return;
  parser.previous = parser.current;
  parser.current = lexer_next_token();
#ifdef POSITRON_DEBUG
  if (DEBUG_MODE) {
    token_print(&parser.previous);
    printf("->");
    token_print(&parser.current);
    printf("\n");
  }
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
  printf("[line %d] Error at '", parser.previous.line);
  token_print_lexeme(&parser.previous);
  printf("': ");
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  parser.had_error = true;
}

/**
 * @brief Initializes the parser with the given name.
 */
void parser_init() {
  parser.function = NULL;
  parser.scope = 0;
  parser.local_count = 0;
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
      case TOKEN_F64:
      case TOKEN_STR:
      case TOKEN_BOOL:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_FOR:
      case TOKEN_VOID:
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
    if (parser.locals[i].depth <= parser.scope)
      break;

    block_new_opcode(parser.function->block, OP_POP);
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
    // check for duplicate local names
    if (strncmp(parser.locals[i].name.start, name->start,
                max(parser.locals[i].name.length, name->length)) == 0) {
      return i;
    }
  }
  return -1;
}

/**
 * @brief Initializes a new local variable on the local stack.
 *
 * @param name the name of the local
 * @param value Value* the value of the local
 * @return int the index of the local
 */
static int new_local(Token* name, Value* value) {
  if (parser.local_count == UINT8_MAX) {
    parse_error("Too many local variables in scope\n");
    return -1;
  }
  if (get_local(name) != -1) {
    parse_error("Redefinition of variable with name '");
    token_print_lexeme(name);
    printf("'\n");
    return -1;
  }
  parser.locals[parser.local_count].name = *name;
  parser.locals[parser.local_count].depth = parser.scope;
  parser.locals[parser.local_count].value = *value;
  parser.local_count++;
  return parser.local_count - 1;
}

/**
 * @brief Type checks two values.
 *
 * @param a a pointer to the first value
 * @param b a pointer to the second value
 * @param message whether or not to print an error message
 * @return true if the types match
 * @return false if the types do not match
 */
static bool type_check(Value* a, Value* b, bool message) {
  if (a->type == VAL_OBJ && b->type == VAL_OBJ) {
    if (a->data.reference->type == b->data.reference->type)
      return true;
  } else if (a->type == b->type)
    return true;
  if (message) {
    parse_error("Type mismatch: received ");
    value_print(a);
    printf(" but expected ");
    value_print(b);
    printf("\n");
  }
  return false;
}

/**
 * @brief Type checks a single value.
 *
 * @param a a pointer to the value
 * @param type the type to check against
 * @param message whether or not to print an error message
 * @return true if the types match
 * @return false if the types do not match
 */
static bool type_check_single(Value* a, enum ValueType type, bool message) {
  if (a->type == type)
    return true;
  if (message) {
    parse_error("Type mismatch: received ");
    value_print(a);
    printf(" but expected ");
    value_type_print(type);
    printf("\n");
  }
  return false;
}

static Value expression(Precedence prec);

/**
 * @brief Parses a variable.
 */
static Value variable(bool canAssign) {
  Token token = parser.previous;
  if (parser.scope) {
    int index = get_local(&token);

    if (index != -1) {
      if (canAssign && match(TOKEN_EQUAL)) {
        Value value = expression(PREC_ASSIGNMENT);
        if (!type_check(&value, &parser.locals[index].value, true))
          return value_new_null();
        parser.locals[index].value = value;
        block_new_opcodes(parser.function->block, OP_LOCAL_SET, index);
        return value;
      } else {
        block_new_opcodes(parser.function->block, OP_LOCAL_GET, index);
        return parser.locals[index].value;
      }
    }
  }

  PString* name = p_object_string_new_n(token.start, token.length);

  Value* val = hash_table_get(&parser.globals, name->value);
  if (val == NULL) {
    parse_error("Undefined variable '");
    token_print_lexeme(&token);
    printf("'\n");
    return value_new_null();
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    Value value = expression(PREC_ASSIGNMENT);
    if (!type_check(&value, val, true))
      return value_new_null();
    block_new_opcodes_3(parser.function->block, OP_CONSTANT,
                        block_new_constant(parser.function->block,
                                           &value_new_object((PObject*)name)),
                        OP_GLOBAL_SET);
    return value;
  } else {
    block_new_opcodes_3(parser.function->block, OP_CONSTANT,
                        block_new_constant(parser.function->block,
                                           &value_new_object((PObject*)name)),
                        OP_GLOBAL_GET);
  }

  return *val;
}

/**
 * @brief Parses a literal.
 */
static Value literal(bool canAssign) {
  enum TokenType type = parser.previous.type;

  if (type == TOKEN_LITERAL_INTEGER) {
    char buffer[200];
    strncpy(buffer, parser.previous.start, parser.previous.length);
    buffer[parser.previous.length] = '\0';
    Value val = value_new_int_32(atoi(buffer));
    uint8_t index = block_new_constant(parser.function->block, &val);
    block_new_opcodes(parser.function->block, OP_CONSTANT, index);
    return val;
  } else if (type == TOKEN_LITERAL_FLOATING) {
    char buffer[200];
    strncpy(buffer, parser.previous.start, parser.previous.length);
    buffer[parser.previous.length] = '\0';
    Value val = value_new_float(atof(buffer));
    uint8_t index = block_new_constant(parser.function->block, &val);
    block_new_opcodes(parser.function->block, OP_CONSTANT, index);
    return val;
  } else if (type == TOKEN_LITERAL_STRING) {
    Value val = value_new_object((PObject*)p_object_string_new_n(
        parser.previous.start, parser.previous.length));
    uint8_t index = block_new_constant(parser.function->block, &val);
    block_new_opcodes(parser.function->block, OP_CONSTANT, index);
    return val;
  } else if (type == TOKEN_NULL) {
    Value val = value_new_null();
    uint8_t index = block_new_constant(parser.function->block, &val);
    block_new_opcodes(parser.function->block, OP_CONSTANT, index);
    return val;
  } else if (type == TOKEN_TRUE) {
    Value val = value_new_boolean(true);
    uint8_t index = block_new_constant(parser.function->block, &val);
    block_new_opcodes(parser.function->block, OP_CONSTANT, index);
    return val;
  } else if (type == TOKEN_FALSE) {
    Value val = value_new_boolean(false);
    uint8_t index = block_new_constant(parser.function->block, &val);
    block_new_opcodes(parser.function->block, OP_CONSTANT, index);
    return val;
  } else if (type == TOKEN_IDENTIFIER) {
    return variable(canAssign);
  } else {
    // TODO: create a better error message
    parse_error("Expected literal but got token of type ");
    token_type_print(parser.previous.type);
    printf("\n");
  }
  return value_new_null();
}

static Value grouping(bool canAssign) {
  Value val = expression(PREC_ASSIGNMENT);
  consume(TOKEN_RPAREN);
  return val;
}

/**
 * @brief Parses a unary expression.
 */
static Value unary(bool canAssign) {
  enum TokenType prev = parser.previous.type;
  Value val = expression(PREC_UNARY);
  switch (prev) {
    case TOKEN_MINUS: {
      if (val.type == VAL_INTEGER_32) {
        block_new_opcode(parser.function->block, OP_NEGATE_INTEGER_32);
      } else {
        parse_error("Cannot negate value of type ");
        value_type_print(val.type);
        printf("\n");
      }
      break;
    }
    case TOKEN_EXCLAMATION: {
      block_new_opcode(parser.function->block, OP_NOT);
      bool data = !value_is_truthy(&val);
      val.type = VAL_BOOL;
      val.data.boolean = data;
      return val;
    }
    default: {
      parse_error("Invalid unary operator\n");
      break;
    }
  }
  return val;
}

/**
 * @brief Handles conditional parsing, e.g. ==, !=
 *
 * @return Value the result of the condition
 */
static Value comparison(Value* lhs, bool canAssign) {
  enum TokenType type = parser.previous.type;
  ParseRule* rule = get_rule(type);

  if (type != TOKEN_EQUAL_EQUAL && type != TOKEN_NOT_EQUAL &&
      type != TOKEN_GREATER && type != TOKEN_GREATER_EQUAL &&
      type != TOKEN_LESS && type != TOKEN_LESS_EQUAL) {
    parse_error("Expected condition operator but got token of type ");
    token_type_print(parser.current.type);
    return value_new_null();
  }

  enum TokenType op = parser.previous.type;
  Value rhs = expression((Precedence)(rule->precedence + 1));

  if (lhs->type == VAL_INTEGER_32 && rhs.type == VAL_INTEGER_32) {
    if (op == TOKEN_EQUAL_EQUAL)
      block_new_opcode(parser.function->block, OP_COMPARE_INTEGER_32);
    else if (op == TOKEN_GREATER)
      block_new_opcode(parser.function->block, OP_GREATER_INTEGER_32);
    else if (op == TOKEN_LESS)
      block_new_opcode(parser.function->block, OP_LESS_INTEGER_32);
    else if (op == TOKEN_GREATER_EQUAL)
      block_new_opcode(parser.function->block, OP_GREATER_EQUAL_INTEGER_32);
    else if (op == TOKEN_LESS_EQUAL)
      block_new_opcode(parser.function->block, OP_LESS_EQUAL_INTEGER_32);
  } else if (lhs->type == VAL_BOOL && rhs.type == VAL_BOOL) {
    if (op == TOKEN_EQUAL_EQUAL)
      block_new_opcode(parser.function->block, OP_COMPARE_BOOLEAN);
    else if (op == TOKEN_NOT_EQUAL) {
      block_new_opcodes(parser.function->block, OP_COMPARE_BOOLEAN, OP_NOT);
    } else {
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
  return value_new_boolean(true);
}

/**
 * @brief Parses a logical expression for and.
 */
static Value and (Value * lhs, bool canAssign) {
  if (!type_check_single(lhs, VAL_BOOL, true))
    return value_new_null();
  block_new_opcodes_3(parser.function->block, OP_CJUMPF, 0, 0);
  int start = parser.function->block->opcodes->size;
  Value rhs = expression(PREC_AND);
  if (!type_check_single(&rhs, VAL_BOOL, true))
    return value_new_null();
  int end = parser.function->block->opcodes->size;
  uint16_t dist = end - start + 1;
  (*(uint8_t*)parser.function->block->opcodes->data[start - 2]) =
      (dist >> 8) & 0xFF;
  (*(uint8_t*)parser.function->block->opcodes->data[start - 1]) = dist & 0xFF;
  return rhs;
}

/**
 * @brief Parses a logical expression for or.
 *
 * @param lhs
 * @param canAssign
 * @return Value
 */
static Value or (Value * lhs, bool canAssign) {
  block_new_opcode(parser.function->block, OP_DUPE);
  block_new_opcodes_3(parser.function->block, OP_CJUMPT, 0, 0);
  int start = parser.function->block->opcodes->size;
  Value rhs = expression(PREC_OR);
  if (!type_check_single(&rhs, VAL_BOOL, true))
    return value_new_null();
  int end = parser.function->block->opcodes->size;
  uint16_t dist = end - start + 1;
  (*(uint8_t*)parser.function->block->opcodes->data[start - 2]) =
      (dist >> 8) & 0xFF;
  (*(uint8_t*)parser.function->block->opcodes->data[start - 1]) = dist & 0xFF;
  return rhs;
}

/**
 * @brief Parse rule for binary expressions.
 */
static Value binary(Value* lhs, bool canAssign) {
  enum TokenType prev = parser.previous.type;
  Value rhs = expression((Precedence)(get_rule(prev)->precedence + 1));
  if (lhs->type == VAL_INTEGER_32 && rhs.type == VAL_INTEGER_32) {
    if (prev == TOKEN_PLUS)
      block_new_opcode(parser.function->block, OP_ADD_INTEGER_32);
    else if (prev == TOKEN_MINUS)
      block_new_opcode(parser.function->block, OP_SUBTRACT_INTEGER_32);
    else if (prev == TOKEN_STAR)
      block_new_opcode(parser.function->block, OP_MULTIPLY_INTEGER_32);
    else if (prev == TOKEN_SLASH)
      block_new_opcode(parser.function->block, OP_DIVIDE_INTEGER_32);
  } else if (lhs->type == VAL_BOOL && rhs.type == VAL_BOOL) {
    if (prev == TOKEN_EQUAL_EQUAL)
      block_new_opcode(parser.function->block, OP_COMPARE_BOOLEAN);
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
  return rhs;
}

/**
 * @brief Parses an assignment expression.
 * TODO: Deprecated? Probably wont use this now
 *
 */
// static Value assignment(Value* lhs, bool canAssign) {
//   Token* name = &parser.previous;
//   int index = get_local(name);

//   const char* name_start = name->start;
//   const int name_length = name->length;
//   Value v_var_name = value_new_object(
//       (PObject*)p_object_string_new_n(name_start, name_length));

//   match(TOKEN_EQUAL);
//   block_new_opcode(parser.function->block, OP_POP);
//   Value rhs = expression(PREC_ASSIGNMENT);

//   if (parser.scope) {
//     if (index != -1) {
//       block_new_opcodes(parser.function->block, OP_LOCAL_SET, index);
//       return rhs;
//     }
//   }

//   Value* val = hash_table_get(&parser.globals, TO_STRING(v_var_name)->value);
//   if (!val) {
//     parse_error("Undefined variable '%s'\n", TO_STRING(v_var_name)->value);
//     return value_new_null();
//   }

//   block_new_opcodes(parser.function->block, OP_CONSTANT,
//                     block_new_constant(parser.function->block, &v_var_name));
//   block_new_opcode(parser.function->block, OP_GLOBAL_SET);

//   return rhs;
// }

/**
 * @brief Descent case for function calls. Handled as an infix operator.
 */
static Value call(Value* lhs, bool canAssign) {
  PFunction* func = TO_FUNCTION(*lhs);
  if (!IS_TYPE(*lhs, P_OBJ_FUNCTION)) {
    parse_error("Expected function but got ");
    value_type_print(lhs->type);
    printf("\n");
    return value_new_null();
  }
  size_t argc = 0;
  while (!check(TOKEN_RPAREN)) {
    if (argc > 0) {
      consume(TOKEN_COMMA);
    }
    Value arg = expression(PREC_ASSIGNMENT);
    if (!type_check_single(&arg, func->parameters[argc].type, true))
      return value_new_null();
    argc++;
  }
  if (argc != func->arity) {
    parse_error("Function '");
    p_object_print((PObject*)func);
    printf("' expects %lld arguments but got %lld\n", func->arity, argc);
    return value_new_null();
  }
  if (argc > 255) {
    parse_error("Cannot call function with more than 255 arguments\n");
    return value_new_null();
  }
  consume(TOKEN_RPAREN);
  block_new_opcodes(parser.function->block, OP_CALL, argc);

  return func->returnType;
}

/**
 * @brief Expression level parsing, handles operators and expressions based on
 * precedence.
 *
 * @param prec the precedence to handle
 * @return Value the result of the expression
 */
static Value expression(Precedence prec) {
  advance();
  PrefixFn prefix = get_rule(parser.previous.type)->prefix;

  // the first rule we hit will always be a prefix rule.
  if (prefix == NULL) {
    parse_error("Expected expression.\n");
    return value_new_null();
  }

  bool canAssign = prec <= PREC_ASSIGNMENT;

  Value result = prefix(canAssign);

  // handle infix rules
  while (prec <= get_rule(parser.current.type)->precedence) {
    advance();
    InfixFn infix = get_rule(parser.previous.type)->infix;
    result = infix(&result, canAssign);
  }

  if (!canAssign && match(TOKEN_EQUAL)) {
    parse_error("Invalid assignment target.");
    return value_new_null();
  }

  return result;
}

/**
 * @brief Parses an if statement.
 */
static void statement_if() {
  consume(TOKEN_LPAREN);
  Value condition = expression(PREC_ASSIGNMENT);
  if (!type_check_single(&condition, VAL_BOOL, true))
    return;
  consume(TOKEN_RPAREN);

  block_new_opcodes_3(parser.function->block, OP_CJUMPF, 0xFF, 0xFF);
  size_t start = parser.function->block->opcodes->size - 2;
  statement();
  if (match(TOKEN_ELSE)) {
    block_new_opcodes_3(parser.function->block, OP_JUMP, 0xFF, 0xFF);
    size_t start2 = parser.function->block->opcodes->size - 2;

    int jump = parser.function->block->opcodes->size - start - 1;
    uint16_t size = jump < UINT16_MAX ? jump : UINT16_MAX;
    (*(uint8_t*)parser.function->block->opcodes->data[start]) =
        (size >> 8) & 0xFF;
    (*(uint8_t*)parser.function->block->opcodes->data[start + 1]) = size & 0xFF;

    statement();
    jump = parser.function->block->opcodes->size - start2 - 1;
    size = jump < UINT16_MAX ? jump : UINT16_MAX;
    (*(uint8_t*)parser.function->block->opcodes->data[start2]) =
        (size >> 8) & 0xFF;
    (*(uint8_t*)parser.function->block->opcodes->data[start2 + 1]) =
        size & 0xFF;
  } else {
    int jump = parser.function->block->opcodes->size - start - 1;
    uint16_t size = jump < UINT16_MAX ? jump : UINT16_MAX;
    (*(uint8_t*)parser.function->block->opcodes->data[start]) =
        (size >> 8) & 0xFF;
    (*(uint8_t*)parser.function->block->opcodes->data[start + 1]) = size & 0xFF;
  }
}

/**
 * @brief Parses a function.
 */
static void statement_function(Value returnType) {
  const char* name = parser.previous.start;
  size_t length = parser.previous.length;

  consume(TOKEN_LPAREN);

  PString* fname = p_object_string_new_n(name, length);
  Value fnameVal = value_new_object((PObject*)fname);
  PFunction* function = p_object_function_new(fname, returnType);

  parser.scope++;
  while (!match(TOKEN_RPAREN)) {
    if (match(TOKEN_I32) || match(TOKEN_F64) || match(TOKEN_BOOL) ||
        match(TOKEN_STR)) {
      ValueType type = value_type_from_token_type(parser.previous.type);
      consume(TOKEN_IDENTIFIER);
      const char* name = parser.previous.start;
      size_t length = parser.previous.length;
      PString* pname = p_object_string_new_n(name, length);
      Value pval = value_new_object((PObject*)pname);
      function->parameters[function->arity++] = (Value){.type = type};
      new_local(&parser.previous, &function->parameters[function->arity - 1]);
    } else {
      parse_error("Expected parameter type but got '");
      token_type_print(parser.previous.type);
      printf("'\n");
    }
    if (!check(TOKEN_RPAREN))
      consume(TOKEN_COMMA);
  }
  parser.scope--;

  Value fval = value_new_object((PObject*)function);
  // TODO: need support for local functions
  if (!parser.scope) {
    // global function
    if (!hash_table_set(&parser.globals, fname->value, &fval))
      parse_error("Global function '%s' already defined\n", fname->value);
  } else {
    parse_error("Local functions not yet supported\n");
  }

  parse_function(function);

  if (!type_check(&function->returnType, &returnType, false)) {
    parse_error("Function '%s' return type does not match declaration\n",
                fname->value);
  }

  uint8_t index = block_new_constant(parser.function->block, &fnameVal);

  block_new_opcodes(parser.function->block, OP_CONSTANT, index);
  block_new_opcode(parser.function->block, OP_GLOBAL_DEFINE);

  block_new_opcodes(parser.function->block, OP_CONSTANT,
                    block_new_constant(parser.function->block, &fval));
  block_new_opcodes(parser.function->block, OP_CONSTANT, index);
  block_new_opcode(parser.function->block, OP_GLOBAL_SET);
}

/**
 * @brief Implementation of a local declaration.
 *
 * @param type
 */
static void statement_declaration_local(enum ValueType type) {
  Token name = parser.previous;

  consume(TOKEN_EQUAL);
  Value val = expression(PREC_ASSIGNMENT);
  if (get_local(&name) != -1) {
    parse_error("Variable '");
    token_print_lexeme(&name);
    printf("' already declared.\n");
  }
  if (!type_check_single(&val, type, true)) {
    return;
  }

  block_new_opcodes(parser.function->block, OP_LOCAL_SET,
                    new_local(&name, &val));
}

/**
 * @brief Implementation of a global declaration.
 */
static void statement_declaration_global(enum ValueType type) {
  Token* name = &parser.previous;
  PString* pstr = p_object_string_new_n(name->start, name->length);
  Value vname = value_new_object((PObject*)pstr);
  uint8_t index = block_new_constant(parser.function->block, &vname);

  block_new_opcodes(parser.function->block, OP_CONSTANT, index);
  block_new_opcode(parser.function->block, OP_GLOBAL_DEFINE);

  consume(TOKEN_EQUAL);

  Value val = expression(PREC_ASSIGNMENT);

  if (!type_check_single(&val, type, true)) {
    return;
  }

  hash_table_set(&parser.globals, pstr->value, &val);

  block_new_opcodes(parser.function->block, OP_CONSTANT, index);
  block_new_opcode(parser.function->block, OP_GLOBAL_SET);
}

/**
 * @brief Parses a declaration.
 */
static void statement_declaration() {
  enum ValueType type = value_type_from_token_type(parser.previous.type);

  consume(TOKEN_IDENTIFIER);

  if (check(TOKEN_LPAREN)) {
    if (type == VAL_OBJ)
      statement_function((Value){
          .type = type, .data.reference = &(PObject){.type = P_OBJ_STRING}});
    else
      statement_function((Value){.type = type});
    return;
  }

  if (parser.scope)
    statement_declaration_local(type);
  else
    statement_declaration_global(type);
}

/**
 * @brief Parses a while statement.
 */
static void statement_while() {
  consume(TOKEN_LPAREN);
  int start = parser.function->block->opcodes->size;
  Value condition = expression(PREC_ASSIGNMENT);
  if (!type_check_single(&condition, VAL_BOOL, true))
    return;
  consume(TOKEN_RPAREN);
  // check the conditional, if it's false, jump to after the loop
  block_new_opcodes_3(parser.function->block, OP_CJUMPF, 0xFF, 0xFF);
  int codes = parser.function->block->opcodes->size - 2;
  statement();

  int end = parser.function->block->opcodes->size;
  int jump = end - (codes + 2) + 4;

  uint16_t size = jump < UINT16_MAX ? jump : UINT16_MAX;
  (*(uint8_t*)parser.function->block->opcodes->data[codes]) =
      (size >> 8) & 0xFF;
  (*(uint8_t*)parser.function->block->opcodes->data[codes + 1]) = size & 0xFF;

  // jump back to before the conditional to run again
  block_new_opcodes_3(parser.function->block, OP_JUMP_BACK, 0xFF, 0xFF);
  codes = parser.function->block->opcodes->size - 2;
  size = (codes + 1 - start) < UINT16_MAX ? (codes + 1 - start) : UINT16_MAX;
  (*(uint8_t*)parser.function->block->opcodes->data[codes]) =
      (size >> 8) & 0xFF;
  (*(uint8_t*)parser.function->block->opcodes->data[codes + 1]) = size & 0xFF;
}

/**
 * @brief Parses a for statement, initializer, conditional, and post expression.
 *
 * TODO: this... probably needs to be refactored
 */
static void statement_for() {
  consume(TOKEN_LPAREN);
  parser.scope++;

  // initializer
  if (match(TOKEN_SEMICOLON)) {
    // no initializer
  } else if (match(TOKEN_I32) || match(TOKEN_F64) || match(TOKEN_BOOL) ||
             match(TOKEN_STR)) {
    statement_declaration();
    if (parser.previous.type != TOKEN_SEMICOLON)
      consume(TOKEN_SEMICOLON);
  } else {
    expression(PREC_ASSIGNMENT);
    consume(TOKEN_SEMICOLON);
  }

  // conditional
  int start = parser.function->block->opcodes->size;
  int conditionalJump = -1;
  Value condition = value_new_boolean(true);
  if (match(TOKEN_SEMICOLON)) {
    // no conditional
  } else {
    condition = expression(PREC_ASSIGNMENT);
    if (condition.type != VAL_BOOL) {
      parse_error("Expected value type VAL_BOOL but got ");
      value_type_print(condition.type);
      printf("\n");
      return;
    }
    consume(TOKEN_SEMICOLON);
    block_new_opcodes_3(parser.function->block, OP_CJUMPF, 0xFF, 0xFF);
    conditionalJump = parser.function->block->opcodes->size - 2;
  }
  block_new_opcodes_3(parser.function->block, OP_JUMP, 0xFF, 0xFF);

  int postPos = parser.function->block->opcodes->size - 2;

  // post expression
  if (match(TOKEN_RPAREN)) {
    // no post expression
  } else {
    expression(PREC_ASSIGNMENT);
    consume(TOKEN_RPAREN);
  }

  // jump back to the conditional after the post condition
  {
    int jsize =
        parser.function->block->opcodes->size - start + 2;  // +2 for jump codes
    uint16_t size = jsize < UINT16_MAX ? jsize : UINT16_MAX;
    block_new_opcodes_3(parser.function->block, OP_JUMP_BACK,
                        (size >> 8) & 0xFF, size & 0xFF);
  }
  {
    int postJump;
    if (conditionalJump == -1) {
      postJump = parser.function->block->opcodes->size - postPos - 1;
    } else
      postJump = parser.function->block->opcodes->size - conditionalJump - 4;
    uint16_t size = postJump < UINT16_MAX ? postJump : UINT16_MAX;
    (*(uint8_t*)parser.function->block->opcodes->data[postPos]) =
        (size >> 8) & 0xFF;
    (*(uint8_t*)parser.function->block->opcodes->data[postPos + 1]) =
        size & 0xFF;
  }

  statement();

  int end = parser.function->block->opcodes->size;

  // jump back to the post condition
  {
    int jsize = end - postPos;
    uint16_t size = jsize < UINT16_MAX ? jsize : UINT16_MAX;
    block_new_opcodes_3(parser.function->block, OP_JUMP_BACK,
                        (size >> 8) & 0xFF, size & 0xFF);
  }

  // patch the conditional jump to jump to the end of the block
  if (conditionalJump != -1) {
    int jump = parser.function->block->opcodes->size - conditionalJump - 1;
    uint16_t size = jump < UINT16_MAX ? jump : UINT16_MAX;
    (*(uint8_t*)parser.function->block->opcodes->data[conditionalJump]) =
        (size >> 8) & 0xFF;
    (*(uint8_t*)parser.function->block->opcodes->data[conditionalJump + 1]) =
        size & 0xFF;
  }

  parser.scope--;
  pop_locals();
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
    block_new_opcode(parser.function->block, OP_PRINT);
  } else if (match(TOKEN_IF)) {
    statement_if();
  } else if (match(TOKEN_I32) || match(TOKEN_F64) || match(TOKEN_BOOL) ||
             match(TOKEN_STR) || match(TOKEN_VOID)) {
    statement_declaration();
  } else if (match(TOKEN_WHILE)) {
    statement_while();
  } else if (match(TOKEN_FOR)) {
    statement_for();
  } else if (match(TOKEN_RETURN)) {
    Value res = expression(PREC_ASSIGNMENT);
    if (!type_check(&res, &parser.function->returnType, true))
      return;
    block_new_opcode(parser.function->block, OP_RETURN);
  } else if (match(TOKEN_EXIT)) {
    Value res = expression(PREC_ASSIGNMENT);
    if (!type_check_single(&res, VAL_INTEGER_32, true))
      return;
    block_new_opcode(parser.function->block, OP_EXIT);
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

/**
 * Parses tokens of a script and returns the script as a function.
 *
 * @return char* the name of the function.
 */
PFunction* parse_script(char* name) {
  parser.function =
      p_object_function_new(p_object_string_new(name), value_new_null());

  while (!match(TOKEN_EOF)) {
    statement();
  }

  block_new_opcode(parser.function->block, OP_RETURN);

  if (parser.had_error) {
    return NULL;
  }

#ifdef POSITRON_DEBUG
  if (DEBUG_MODE) {
    printf("\n::::: FUNCTION: ");
    p_object_print((PObject*)parser.function);
    printf(" :::::\n");
    block_print(parser.function->block);
  }
#endif
  return parser.function;
}

/**
 * @brief Parses a function as opposed to a script. I just separated the two
 * for organization purposes, theoretically these could and maybe should be
 * merged.
 *
 * @return PFunction* the function that was generated.
 */
PFunction* parse_function(PFunction* target) {
  parser.scope++;

  consume(TOKEN_LBRACE);

  PFunction* enclosing = parser.function;

  parser.function = target;

  while (!match(TOKEN_EOF) && !match(TOKEN_RBRACE)) {
    statement();
  }

  if (parser.previous.type != TOKEN_RBRACE) {
    parse_error("Expected '}' at end of function");
  }

  block_new_opcode(parser.function->block, OP_RETURN);

  if (parser.had_error) {
    return NULL;
  }
  parser.function = enclosing;

#ifdef POSITRON_DEBUG
  if (DEBUG_MODE) {
    printf("\n::::: FUNCTION: ");
    p_object_print((PObject*)target);
    printf(" :::::\n");
    block_print(target->block);
  }
#endif

  parser.scope--;
  return target;
}
ParseRule rules[] = {
    [TOKEN_NONE] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},

    [TOKEN_LITERAL_INTEGER] = {literal, NULL, PREC_NONE},
    [TOKEN_LITERAL_FLOATING] = {literal, NULL, PREC_NONE},
    [TOKEN_LITERAL_STRING] = {literal, NULL, PREC_NONE},

    [TOKEN_BOOL] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_EXIT] = {NULL, NULL, PREC_NONE},
    [TOKEN_F64] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {literal, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_I32] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NULL] = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_STR] = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE] = {literal, NULL, PREC_NONE},
    [TOKEN_VOID] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},

    [TOKEN_IDENTIFIER] = {variable, NULL, PREC_NONE},

    [TOKEN_EXCLAMATION] = {unary, NULL, PREC_UNARY},
    [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_MINUS] = {unary, binary, PREC_TERM},
    [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
    [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
    [TOKEN_EQUAL] = {NULL, NULL, PREC_ASSIGNMENT},
    [TOKEN_LPAREN] = {grouping, call, PREC_CALL},
    [TOKEN_RPAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_LBRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_RBRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_GREATER] = {NULL, comparison, PREC_COMPARISON},
    [TOKEN_LESS] = {NULL, comparison, PREC_COMPARISON},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},

    [TOKEN_EQUAL_EQUAL] = {NULL, comparison, PREC_COMPARISON},
    [TOKEN_NOT_EQUAL] = {NULL, comparison, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, comparison, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL] = {NULL, comparison, PREC_COMPARISON},
    [TOKEN_AND] = {NULL, and, PREC_AND},
    [TOKEN_OR] = {NULL, or, PREC_OR},

    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

/**
 * @brief Looks up the rule for a token from the parser's grammar.
 */
ParseRule* get_rule(enum TokenType type) {
  return &rules[type];
}