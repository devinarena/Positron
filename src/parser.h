
/**
* @file parser.h
* @author Devin Arena
* @brief Parses tokens into 
* @since 1/6/2023
**/

#ifndef POSITRON_PARSER_H
#define POSITRON_PARSER_H

#include "positron.h"
#include "token.h"
#include "block.h"
#include "hash_table.h"
#include "object.h"

#define MAX_LOCALS UINT8_MAX

enum Precedence {
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR, // or
    PREC_AND, // and
    PREC_EQUALITY, // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM, // + -
    PREC_FACTOR, // * /
    PREC_UNARY, // ! -
    PREC_CALL, // . ()
    PREC_PRIMARY
};

typedef struct Local {
    Token* name;
    size_t depth;
    Value* value;
} Local;

typedef struct Parser {
    Token current;
    Token previous;
    Block* block;
    HashTable globals;
    size_t scope;
    Local locals[MAX_LOCALS];
    size_t local_count;
    bool had_error;
} Parser;

// initializes the parser with the given block
void parser_init(Block* block);
// produces a parse error and triggers synchronization
void parse_error(const char* format, ...);
bool parse();
// frees the parser's memory
void parser_free();

void statement();

#endif