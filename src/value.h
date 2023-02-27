
/**
 * @file value.h
 * @author Devin Arena
 * @brief 1/6/2023
 * @since Represents a value in the language, such as a number, string, etc.
 **/

#ifndef POSITRON_VALUE_H
#define POSITRON_VALUE_H

#include <stdbool.h>

#include "token.h"

typedef struct PObject PObject;

typedef enum ValueType {
  VAL_NULL,
  VAL_BOOL,
  VAL_NUMBER,
  VAL_OBJ
} ValueType;

typedef struct {
  enum ValueType type;
  union data {
    double number;
    bool boolean;
    PObject* reference;
  } data;
} Value;

// returns the truthiness of a value (see function)
bool value_is_truthy(Value* value);

// clones a value as a heap-allocated value
Value* value_clone(Value* value);
// Gets the value type associated with a token type
ValueType value_type_from_token_type(enum TokenType type);

// prints a value's data
void value_print(Value* value);
// prints the type of a value
void value_print_type(Value* value);
// prints the type of a ValueType
void value_type_print_type(enum ValueType value);
// frees the memory allocated by a value
void value_free(Value* value);

/////////////////////////////
// Value definition macros
/////////////////////////////

/**
 * @brief Returns a new 32-bit floating point value.
 */
#define value_new_number(val) \
  ((Value){.type = VAL_NUMBER, .data.number = (val)})

/**
 * @brief Returns a new null value.
 */
#define value_new_null() ((Value){.type = VAL_NULL, .data.number = 0})

/**
 * @brief Returns a new object value.
 *
 * @param val a pointer to the object
 */
#define value_new_object(val) \
  ((Value){.type = VAL_OBJ, .data.reference = ((PObject*)val)})

/**
 * @brief Returns a new boolean value.
 *
 * @param data the data to store in the value
 */
#define value_new_boolean(val) \
  ((Value){.type = VAL_BOOL, .data.boolean = (val)})

#endif