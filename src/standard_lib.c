

/**
 * @file standard_lib.c
 * @author Devin Arena
 * @brief Contains the standard library for the language.
 * @since 2/27/2023
 **/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "standard_lib.h"

Value p_abs(size_t argc, Value* args) {
  if (argc != 1) {
    printf("abs() takes exactly 1 arguments (%lld given)", argc);
    exit(1);
  }
  if (args[0].type != VAL_NUMBER) {
    printf("abs() only takes a number as an argument");
    exit(1);
  }
  return value_new_number(fabs(args[0].data.number));
}

/**
 * @brief Initializes the standard library.
 */
void init_standard_lib(HashTable* table) {
  hash_table_set(table, "abs",
                 &value_new_object(p_object_builtin_new(
                     p_object_string_new_n("abs", 3), p_abs)));
}