
/**
* @file interpreter.h
* @author Devin Arena
* @brief A bytecode VM interpreter to read emitted opcodes and execute them.
* @since 1/7/2023
**/

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "block.h"

typedef struct Interpreter {
    size_t ip;
} Interpreter;

// Initialize the interpreter's memory.
void interpreter_init();
void interpret(Block* block);

#endif