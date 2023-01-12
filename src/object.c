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
PObject* p_object_new(enum PObjectType type) {
  PObject* object = malloc(sizeof(PObject));
  object->type = type;
  return object;
}

/**
 * @brief Allocates and returns a new string object.
 */
PString* p_object_string_new(const char* data) {
  PString* string = malloc(sizeof(PString));
  string->base = *p_object_new(P_OBJ_STRING);
  string->length = strlen(data);
  string->value = malloc(strlen(data));
  strcpy(string->value, data);
  return string;
}

void p_object_type_print(enum PObjectType type) {
  switch (type) {
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
    default:
      printf("<object %p>", object);
      break;
  }
}

/**
 * @brief Frees the memory allocated by an object.
 */
void object_free(PObject* object) {
  switch (object->type) {
    case P_OBJ_STRING: {
      PString* string = (PString*)object;
      free(string->value);
      break;
    }
    default:
      break;
  }
  free(object);
}