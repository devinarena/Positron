
/**
 * @file value.c
 * @author Devin Arena
 * @brief 1/6/2023
 * @since Represents a value in the language, such as a number, string, etc.
 **/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "object.h"
#include "value.h"

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
    case VAL_NUMBER:
      return fabs(value->data.number) > 0.00001f;
    case VAL_OBJ:
      return value->data.reference != NULL;
    default:
      return false;
  }
}

/**
 * @brief Clones a value as a heap allocated value.
 *
 * @param value the value to clone
 * @return Value* a newly allocated clone of that value
 */
Value* value_clone(Value* value) {
  Value* clone = malloc(sizeof(Value));

  if (!clone) {
    printf("Failed to allocate memory for value clone.\n");
    exit(1);
  }

  clone->type = value->type;
  clone->data = value->data;

  return clone;
}

/**
 * @brief Gets the value type associated with a token type.
 *
 * @param type the token type
 * @return enum ValueType the value type
 */
enum ValueType value_type_from_token_type(enum TokenType type) {
  switch (type) {
    case TOKEN_NULL:
      return VAL_NULL;
    case TOKEN_TRUE:
    case TOKEN_FALSE:
    case TOKEN_BOOL:
      return VAL_BOOL;
    case TOKEN_LITERAL_INTEGER:
      return VAL_NUMBER;
    case TOKEN_LITERAL_FLOATING:
      return VAL_NUMBER;
    case TOKEN_LITERAL_STRING:
    case TOKEN_IDENTIFIER:
      return VAL_OBJ;
    default:
      return VAL_NULL;
  }
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
    case VAL_NUMBER:
      // print as an integer if it's an integer
      if (value->data.number == (int)value->data.number)
        printf("%d", (int)value->data.number);
      else
        printf("%f", value->data.number);
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
void value_print_type(Value* value) {
  switch (value->type) {
    case VAL_OBJ:
      p_object_type_print(value->data.reference);
      break;
    default:
      value_type_print_type(value->type);
      break;
  }
}

/**
 * @brief Prints the type of a value type (the enum)
 *
 * @param type the value type to print
 */
void value_type_print_type(enum ValueType type) {
  switch (type) {
    case VAL_NULL:
      printf("null");
      break;
    case VAL_BOOL:
      printf("bool");
      break;
    case VAL_NUMBER:
      printf("f32");
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