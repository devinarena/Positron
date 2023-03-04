
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
#include "object.h"
#include "value.h"

typedef enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_FAIL,
} InterpretResult;

typedef struct CallFrame {
    size_t ip;
    PFunction* function;
    Value* slots;
    size_t slotCount;
} CallFrame;

typedef struct Interpreter {
    int fp;
    int sp;
    Value stack[STACK_SIZE];
    HashTable globals;
    HashTable strings;
    CallFrame frames[MAX_FRAMES];
    PObject* heap;
} Interpreter;

extern Interpreter interpreter;

// Initialize the interpreter's memory.
void interpreter_init();
// Interpret the emitted opcodes of a function.
InterpretResult interpret(PFunction* function);
// Prints the interpreter.
void interpreter_print();
// Frees the interpreter's memory.
void interpreter_free();

#endif