
/**
* @file interpreter.h
* @author Devin Arena
* @brief A bytecode VM interpreter to read emitted opcodes and execute them.
* @since 1/7/2023
**/

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "dyn_list.h"
#include "block.h"

typedef struct Interpreter {
    size_t ip;
    dyn_list* stack;
} Interpreter;

// Initialize the interpreter's memory.
void interpreter_init();
// Interpret the emitted opcodes of a block.
void interpret(Block* block);
// Prints the interpreter.
void interpreter_print(Block* block);
// Frees the interpreter's memory.
void interpreter_free();

#endif