
/**
 * @file interpreter.c
 * @author Devin Arena
 * @brief A bytecode VM that interprets the parsed tokens.
 * @since 1/7/2023
 **/

#include <stdio.h>

#include "interpreter.h"
#include "positron.h"

Interpreter interpreter;

/**
 * @brief Initializes the interpreter.
 */
void interpreter_init() {
  interpreter.ip = 0;
  interpreter.sp = 0;
  hash_table_init(&interpreter.globals);
}

/**
 * @brief Pops and returns value from the stack.
 *
 * @return Value* the value popped from the stack
 */
static Value* pop_stack() {
  if (interpreter.sp < 0) {
    printf("pop from empty stack");
    exit(1);
  }
  Value* value = interpreter.stack[--interpreter.sp];
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
  interpreter.stack[interpreter.sp++] = value;
}

/**
 * @brief Peeks at a local variable at a specific stack depth.
 *
 * @param depth the depth to peek at
 * @return Value* the value at the depth
 */
static Value* peek_stack(int depth) {
  if (interpreter.sp < depth + 1) {
    printf("peek depth exceeds stack size");
    exit(1);
  }
  return interpreter.stack[interpreter.sp - depth - 1];
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
    if (DEBUG_MODE)
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
      case OP_DUPE: {
        Value* v = peek_stack(0);
        push_stack(v);
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
        push_stack(&value_new_int_32(-v->data.integer_32));
        interpreter.ip++;
        break;
      }
      case OP_ADD_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        push_stack(
            &value_new_int_32(v1->data.integer_32 + v2->data.integer_32));
        interpreter.ip++;
        break;
      }
      case OP_SUBTRACT_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        push_stack(
            &value_new_int_32(v1->data.integer_32 - v2->data.integer_32));
        interpreter.ip++;
        break;
      }
      case OP_MULTIPLY_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        push_stack(
            &value_new_int_32(v1->data.integer_32 * v2->data.integer_32));
        push_stack(v1);
        interpreter.ip++;
        break;
      }
      case OP_DIVIDE_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        push_stack(
            &value_new_int_32(v1->data.integer_32 / v2->data.integer_32));
        push_stack(v1);
        interpreter.ip++;
        break;
      }
      case OP_COMPARE_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        push_stack(
            &value_new_boolean(v1->data.integer_32 == v2->data.integer_32));
        interpreter.ip++;
        break;
      }
      case OP_GREATER_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        push_stack(
            &value_new_boolean(v1->data.integer_32 > v2->data.integer_32));
        interpreter.ip++;
        break;
      }
      case OP_LESS_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        push_stack(
            &value_new_boolean(v1->data.integer_32 < v2->data.integer_32));
        interpreter.ip++;
        break;
      }
      case OP_GREATER_EQUAL_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        push_stack(
            &value_new_boolean(v1->data.integer_32 >= v2->data.integer_32));
        interpreter.ip++;
        break;
      }
      case OP_LESS_EQUAL_INTEGER_32: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        push_stack(
            &value_new_boolean(v1->data.integer_32 <= v2->data.integer_32));
        interpreter.ip++;
        break;
      }
      case OP_COMPARE_BOOLEAN: {
        Value* v2 = pop_stack();
        Value* v1 = pop_stack();
        push_stack(&value_new_boolean(v1->data.boolean == v2->data.boolean));
        interpreter.ip++;
        break;
      }
      case OP_NOT: {
        Value* v = pop_stack();
        bool data = !value_is_truthy(v);
        v->type = VAL_BOOL;
        v->data.boolean = data;
        push_stack(v);
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
        hash_table_set(&interpreter.globals, TO_STRING(*name)->value,
                       &value_new_null());
        interpreter.ip++;
        break;
      }
      case OP_GLOBAL_SET: {
        Value* name = pop_stack();
        Value* value = pop_stack();
        hash_table_set(&interpreter.globals, TO_STRING(*name)->value, value);
        interpreter.ip++;
        break;
      }
      case OP_GLOBAL_GET: {
        Value* name = pop_stack();
        Value* value =
            hash_table_get(&interpreter.globals, TO_STRING(*name)->value);
        push_stack(value_clone(value));
        interpreter.ip++;
        break;
      }
      case OP_LOCAL_GET: {
        uint8_t index = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        push_stack(interpreter.stack[index]);
        interpreter.ip++;
        break;
      }
      case OP_LOCAL_SET: {
        uint8_t index = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        interpreter.stack[index] = peek_stack(0);
        if (interpreter.sp != 1)
          pop_stack();
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
      case OP_CJUMPT: {
        Value* condition = pop_stack();
        uint8_t high = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        uint8_t low = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        uint16_t offset = (high << 8) | low;
        if (condition->data.boolean == true) {
          interpreter.ip += offset;
        } else {
          interpreter.ip++;
        }
        break;
      }
      case OP_JUMP: {
        uint8_t high = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        uint8_t low = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        uint16_t offset = (high << 8) | low;
        interpreter.ip += offset;
        break;
      }
      case OP_JUMP_BACK: {
        uint8_t high = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        uint8_t low = *(uint8_t*)block->opcodes->data[++interpreter.ip];
        uint16_t offset = (high << 8) | low;
        interpreter.ip -= offset;
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
  if (DEBUG_MODE)
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
  for (int i = 0; i < interpreter.sp; i++) {
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