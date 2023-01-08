
/**
 * @file interpreter.c
 * @author Devin Arena
 * @brief A bytecode VM that interprets the parsed tokens.
 * @since 1/7/2023
 **/

#include "interpreter.h"

Interpreter interpreter;

void interpreter_init() {
  interpreter.ip = 0;
  interpreter.stack = dyn_list_new((void*)(void*)value_free);
}

void interpret(Block* block) {
  interpreter_init();

  while (interpreter.ip < block->opcodes->size) {
    interpreter_print(block);
    switch (*(uint8_t*)block->opcodes->data[interpreter.ip]) {
      case OP_CONSTANT_INTEGER_32: {
        uint8_t index = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        Value* constant = block->constants->data[index];
        printf("%d", constant->data.integer_32);
        interpreter.ip++;
        break;
      }
    }
  }

  interpreter_print(block);
}

/**
 * @brief Debug for printing out the current state of the interpreter.
 *
 * @param block
 */
void interpreter_print(Block* block) {
  printf("\n=====================\n");
  printf("ip: %d,\n", (int)interpreter.ip);
  printf("opcode: ");
  if (interpreter.ip < block->opcodes->size) {
    block_print_opcode(block, interpreter.ip);
  }
  printf("\nstack: ");
  for (size_t i = 0; i < interpreter.stack->size; i++) {
    Value* value = interpreter.stack->data[i];
    value_print(value);
    printf(" ");
  }
}