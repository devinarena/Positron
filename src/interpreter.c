
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
}

void interpret(Block* block) {
  interpreter_init();

  while (interpreter.ip < block->opcodes->size) {
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
}