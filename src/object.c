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
#include "interpreter.h"


static PObject* _p_object_new(PObjectType type, size_t size) {
  PObject* object = malloc(size);
  object->type = type;

  object->next = interpreter.heap;
  interpreter.heap = object;

  return object;
}

#define p_object_new(type, objType) \
  (type*)_p_object_new(objType, sizeof(type))

/**
 * @brief Allocates and returns a new string object.
 */
PString* p_object_string_new_n(const char* data, size_t length) {
  PString* string = p_object_new(PString, P_OBJ_STRING);
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
  PString* string = p_object_new(PString, P_OBJ_STRING);
  string->length = strlen(data);
  string->value = strdup(data);
  return string;
}

/**
 * @brief Allocates and returns a new function object.
 */
PFunction* p_object_function_new(PString* name) {
  PFunction* function = p_object_new(PFunction, P_OBJ_FUNCTION);
  function->name = name;
  function->arity = 0;
  function->block = block_new();
  return function;
}

/**
 * @brief Allocates and returns a new builtin function object.
 * 
 */
PBuiltin* p_object_builtin_new(PString* name, BuiltinFn function) {
  PBuiltin* builtin = p_object_new(PBuiltin, P_OBJ_BUILTIN);
  builtin->name = name;
  builtin->function = *function;
  return builtin;
}

/**
 * @brief Outputs the objects type to stdout.
 * 
 * @param type the type of the object
 */
void p_object_type_print(PObject* object) {
  switch (object->type) {
    case P_OBJ_STRING:
      printf("string");
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
      printf("<fun %s>", ((PFunction*)object)->name->value);
      break;
    case P_OBJ_BUILTIN:
      printf("<builtin %s>", ((PBuiltin*)object)->name->value);
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
    case P_OBJ_BUILTIN: {
      PBuiltin* builtin = (PBuiltin*)object;
      p_object_free((PObject*)builtin->name);
      p_object_free((PObject*)builtin->name);
      break;
    }
    default:
      break;
  }
  free(object);
}