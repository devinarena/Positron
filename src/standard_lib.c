

/**
 * @file standard_lib.c
 * @author Devin Arena
 * @brief Contains the standard library for the language.
 * @since 2/27/2023
 **/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "standard_lib.h"

/**
 * @brief Prints a value to the console.
 *
 * @param argc the number of arguments
 * @param args the arguments
 * @return Value
 */
Value p_wln(PObject* parent, size_t argc, Value* args) {
  assert(argc == 1);
  value_print(args);
  printf("\n");
  return value_new_null();
}

Value p_abs(PObject* parent, size_t argc, Value* args) {
  assert(argc == 1);
  if (args[0].type != VAL_NUMBER) {
    printf("abs() only takes a number as an argument");
    exit(1);
  }
  return value_new_number(fabs(args[0].data.number));
}

Value p_clock(PObject* parent, size_t argc, Value* _args) {
  assert(argc == 0);
  return value_new_number((double)clock());
}

/**
 * @brief Builtin methods for lists.
 */

Value p_list_size(PObject* parent, size_t argc, Value* args) {
  assert(argc == 0);
  return value_new_number(((PList*)parent)->list->size);
}

Value p_list_add(PObject* parent, size_t argc, Value* args) {
  assert(argc == 1);
  dyn_list_add(((PList*)parent)->list, value_clone(args + 0));
  return value_new_null();
}

#define ADD_STD_LIB(name, argc)               \
  hash_table_set(                             \
      table, #name,                           \
      &value_new_object(p_object_builtin_new( \
          NULL, p_object_string_new_n(#name, sizeof(#name)), p_##name, argc)))

#define ADD_STD_LIB_N(name, function, argc)                             \
  hash_table_set(table, #name,                                          \
                 &value_new_object(p_object_builtin_new(                \
                     NULL, p_object_string_new_n(#name, sizeof(#name)), \
                     (#function), argc)))

/**
 * @brief Initializes the standard library.
 */
void init_standard_lib(HashTable* table) {
  ADD_STD_LIB(abs, 1);
  ADD_STD_LIB(wln, 1);
  ADD_STD_LIB(clock, 0);
}