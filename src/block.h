
/**
 * @file block.h
 * @author Devin Arena
 * @brief Represents blocks of code, stores information about them such as
 * constants, etc.
 * @since 1/6/2023
 **/

#ifndef POSITRON_BLOCK_H
#define POSITRON_BLOCK_H

#include <stdint.h>
#include <stdlib.h>

#include "dyn_list.h"
#include "value.h"

enum OpCode {
    // 1 byte
    OP_NOP,
    OP_POP,
    OP_PRINT,
    OP_GLOBAL_DEFINE,
    OP_GLOBAL_SET,
    OP_GLOBAL_GET,

    OP_ADD_INTEGER_32,
    OP_SUBTRACT_INTEGER_32,
    OP_MULTIPLY_INTEGER_32,
    OP_DIVIDE_INTEGER_32,
    OP_NEGATE_INTEGER_32,
    OP_COMPARE_INTEGER_32,
    
    OP_NOT,

    OP_CJUMPF,

    // Two bytes
    OP_CONSTANT,
    OP_JUMP,
};

typedef struct {
    const char* name;
    dyn_list* opcodes;
    dyn_list* constants;
} Block;

// allocates and returns a pointer to a new block
Block* block_new(const char* name);
// adds a new opcode to a block
void block_new_opcode(Block* block, uint8_t opcode);
// Adds two new opcodes to a block
void block_new_opcodes(Block* block, uint8_t opcodeA, uint8_t opcodeB);
// Adds three new opcodes to a block
void block_new_opcodes_3(Block* block, uint8_t opcodeA, uint8_t opcodeB, uint8_t opcodeC);
// adds a new constant to a block, returning the index of the constant
uint8_t block_new_constant(Block* block, Value* constant);
// prints a block's information
void block_print(Block* block);
// frees the memory allocated by a block
void block_free(Block* block);
// prints an opcode name
size_t block_print_opcode(Block* block, size_t index);

#endif