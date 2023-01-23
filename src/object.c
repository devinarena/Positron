/**
 * @file object.c
 * @author Devin Arena
 * @brief Representation of objects in Positron.
 * @since 1/8/2023
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"

/**
 * @brief Allocates and returns a new object.
 *
 * @param type the type of the object
 * @return Object* a pointer to the newly allocated object
 */
PObject* p_object_new(PObjectType type) {
  PObject* object = malloc(sizeof(PObject));
  object->type = type;
  return object;
}

/**
 * @brief Allocates and returns a new string object.
 */
PString* p_object_string_new_n(const char* data, size_t length) {
  PString* string = malloc(sizeof(PString));
  string->base = *p_object_new(P_OBJ_STRING);
  string->length = length;

  string->value = malloc(length + 1);
  memcpy(string->value, data, length);
  string->value[length] = '\0';
  
  return string;
}

/**
 * @brief Allocates a new pstring from a const char*.
 * 
 * @param data the data to copy into the string
 * @return PString* the newly allocated string
 */
PString* p_object_string_new(const char* data) {
  PString* string = malloc(sizeof(PString));
  string->base = *p_object_new(P_OBJ_STRING);
  string->length = strlen(data);
  string->value = strdup(data);
  return string;
}

/**
 * @brief Allocates and returns a new function object.
 */
PFunction* p_object_function_new(PString* name, Value returnType) {
  PFunction* function = malloc(sizeof(PFunction));
  function->base = *p_object_new(P_OBJ_FUNCTION);
  function->name = name;
  function->arity = 0;
  function->block = block_new();
  function->returnType = returnType;
  return function;
}

/**
 * @brief Outputs the objects type to stdout.
 * 
 * @param type the type of the object
 */
void p_object_type_print(PObjectType type) {
  switch (type) {
    case P_OBJ_STRING:
      printf("string");
      break;
    case P_OBJ_FUNCTION:
      printf("object");
      break;
    default:
      printf("object");
      break;
  }
}

/**
 * @brief Prints the given object.
 */
void p_object_print(PObject* object) {
  switch (object->type) {
    case P_OBJ_STRING:
      printf(((PString*)object)->value);
      break;
    case P_OBJ_FUNCTION:
      printf("<");
      value_print(&(((PFunction*)object)->returnType));
      printf(" %s>", ((PFunction*)object)->name->value);
      break;
    default:
      printf("<object %p>", object);
      break;
  }
}

/**
 * @brief Frees the memory allocated by an object.
 */
void p_object_free(PObject* object) {
  switch (object->type) {
    case P_OBJ_STRING: {
      PString* string = (PString*)object;
      free(string->value);
      break;
    }
    case P_OBJ_FUNCTION: {
      PFunction* function = (PFunction*)object;
      p_object_free((PObject*)function->name);
      block_free(function->block);
      break;
    }
    default:
      break;
  }
  free(object);
}