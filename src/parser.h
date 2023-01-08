
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

typedef struct Parser {
    Token* current;
    Token* previous;
    Block* block;
} Parser;

// initializes the parser with the given block
void parser_init(Block* block);

Value* expression();
void statement();

#endif