
/**
 * @file block.h
 * @author Devin Arena
 * @brief Represents blocks of code, stores information about them such as
 * constants, etc.
 * @since 1/6/2023
 **/

#include <stdio.h>

#include "block.h"

/**
 * @brief Allocates and returns a pointer to a new block.
 *
 * @return Block* a pointer to the newly allocated block
 */
Block* block_new() {
  Block* block = malloc(sizeof(Block));
  block->opcodes = dyn_list_new(free);
  block->constants = dyn_list_new((void*)(void*)value_free);
  return block;
}

/**
 * @brief Adds a new opcode to a block.
 *
 * @param block the block to add the opcode to
 * @param opcode the opcode to add
 */
void block_new_opcode(Block* block, uint8_t opcode) {
  uint8_t* bytes = malloc(sizeof(uint8_t));
  *bytes = opcode;
  dyn_list_add(block->opcodes, (void*)bytes);
}

/**
 * @brief Adds two new opcodes to a block.
 *
 * @param block the block to add the opcodes to
 * @param opcodeA the first opcode to add
 * @param opcodeB the second opcode to add
 */
void block_new_opcodes(Block* block, uint8_t opcodeA, uint8_t opcodeB) {
  uint8_t* bytes = malloc(sizeof(uint8_t));
  *bytes = opcodeA;
  dyn_list_add(block->opcodes, (void*)bytes);

  bytes = malloc(sizeof(uint8_t));
  *bytes = opcodeB;
  dyn_list_add(block->opcodes, (void*)bytes);
}

/**
 * @brief Adds three new opcodes to a block.
 *
 * @param block the block to add the opcodes to
 * @param opcodeA the first opcode to add
 * @param opcodeB the second opcode to add
 * @param opcodeC the third opcode to add
 */
void block_new_opcodes_3(Block* block,
                         uint8_t opcodeA,
                         uint8_t opcodeB,
                         uint8_t opcodeC) {
  uint8_t* bytes = malloc(sizeof(uint8_t));
  *bytes = opcodeA;
  dyn_list_add(block->opcodes, (void*)bytes);

  bytes = malloc(sizeof(uint8_t));
  *bytes = opcodeB;
  dyn_list_add(block->opcodes, (void*)bytes);

  bytes = malloc(sizeof(uint8_t));
  *bytes = opcodeC;
  dyn_list_add(block->opcodes, (void*)bytes);
}

/**
 * @brief Adds a new constant to a block.
 *
 * @param block the block to add the constant to
 * @param constant the constant to add
 */
uint8_t block_new_constant(Block* block, Value* constant) {
  Value* clone = value_clone(constant);
  dyn_list_add(block->constants, (void*)clone);
  return (uint8_t)block->constants->size - 1;
}

/**
 * @brief Prints a block's information.
 *
 * @param block the block to print
 */
void block_print(Block* block) {
  printf("========== Block ==========\n");
  printf("========== Opcodes ==========\n");
  for (size_t i = 0; i < block->opcodes->size;) {
    printf("%.8d: ", (int)i);
    i += block_print_opcode(block, i);
    printf("\n");
  }
  printf("========== Constants ==========\n");
  for (size_t i = 0; i < block->constants->size; i++) {
    printf("%.4d: ", (int)i);
    value_print(block->constants->data[i]);
    printf("\n");
  }
  printf("========== End Block ==========\n");
}

/**
 * @brief Frees the memory allocated by a block.
 *
 * @param block
 */
void block_free(Block* block) {
  dyn_list_free(block->opcodes);
  dyn_list_free(block->constants);
  free(block);
}

/**
 * @brief Prints an opcode name and returns the number of bytes the opcode
 * takes.
 */
size_t block_print_opcode(Block* block, size_t index) {
  uint8_t* opcode = (uint8_t*)block->opcodes->data[index];
  switch (*opcode) {
    case OP_NOP:
      printf("OP_NOP");
      return 1;
    case OP_POP:
      printf("OP_POP");
      return 1;
    case OP_DUPE:
      printf("OP_DUPE");
      return 1;
    case OP_SWAP:
      printf("OP_SWAP");
      return 1;
    case OP_EXIT:
      printf("OP_EXIT");
      return 1;
    case OP_RETURN:
      printf("OP_RETURN");
      return 1;
    case OP_PRINT:
      printf("OP_PRINT");
      return 1;
    case OP_GLOBAL_DEFINE:
      printf("OP_GLOBAL_DEFINE");
      return 1;
    case OP_GLOBAL_SET:
      printf("OP_GLOBAL_SET");
      return 1;
    case OP_GLOBAL_GET:
      printf("OP_GLOBAL_GET");
      return 1;
    case OP_NOT:
      printf("OP_NOT");
      return 1;
    case OP_NEGATE:
      printf("OP_NEGATE");
      return 1;
    case OP_ADD: 
      printf("OP_ADD");
      return 1;
    case OP_SUB:
      printf("OP_SUB");
      return 1;
    case OP_MUL:
      printf("OP_MUL");
      return 1;
    case OP_DIV:
      printf("OP_DIV");
      return 1;
    case OP_LT:
      printf("OP_LT");
      return 1;
    case OP_GT:
      printf("OP_GT");
      return 1;
    case OP_LTE:
      printf("OP_LTE");
      return 1;
    case OP_GTE:
      printf("OP_GTE");
      return 1;
    case OP_EQ:
      printf("OP_EQ");
      return 1;
    case OP_NEQ:
      printf("OP_NEQ");
      return 1;
    case OP_CONSTANT:
      printf("OP_CONSTANT [%d]", *(uint8_t*)block->opcodes->data[index + 1]);
      return 2;
    case OP_CALL:
      printf("OP_CALL [%d]", *(uint8_t*)block->opcodes->data[index + 1]);
      return 2;
    case OP_LOCAL_GET: {
      uint8_t slot = *(uint8_t*)block->opcodes->data[index + 1];
      printf("OP_LOCAL_GET [%d]", slot);
      return 2;
    }
    case OP_LOCAL_SET: {
      uint8_t slot = *(uint8_t*)block->opcodes->data[index + 1];
      printf("OP_LOCAL_SET [%d]", slot);
      return 2;
    }
    case OP_FIELD_GET: {
      printf("OP_FIELD_GET");
      return 1;
    }
    case OP_FIELD_SET: {
      printf("OP_FIELD_SET");
      return 1;
    }
    case OP_CJUMPF: {
      uint16_t addr = *(uint8_t*)block->opcodes->data[index + 1] << 8 |
                      *(uint8_t*)block->opcodes->data[index + 2];
      printf("OP_CJUMPF [%d]", addr);
      return 3;
    }
    case OP_CJUMPT: {
      uint16_t addr = *(uint8_t*)block->opcodes->data[index + 1] << 8 |
                      *(uint8_t*)block->opcodes->data[index + 2];
      printf("OP_CJUMPT [%d]", addr);
      return 3;
    }
    case OP_JUMP: {
      uint16_t addr = *(uint8_t*)block->opcodes->data[index + 1] << 8 |
                      *(uint8_t*)block->opcodes->data[index + 2];
      printf("OP_JUMP [%d]", addr);
      return 3;
    }
    case OP_JUMP_BACK: {
      uint16_t addr = *(uint8_t*)block->opcodes->data[index + 1] << 8 |
                      *(uint8_t*)block->opcodes->data[index + 2];
      printf("OP_JUMP_BACK [%d]", addr);
      return 3;
    }
    default:
      printf("Unknown opcode: %d", *opcode);
      return 1;
  }
}