
/**
 * @file parser.h
 * @author Devin Arena
 * @brief Parses tokens into
 * @since 1/6/2023
 **/

#ifndef POSITRON_PARSER_H
#define POSITRON_PARSER_H

#include "block.h"
#include "hash_table.h"
#include "object.h"
#include "positron.h"
#include "token.h"

#define MAX_LOCALS UINT8_MAX

typedef enum Precedence {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;

typedef Value (*ParseFn)(bool can_assign);

typedef struct ParseRule {
    ParseFn prefix;
    ParseFn suffix;
    Precedence precedence;
} ParseRule;

typedef struct Local {
  Token name;
  size_t depth;
  Value value;
} Local;

typedef struct Parser {
  Token current;
  Token previous;
  PFunction* function;
  HashTable globals;
  size_t scope;
  Local locals[MAX_LOCALS];
  size_t local_count;
  bool had_error;
} Parser;

// initializes the parser with the given char* name
void parser_init();
// produces a parse error and triggers synchronization
void parse_error(const char* format, ...);
// parses a script
PFunction* parse_script(char* name);
// parses a function
PFunction* parse_function(PFunction* function);
// frees the parser's memory
void parser_free();
ParseRule get_rule(TokenType type);

void statement();

#endif