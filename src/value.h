
/**
 * @file value.h
 * @author Devin Arena
 * @brief 1/6/2023
 * @since Represents a value in the language, such as a number, string, etc.
 **/

#ifndef POSITRON_VALUE_H
#define POSITRON_VALUE_H

#include <stdio.h>
#include <stdlib.h>

#include "object.h"

enum ValueType { VAL_NULL, VAL_INTEGER_32, VAL_OBJ };

typedef struct {
  enum ValueType type;
  union data {
    int integer_32;
    PObject* reference;
  } data;
} Value;

// allocates a new value and returns a pointer to it
Value* value_new_null();
// allocates a 32 bit integer value and returns a pointer to it
Value* value_new_int_32(int data);
// allocates a new object value and returns a pointer to it
Value* value_new_object(PObject* object);

// prints a value's data
void value_print(Value* value);
// prints the type of a value
void value_type_print(enum ValueType value);
// frees the memory allocated by a value
void value_free(Value* value);

#endif