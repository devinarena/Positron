
/**
 * @file interpreter.c
 * @author Devin Arena
 * @brief A bytecode VM that interprets the parsed tokens.
 * @since 1/7/2023
 **/

#include <stdio.h>

#include "interpreter.h"
#include "positron.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

Interpreter interpreter;

/**
 * @brief Initializes the interpreter.
 */
void interpreter_init() {
  interpreter.fp = 0;
  interpreter.sp = 0;
  hash_table_init(&interpreter.globals);
}

/**
 * @brief Pops and returns value from the stack.
 *
 * @return Value the value popped from the stack
 */
static Value pop_stack() {
  if (interpreter.sp <= 0) {
    printf("pop from empty stack");
    exit(1);
  }
  Value value = interpreter.stack[--interpreter.sp];
  return value;
}

/**
 * @brief Pushes a value onto the stack.
 *
 * @param value the value to push onto the stack
 */
static void push_stack(Value value) {
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
 * @return Value the value at the depth
 */
static Value peek_stack(int depth) {
  if (interpreter.sp < depth + 1) {
    printf("peek depth exceeds stack size");
    exit(1);
  }
  return interpreter.stack[interpreter.sp - depth - 1];
}

static void push_frame(CallFrame frame) {
  if (interpreter.fp == MAX_FRAMES) {
    printf("frame stack overflow");
    exit(1);
  }
  interpreter.frames[interpreter.fp++] = frame;
}

static void pop_frame() {
  if (interpreter.fp < 0) {
    printf("pop from empty frame stack");
    exit(1);
  }
  interpreter.fp--;
}

/**
 * @brief Interprets the emitted opcodes of a frame->function.
 *
 * @param frame->function the frame->function to interpret
 * @return InterpretResult the result of the interpretation
 */
InterpretResult interpret(PFunction* function) {
  interpreter_init();

  push_frame(
      (CallFrame){.ip = 0, .function = function, .slotCount = function->arity});
  CallFrame* frame = &interpreter.frames[interpreter.fp - 1];
  frame->slots = interpreter.stack;

  while (frame->ip < frame->function->block->opcodes->size) {
#ifdef POSITRON_DEBUG
    if (DEBUG_MODE)
      interpreter_print();
#endif
    switch (*(uint8_t*)frame->function->block->opcodes->data[frame->ip]) {
      case OP_NOP: {
        frame->ip++;
        break;
      }
      case OP_POP: {
        pop_stack();
        frame->ip++;
        break;
      }
      case OP_DUPE: {
        Value v = peek_stack(0);
        push_stack(v);
        frame->ip++;
        break;
      }
      case OP_EXIT: {
        Value res = pop_stack();
        return (InterpretResult)res.data.integer_32;
      }
      case OP_CALL: {
        uint8_t arg_count =
            *(uint8_t*)frame->function->block->opcodes->data[frame->ip + 1];
        Value fun = peek_stack(arg_count);
        if (fun.type != VAL_OBJ || fun.data.reference->type != P_OBJ_FUNCTION) {
          printf("Cannot call non-function");
          exit(1);
        }
        frame->ip += 2;
        push_frame(
            (CallFrame){.ip = 0, .function = (PFunction*)fun.data.reference});
        frame = &interpreter.frames[interpreter.fp - 1];
        frame->slots = &interpreter.stack[interpreter.sp - arg_count];
        frame->slotCount = arg_count;
        break;
      }
      case OP_RETURN: {
        Value res = value_new_null();
        if (frame->function->returnType.type != VAL_NULL) {
          res = pop_stack();
        }
        pop_frame();
        if (interpreter.fp <= 0) {
          return INTERPRET_OK;
        }
        frame = &interpreter.frames[interpreter.fp - 1];
        frame->slots = &interpreter.stack[interpreter.sp];
        push_stack(res);
        break;
      }
      case OP_PRINT: {
        Value v = pop_stack();
        value_print(&v);
        printf("\n");
        frame->ip++;
        break;
      }
      case OP_NEGATE_INTEGER_32: {
        Value v = pop_stack();
        push_stack(value_new_int_32(-v.data.integer_32));
        frame->ip++;
        break;
      }
      case OP_ADD_INTEGER_32: {
        Value v2 = pop_stack();
        Value v1 = pop_stack();
        push_stack(value_new_int_32(v1.data.integer_32 + v2.data.integer_32));
        frame->ip++;
        break;
      }
      case OP_SUBTRACT_INTEGER_32: {
        Value v2 = pop_stack();
        Value v1 = pop_stack();
        push_stack(value_new_int_32(v1.data.integer_32 - v2.data.integer_32));
        frame->ip++;
        break;
      }
      case OP_MULTIPLY_INTEGER_32: {
        Value v2 = pop_stack();
        Value v1 = pop_stack();
        push_stack(value_new_int_32(v1.data.integer_32 * v2.data.integer_32));
        push_stack(v1);
        frame->ip++;
        break;
      }
      case OP_DIVIDE_INTEGER_32: {
        Value v2 = pop_stack();
        Value v1 = pop_stack();
        push_stack(value_new_int_32(v1.data.integer_32 / v2.data.integer_32));
        push_stack(v1);
        frame->ip++;
        break;
      }
      case OP_COMPARE_INTEGER_32: {
        Value v2 = pop_stack();
        Value v1 = pop_stack();
        push_stack(value_new_boolean(v1.data.integer_32 == v2.data.integer_32));
        frame->ip++;
        break;
      }
      case OP_GREATER_INTEGER_32: {
        Value v2 = pop_stack();
        Value v1 = pop_stack();
        push_stack(value_new_boolean(v1.data.integer_32 > v2.data.integer_32));
        frame->ip++;
        break;
      }
      case OP_LESS_INTEGER_32: {
        Value v2 = pop_stack();
        Value v1 = pop_stack();
        push_stack(value_new_boolean(v1.data.integer_32 < v2.data.integer_32));
        frame->ip++;
        break;
      }
      case OP_GREATER_EQUAL_INTEGER_32: {
        Value v2 = pop_stack();
        Value v1 = pop_stack();
        push_stack(value_new_boolean(v1.data.integer_32 >= v2.data.integer_32));
        frame->ip++;
        break;
      }
      case OP_LESS_EQUAL_INTEGER_32: {
        Value v2 = pop_stack();
        Value v1 = pop_stack();
        push_stack(value_new_boolean(v1.data.integer_32 <= v2.data.integer_32));
        frame->ip++;
        break;
      }
      case OP_COMPARE_BOOLEAN: {
        Value v2 = pop_stack();
        Value v1 = pop_stack();
        push_stack(value_new_boolean(v1.data.boolean == v2.data.boolean));
        frame->ip++;
        break;
      }
      case OP_NOT: {
        Value v = pop_stack();
        bool data = !value_is_truthy(&v);
        v.type = VAL_BOOL;
        v.data.boolean = data;
        push_stack(v);
        frame->ip++;
        break;
      }
      case OP_CONSTANT: {
        uint8_t index =
            *(uint8_t*)frame->function->block->opcodes->data[++frame->ip];
        Value constant =
            *(Value*)frame->function->block->constants->data[index];
        push_stack(constant);
        frame->ip++;
        break;
      }
      case OP_GLOBAL_DEFINE: {
        Value name = pop_stack();
        hash_table_set(&interpreter.globals, TO_STRING(name)->value,
                       &value_new_null());
        frame->ip++;
        break;
      }
      case OP_GLOBAL_SET: {
        Value name = pop_stack();
        Value value = pop_stack();
        hash_table_set(&interpreter.globals, TO_STRING(name)->value, &value);
        frame->ip++;
        break;
      }
      case OP_GLOBAL_GET: {
        Value name = pop_stack();
        Value value =
            *hash_table_get(&interpreter.globals, TO_STRING(name)->value);
        push_stack(value);
        frame->ip++;
        break;
      }
      case OP_LOCAL_GET: {
        uint8_t index =
            *(uint8_t*)frame->function->block->opcodes->data[++frame->ip];
        push_stack(frame->slots[index]);
        frame->ip++;
        break;
      }
      case OP_LOCAL_SET: {
        uint8_t index =
            *(uint8_t*)frame->function->block->opcodes->data[++frame->ip];
        frame->slots[index] = peek_stack(0);
        if (interpreter.sp > (frame->slots - interpreter.stack) + index + 1)
          pop_stack();
        frame->ip++;
        break;
      }
      case OP_CJUMPF: {
        Value condition = pop_stack();
        uint8_t high =
            *(uint8_t*)frame->function->block->opcodes->data[++frame->ip];
        uint8_t low =
            *(uint8_t*)frame->function->block->opcodes->data[++frame->ip];
        uint16_t offset = (high << 8) | low;
        if (condition.data.boolean == false) {
          frame->ip += offset;
        } else {
          frame->ip++;
        }
        break;
      }
      case OP_CJUMPT: {
        Value condition = pop_stack();
        uint8_t high =
            *(uint8_t*)frame->function->block->opcodes->data[++frame->ip];
        uint8_t low =
            *(uint8_t*)frame->function->block->opcodes->data[++frame->ip];
        uint16_t offset = (high << 8) | low;
        if (condition.data.boolean == true) {
          frame->ip += offset;
        } else {
          frame->ip++;
        }
        break;
      }
      case OP_JUMP: {
        uint8_t high =
            *(uint8_t*)frame->function->block->opcodes->data[++frame->ip];
        uint8_t low =
            *(uint8_t*)frame->function->block->opcodes->data[++frame->ip];
        uint16_t offset = (high << 8) | low;
        frame->ip += offset;
        break;
      }
      case OP_JUMP_BACK: {
        uint8_t high =
            *(uint8_t*)frame->function->block->opcodes->data[++frame->ip];
        uint8_t low =
            *(uint8_t*)frame->function->block->opcodes->data[++frame->ip];
        uint16_t offset = (high << 8) | low;
        frame->ip -= offset;
        break;
      }
      default: {
        printf("Unknown opcode: %d\n",
               *(uint8_t*)frame->function->block->opcodes->data[frame->ip]);
        exit(1);
      }
    }
  }

#ifdef POSITRON_DEBUG
  if (DEBUG_MODE)
    interpreter_print();
#endif

  return INTERPRET_OK;
}

/**
 * @brief Debug for printing out the current state of the interpreter.
 */
void interpreter_print() {
  CallFrame* frame = &interpreter.frames[max(interpreter.fp - 1, 0)];

  printf("\n==========================================\n");
  printf("fp: %d,\n", interpreter.fp);
  printf("sp: %d,\n", interpreter.sp);
  printf("ip: %d,\n", (int)frame->ip);
  printf("opcode: ");
  if (frame->ip < frame->function->block->opcodes->size) {
    block_print_opcode(frame->function->block, frame->ip);
  }
  printf("\nstack: ");
  if (interpreter.sp == 0) {
    printf("{}");
  } else {
    for (int i = 0; i < interpreter.sp; i++) {
      if ((i == 0 && interpreter.fp == 1) ||
          i == frame->slots - interpreter.stack)
        printf("{");
      Value value = interpreter.stack[i];
      printf("[");
      value_print(&value);
      printf("]");
    }
    printf("}");
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