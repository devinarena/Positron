
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
    OP_DUPE,
    OP_SWAP,
    OP_EXIT,
    OP_RETURN,
    OP_PRINT,
    OP_GLOBAL_DEFINE,
    OP_GLOBAL_SET,
    OP_GLOBAL_GET,
    OP_LOCAL_SET,
    OP_LOCAL_GET,

    OP_NEGATE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,

    OP_NOT,
    OP_LT,
    OP_GT,
    OP_LTE,
    OP_GTE,
    OP_EQ,
    OP_NEQ,

    // Two bytes
    OP_CONSTANT,
    OP_CALL,
    OP_FIELD_GET,
    OP_FIELD_SET,

    // Three bytes
    OP_JUMP,
    OP_JUMP_BACK,
    OP_CJUMPF,
    OP_CJUMPT,
};

typedef struct Block {
    dyn_list* opcodes;
    dyn_list* constants;
} Block;

// allocates and returns a pointer to a new block
Block* block_new();
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