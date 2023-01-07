
/**
* @file parser.h
* @author Devin Arena
* @brief Parses tokens into 
* @since 1/6/2023
**/

#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "block.h"

typedef struct Parser {
    Token* current;
    Token* previous;
    Block* block;
} Parser;

// initializes the parser with the given block
void parser_init(Block* block);

void expression();

#endif