
/**
 * @file interpreter.c
 * @author Devin Arena
 * @brief A bytecode VM that interprets the parsed tokens.
 * @since 1/7/2023
 **/

#include <stdio.h>

#include "interpreter.h"
#include "positron.h"
#include "standard_lib.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

Interpreter interpreter;

/**
 * @brief Initializes the interpreter.
 */
void interpreter_init() {
  interpreter.fp = 0;
  interpreter.sp = 0;
  interpreter.heap = NULL;
  hash_table_init(&interpreter.globals);
  init_standard_lib(&interpreter.globals);
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

static void call_object(CallFrame** frame, Value obj, size_t arg_count) {
  PObject* object = (PObject*)obj.data.reference;
  switch (object->type) {
    case P_OBJ_FUNCTION: {
      (*frame)->ip += 2;
      push_frame((CallFrame){.ip = 0, .function = (PFunction*)object});
      (*frame) = &interpreter.frames[interpreter.fp - 1];
      (*frame)->slots = &interpreter.stack[interpreter.sp - arg_count];
      (*frame)->slotCount = arg_count;
      break;
    }
    case P_OBJ_BUILTIN: {
      (*frame)->ip += 2;
      PBuiltin* builtin = (PBuiltin*)object;
      Value result = builtin->function(arg_count, &interpreter.stack[interpreter.sp - arg_count]);
      for (size_t i = 0; i < arg_count; i++) {
        pop_stack();
      }
      push_stack(result);
      break;
    }
    default: {
      printf("Expected callable object type.");
      exit(1);
    }
  }
}

/**
 * OPCODE FUNCTIONS
 */
static int negate() {
  Value a = pop_stack();
  switch (a.type) {
    case VAL_NUMBER:
      push_stack(value_new_number(-a.data.number));
      break;
    default:
      printf("Expected numeric value to negate.");
      exit(1);
  }
  return 1;
}

static int binary(enum TokenType op) {
  Value b = pop_stack();
  Value a = pop_stack();
  switch (op) {
    case TOKEN_PLUS: {
      if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        push_stack(value_new_number(a.data.number + b.data.number));
      } else {
        printf("Undefined operation for given values.");
        exit(1);
      }
      break;
    }
    case TOKEN_MINUS: {
      if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        push_stack(value_new_number(a.data.number - b.data.number));
      } else {
        printf("Undefined operation for given values.");
        exit(1);
      }
      break;
    }
    case TOKEN_STAR: {
      if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        push_stack(value_new_number(a.data.number * b.data.number));
      } else {
        printf("Undefined operation for given values.");
        exit(1);
      }
      break;
    }
    case TOKEN_SLASH: {
      if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        if (b.data.number == 0) {
          printf("Division by zero.");
          exit(1);
        }
        push_stack(value_new_number(a.data.number / b.data.number));
      } else {
        printf("Undefined operation for given values.");
        exit(1);
      }
      break;
    }
    case TOKEN_LESS: {
      if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        push_stack(value_new_boolean(a.data.number < b.data.number));
      } else {
        printf("Undefined operation for given values.");
        exit(1);
      }
      break;
    }
    case TOKEN_LESS_EQUAL: {
      if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        push_stack(value_new_boolean(a.data.number <= b.data.number));
      } else {
        printf("Undefined operation for given values.");
        exit(1);
      }
      break;
    }
    case TOKEN_GREATER: {
      if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        push_stack(value_new_boolean(a.data.number > b.data.number));
      } else {
        printf("Undefined operation for given values.");
        exit(1);
      }
      break;
    }
    case TOKEN_GREATER_EQUAL: {
      if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        push_stack(value_new_boolean(a.data.number >= b.data.number));
      } else {
        printf("Undefined operation for given values.");
        exit(1);
      }
      break;
    }
    case TOKEN_EQUAL_EQUAL: {
      if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        push_stack(value_new_boolean(a.data.number == b.data.number));
      } else {
        push_stack(value_new_boolean(false));
      }
      break;
    }
    case TOKEN_NOT_EQUAL: {
      if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        push_stack(value_new_boolean(a.data.number != b.data.number));
      } else {
        push_stack(value_new_boolean(true));
      }
      break;
    }
    default: {
      printf("Undefined binary operator.");
      exit(1);
    }
  }
  return 1;
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
      case OP_SWAP: {
        Value a = pop_stack();
        Value b = pop_stack();
        push_stack(a);
        push_stack(b);
        frame->ip++;
        break;
      }
      case OP_EXIT: {
        Value res = pop_stack();
        return (InterpretResult)res.data.number;
      }
      case OP_CALL: {
        uint8_t arg_count =
            *(uint8_t*)frame->function->block->opcodes->data[frame->ip + 1];
        Value callable = peek_stack(arg_count);
        if (callable.type != VAL_OBJ) {
          printf("Expected callable object type.");
          exit(1);
        }
        call_object(&frame, callable, arg_count);
        break;
      }
      case OP_RETURN: {
        Value res = value_new_null();
        // TODO: look at this potentially
        if (interpreter.stack + interpreter.sp - frame->slots -
                frame->slotCount >
            0) {
          res = pop_stack();
        }
        pop_frame();
        if (interpreter.fp <= 0) {
          return INTERPRET_OK;
        }
        for (size_t i = 0; i < frame->slotCount + 1; i++) {
          pop_stack();
        }
        frame = &interpreter.frames[interpreter.fp - 1];
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
      case OP_NOT: {
        Value v = pop_stack();
        bool data = !value_is_truthy(&v);
        v.type = VAL_BOOL;
        v.data.boolean = data;
        push_stack(v);
        frame->ip++;
        break;
      }
      case OP_NEGATE: {
        frame->ip += negate();
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
      case OP_ADD: {
        frame->ip += binary(TOKEN_PLUS);
        break;
      }
      case OP_SUB: {
        frame->ip += binary(TOKEN_MINUS);
        break;
      }
      case OP_MUL: {
        frame->ip += binary(TOKEN_STAR);
        break;
      }
      case OP_DIV: {
        frame->ip += binary(TOKEN_SLASH);
        break;
      }
      case OP_LT: {
        frame->ip += binary(TOKEN_LESS);
        break;
      }
      case OP_GT: {
        frame->ip += binary(TOKEN_GREATER);
        break;
      }
      case OP_LTE: {
        frame->ip += binary(TOKEN_LESS_EQUAL);
        break;
      }
      case OP_GTE: {
        frame->ip += binary(TOKEN_GREATER_EQUAL);
        break;
      }
      case OP_EQ: {
        frame->ip += binary(TOKEN_EQUAL_EQUAL);
        break;
      }
      case OP_NEQ: {
        frame->ip += binary(TOKEN_NOT_EQUAL);
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

  // Free the heap
  PObject* object = interpreter.heap;
  while (object != NULL) {
    PObject* next = object->next;
    p_object_free(object);
    object = next;
  }
}