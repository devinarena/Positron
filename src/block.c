
/**
 * @file block.h
 * @author Devin Arena
 * @brief Represents blocks of code, stores information about them such as
 * constants, etc.
 * @since 1/6/2023
 **/

#include "block.h"

/**
 * @brief Allocates and returns a pointer to a new block.
 *
 * @param name the name of the block
 * @return Block* a pointer to the newly allocated block
 */
Block* block_new(const char* name) {
  Block* block = malloc(sizeof(Block));
  block->name = name;
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
 * @brief Adds a new constant to a block.
 *
 * @param block the block to add the constant to
 * @param constant the constant to add
 */
uint8_t block_new_constant(Block* block, Value* constant) {
  dyn_list_add(block->constants, (void*)constant);
  return (uint8_t)block->constants->size - 1;
}

/**
 * @brief Prints a block's information.
 *
 * @param block the block to print
 */
void block_print(Block* block) {
  printf("========== Block: %s ==========\n", block->name);
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
  free((void*)block->name);
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
    case OP_CONSTANT_INTEGER_32:
      printf("OP_CONSTANT_INTEGER_32 [%d]",
             *(uint8_t*)block->opcodes->data[index + 1]);
      return 2;
    case OP_NEGATE_INTEGER_32:
      printf("OP_NEGATE_INTEGER_32");
      return 1;
    case OP_ADD_INTEGER_32:
      printf("OP_ADD_INTEGER_32");
      return 1;
    case OP_SUBTRACT_INTEGER_32:
      printf("OP_SUBTRACT_INTEGER_32");
      return 1;
    case OP_MULTIPLY_INTEGER_32:
      printf("OP_MULTIPLY_INTEGER_32");
      return 1;
    case OP_DIVIDE_INTEGER_32:
      printf("OP_DIVIDE_INTEGER_32");
      return 1;
    default:
      printf("Unknown opcode: %d", *opcode);
      return 0;
  }
}