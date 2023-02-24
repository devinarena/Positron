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
PFunction* p_object_function_new(PString* name, Value returnType) {
  PFunction* function = p_object_new(PFunction, P_OBJ_FUNCTION);
  function->name = name;
  function->arity = 0;
  function->block = block_new();
  function->returnType = returnType;
  return function;
}

/**
 * @brief Allocates and returns a new struct object.
 */
PStruct* p_object_struct_new(PString* name) {
  PStruct* struct_ = p_object_new(PStruct, P_OBJ_STRUCT);
  struct_->name = name;
  hash_table_init(&struct_->fields);
  return struct_;
}

/**
 * @brief Allocates and returns a new struct instance object.
 */
PStructInstance* p_object_struct_instance_new(PStruct* template) {
  PStructInstance* instance = p_object_new(PStructInstance, P_OBJ_STRUCT_INSTANCE);
  instance->template = template;
  instance->slots = malloc(sizeof(Value) * template->fields.count);

  for (int i = 0; i < template->fields.capacity; i++) {
    Entry* entry = &template->fields.entries[i];
    if (entry->key == NULL || entry->value == NULL) continue;
    instance->slots[entry->value->data.integer_32] = *entry->value;
  }

  return instance;
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
    case P_OBJ_FUNCTION:
      printf("object");
      break;
    case P_OBJ_STRUCT:
      printf("struct");
      break;
    case P_OBJ_STRUCT_INSTANCE:
      printf("struct instance");
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
      value_print_type(&((PFunction*)object)->returnType);
      printf(" %s>", ((PFunction*)object)->name->value);
      break;
    case P_OBJ_STRUCT:
      printf("<struct %p>", object);
      break;
    case P_OBJ_STRUCT_INSTANCE:
      printf("<struct instance %p>", object);
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
    case P_OBJ_STRUCT: {
      PStruct* struct_ = (PStruct*)object;
      p_object_free((PObject*)struct_->name);
      hash_table_free(&struct_->fields);
      break;
    }
    case P_OBJ_STRUCT_INSTANCE: {
      PStructInstance* instance = (PStructInstance*)object;
      free(instance->slots);
      break;
    }
    default:
      break;
  }
  free(object);
}