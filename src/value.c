
/**
 * @file value.c
 * @author Devin Arena
 * @brief 1/6/2023
 * @since Represents a value in the language, such as a number, string, etc.
 **/

#include "value.h"

/**
 * @brief Allocates and returns a new 32-bit integer value.
 *
 * @param data the data to store in the value
 * @return Value* a pointer to the newly allocated value
 */
Value* value_new_int_32(int data) {
  Value* value = malloc(sizeof(Value));
  value->type = VAL_INTEGER_32;
  value->data.integer_32 = data;
  return value;
}

/**
 * @brief Prints a value's data.
 *
 * @param value the value to print
 */
void value_print(Value* value) {
  switch (value->type) {
    case VAL_INTEGER_32:
      printf("%d", value->data.integer_32);
      break;
    default:
      printf("null");
      break;
  }
}

/**
 * @brief Prints the type of a value.
 *
 * @param value the value to print
 */
void value_type_print(enum ValueType type) {
  switch (type) {
    case VAL_INTEGER_32:
      printf("i32");
      break;
    default:
      printf("null");
      break;
  }
}

/**
 * @brief Frees the memory allocated by a value.
 */
void value_free(Value* value) {
    free(value);
}
