
/**
* @file interpreter.h
* @author Devin Arena
* @brief A bytecode VM interpreter to read emitted opcodes and execute them.
* @since 1/7/2023
**/

#ifndef POSITRON_INTERPRETER_H
#define POSITRON_INTERPRETER_H

#include "positron.h"
#include "block.h"
#include "dyn_list.h"
#include "hash_table.h"

typedef enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_FAIL,
} InterpretResult;

typedef struct Interpreter {
    size_t ip;
    int sp;
    Value stack[STACK_SIZE];
    HashTable globals;
} Interpreter;

// Initialize the interpreter's memory.
void interpreter_init();
// Interpret the emitted opcodes of a block.
InterpretResult interpret(Block* block);
// Prints the interpreter.
void interpreter_print(Block* block);
// Frees the interpreter's memory.
void interpreter_free();

#endif