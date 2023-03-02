

/**
 * @file standard_lib.c
 * @author Devin Arena
 * @brief Contains the standard library for the language.
 * @since 2/27/2023
 **/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "standard_lib.h"

Value p_abs(size_t argc, Value* args) {
  assert(argc == 1);
  if (args[0].type != VAL_NUMBER) {
    printf("abs() only takes a number as an argument");
    exit(1);
  }
  return value_new_number(fabs(args[0].data.number));
}

#define ADD_STD_LIB(name, argc)               \
  hash_table_set(                             \
      table, #name,                           \
      &value_new_object(p_object_builtin_new( \
          p_object_string_new_n(#name, sizeof(#name)), p_##name, argc)))

#define ADD_STD_LIB_N(name, function, argc)   \
  hash_table_set(                             \
      table, #name,                           \
      &value_new_object(p_object_builtin_new( \
          p_object_string_new_n(#name, sizeof(#name)), (#function), argc)))

/**
 * @brief Initializes the standard library.
 */
void init_standard_lib(HashTable* table) {
  ADD_STD_LIB(abs, 1);
}