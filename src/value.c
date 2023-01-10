
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
 * @brief Allocates and returns a new null value.
 *
 * @return Value* a pointer to the newly allocated value
 */
Value* value_new_null() {
  Value* value = malloc(sizeof(Value));
  value->type = VAL_NULL;
  value->data.integer_32 = 0;
  return value;
}

/**
 * @brief Allocates a new object value and returns a pointer to it.
 *
 */
Value* value_new_object(PObject* object) {
  Value* value = malloc(sizeof(Value));
  value->type = VAL_OBJ;
  value->data.reference = object;
  return value;
}

/**
 * @brief Allocates a new boolean value and returns a pointer to it.
 *
 * @param data the data to store in the value
 * @return Value* a pointer to the newly allocated value
 */
Value* value_new_boolean(bool data) {
  Value* value = malloc(sizeof(Value));
  value->type = VAL_BOOL;
  value->data.boolean = data;
  return value;
}

/**
 * @brief Returns the truthiness of a value.
 * - null is always false
 * - bools are based on their boolean value
 * - integers and floats are true if != 0
 * - objects (pointers) are true if != NULL
 *
 * @param value
 * @return true
 * @return false
 */
bool value_is_truthy(Value* value) {
  switch (value->type) {
    case VAL_NULL:
      return false;
    case VAL_BOOL:
      return value->data.boolean;
    case VAL_INTEGER_32:
      return value->data.integer_32 != 0;
    case VAL_OBJ:
      return value->data.reference != NULL;
    default:
      return false;
  }
}

/**
 * @brief Clones a value.
 *
 * @param value the value to clone
 * @return Value* a newly allocated clone of that value
 */
Value* value_clone(Value* value) {
  Value* clone = malloc(sizeof(Value));
  clone->type = value->type;
  clone->data = value->data;
  return clone;
}

/**
 * @brief Prints a value's data.
 *
 * @param value the value to print
 */
void value_print(Value* value) {
  switch (value->type) {
    case VAL_NULL:
      printf("null");
      break;
    case VAL_BOOL:
      printf("%s", value->data.boolean ? "true" : "false");
      break;
    case VAL_INTEGER_32:
      printf("%d", value->data.integer_32);
      break;
    case VAL_OBJ:
      p_object_print(value->data.reference);
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
    case VAL_NULL:
      printf("null");
      break;
    case VAL_BOOL:
      printf("bool");
      break;
    case VAL_INTEGER_32:
      printf("i32");
      break;
    case VAL_OBJ:
      printf("obj");
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
