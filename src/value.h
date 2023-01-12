
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
#include <stdbool.h>

#include "object.h"

enum ValueType { VAL_NULL, VAL_BOOL, VAL_INTEGER_32, VAL_OBJ };

typedef struct {
  enum ValueType type;
  union data {
    int integer_32;
    bool boolean;
    PObject* reference;
  } data;
} Value;

// returns the truthiness of a value (see function)
bool value_is_truthy(Value* value);

// clones a value as a heap-allocated value
Value* value_clone(Value* value);

// prints a value's data
void value_print(Value* value);
// prints the type of a value
void value_type_print(enum ValueType value);
// frees the memory allocated by a value
void value_free(Value* value);

/////////////////////////////
// Value definition macros
/////////////////////////////

/**
 * @brief Allocates and returns a new 32-bit integer value.
 *
 * @param data the data to store in the value
 * @return Value* a pointer to the newly allocated value
 */
#define value_new_int_32(val) \
  ((Value){.type = VAL_INTEGER_32, .data.integer_32 = (val)})

/**
 * @brief Allocates and returns a new null value.
 *
 * @return Value* a pointer to the newly allocated value
 */
#define value_new_null() ((Value){.type = VAL_NULL, .data.integer_32 = 0})

/**
 * @brief Allocates a new object value and returns a pointer to it.
 *
 */
#define value_new_object(val) ((Value){.type = VAL_OBJ, .data.reference = val})

/**
 * @brief Allocates a new boolean value and returns a pointer to it.
 *
 * @param data the data to store in the value
 * @return Value* a pointer to the newly allocated value
 */
#define value_new_boolean(val) \
  ((Value){.type = VAL_BOOL, .data.boolean = (val)})

#endif