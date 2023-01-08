
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
  interpreter.stack = dyn_list_new(NULL);
}

static Value* pop_stack() {
  if (interpreter.stack->size == 0) {
    printf("pop from empty stack");
    exit(1);
  }
  Value* value = interpreter.stack->data[interpreter.stack->size - 1];
  dyn_list_pop(interpreter.stack);
  return value;
}

static void push_stack(Value* value) {
  dyn_list_add(interpreter.stack, value);
}

void interpret(Block* block) {
  interpreter_init();

  while (interpreter.ip < block->opcodes->size) {
    interpreter_print(block);
    switch (*(uint8_t*)block->opcodes->data[interpreter.ip]) {
      case OP_CONSTANT_INTEGER_32: {
        uint8_t index = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        Value* constant = block->constants->data[index];
        push_stack(constant);
        interpreter.ip++;
        break;
      }
      case OP_NEGATE_INTEGER_32: {
        Value* v = pop_stack();
        v->data.integer_32 = -v->data.integer_32;
        push_stack(v);
        interpreter.ip++;
        break;
      }
      case OP_NOP: {
        interpreter.ip++;
        break;
      }
      default: {
        printf("Unknown opcode: %d",
               *(uint8_t*)block->opcodes->data[interpreter.ip]);
        exit(1);
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
  printf("\n==========================================\n");
  printf("ip: %d,\n", (int)interpreter.ip);
  printf("opcode: ");
  if (interpreter.ip < block->opcodes->size) {
    block_print_opcode(block, interpreter.ip);
  }
  printf("\nstack: ");
  for (size_t i = 0; i < interpreter.stack->size; i++) {
    Value* value = interpreter.stack->data[i];
    printf("[");
    value_print(value);
    printf("]");
  }
  printf("\n==========================================\n");
}

/**
 * @brief Frees the memory allocated by the interpreter.
 */
void interpreter_free() {
  dyn_list_free(interpreter.stack);
}