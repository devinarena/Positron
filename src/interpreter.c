
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
  interpreter.sp = -1;
  hash_table_init(&interpreter.globals);
}

static Value* pop_stack() {
  if (interpreter.sp < 0) {
    printf("pop from empty stack");
    exit(1);
  }
  Value* value = interpreter.stack[interpreter.sp--];
  return value;
}

/**
 * @brief Pushes a value onto the stack.
 *
 * @param value
 */
static void push_stack(Value* value) {
  if (interpreter.sp == STACK_SIZE) {
    printf("stack overflow");
    exit(1);
  }
  interpreter.stack[++interpreter.sp] = value;
}

/**
 * @brief Interprets the emitted opcodes of a block.
 *
 * @param block the block to interpret
 */
void interpret(Block* block) {
  interpreter_init();

  while (interpreter.ip < block->opcodes->size) {
#ifdef POSITRON_DEBUG
    interpreter_print(block);
#endif
    switch (*(uint8_t*)block->opcodes->data[interpreter.ip]) {
      case OP_NOP: {
        interpreter.ip++;
        break;
      }
      case OP_POP: {
        pop_stack();
        interpreter.ip++;
        break;
      }
      case OP_PRINT: {
        Value* v = pop_stack();
        value_print(v);
        printf("\n");
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
      case OP_ADD_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        v1->data.integer_32 += v2->data.integer_32;
        push_stack(v1);
        interpreter.ip++;
        break;
      }
      case OP_SUBTRACT_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        v1->data.integer_32 -= v2->data.integer_32;
        push_stack(v1);
        interpreter.ip++;
        break;
      }
      case OP_MULTIPLY_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        v1->data.integer_32 *= v2->data.integer_32;
        push_stack(v1);
        interpreter.ip++;
        break;
      }
      case OP_DIVIDE_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        v1->data.integer_32 /= v2->data.integer_32;
        push_stack(v1);
        interpreter.ip++;
        break;
      }
      case OP_CONSTANT: {
        uint8_t index = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        Value* constant = block->constants->data[index];
        push_stack(constant);
        interpreter.ip++;
        break;
      }
      case OP_GLOBAL_DEFINE: {
        Value* name = pop_stack();
        hash_table_set(&interpreter.globals, TO_STRING(name)->value, NULL);
        interpreter.ip++;
        break;
      }
      case OP_GLOBAL_SET: {
        Value* name = pop_stack();
        Value* value = pop_stack();
        hash_table_set(&interpreter.globals, TO_STRING(name)->value, value);
        interpreter.ip++;
        break;
      }
      case OP_GLOBAL_GET: {
        Value* name = pop_stack();
        Value* value = hash_table_get(&interpreter.globals, TO_STRING(name)->value);
        push_stack(value_clone(value));
        interpreter.ip++;
        break;
      }
      case OP_CJUMPF: {
        Value* condition = pop_stack();
        uint8_t high = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        uint8_t low = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        uint16_t offset = (high << 8) | low;
        if (condition->data.boolean == false) {
          interpreter.ip += offset;
        } else {
          interpreter.ip++;
        }
        break;
      }
      default: {
        printf("Unknown opcode: %d\n",
               *(uint8_t*)block->opcodes->data[interpreter.ip]);
        exit(1);
      }
    }
  }

#ifdef POSITRON_DEBUG
  interpreter_print(block);
#endif
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
  for (int i = 0; i <= interpreter.sp; i++) {
    Value* value = interpreter.stack[i];
    printf("[");
    value_print(value);
    printf("]");
  }
  printf("\nGlobals: ");
  hash_table_print(&interpreter.globals);
  printf("\n==========================================\n");
}

/**
 * @brief Frees the memory allocated by the interpreter.
 */
void interpreter_free() {
  hash_table_free(&interpreter.globals);
}