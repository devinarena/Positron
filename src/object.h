
/**
 * @file object.h
 * @author Devin Arena
 * @brief Representation of objects in Positron.
 * @since 1/8/2023
 **/

#ifndef POSITRON_OBJECT_H
#define POSITRON_OBJECT_H

#define IS_TYPE(val, type) _p_object_check((val), (type))

#define TO_STRING(val) ((PString*)(val).data.reference)
#define TO_FUNCTION(val) ((PFunction*)(val).data.reference)
#define TO_STRUCT(val) ((PStruct*)(val).data.reference)
#define TO_STRUCT_INSTANCE(val) ((PStructInstance*)(val).data.reference)
#define TO_LIST(val) ((PList*)(val).data.reference)

#define LIST_GROW_FACTOR 2

#include <stdint.h>

#include "block.h"
#include "value.h"
#include "hash_table.h"
#include "dyn_list.h"

typedef enum PObjectType {
  P_OBJ,
  P_OBJ_STRING,
  P_OBJ_FUNCTION,
  P_OBJ_BUILTIN,
  P_OBJ_STRUCT_TEMPLATE,
  P_OBJ_STRUCT_INSTANCE,
  P_OBJ_LIST
} PObjectType;

struct PObject {
  PObjectType type;
  PObject* next;
};

typedef struct PString {
  PObject base;
  char* value;
  size_t length;
} PString;

typedef struct PFunction {
  PObject base;
  PString* name;
  Block* block;
  size_t arity;
} PFunction;

typedef Value (*BuiltinFn)(PObject* parent, size_t argc, Value* args);

typedef struct PBuiltin {
  PObject base;
  PObject* parent;
  PString* name;
  size_t arity;
  BuiltinFn function;
} PBuiltin;

typedef struct PStructTemplate {
  PObject base;
  PString* name;
  HashTable fields;
} PStructTemplate;

typedef struct PStructInstance {
  PObject base;
  PStructTemplate* template;
  HashTable fields;
} PStructInstance;

typedef struct PList {
  PObject base;
  HashTable methods;
  dyn_list* list;
} PList;

// allocates and returns a new PString.
PString* p_object_string_new_n(const char* data, size_t length);
PString* p_object_string_new(const char* data);
// allocates and returns a new PFunction.
PFunction* p_object_function_new(PString* name);
// allocates and returns a new PBuiltin.
PBuiltin* p_object_builtin_new(PObject* parent, PString* name, BuiltinFn function, size_t argc);
// allocates and returns a new PStructTemplate.
PStructTemplate* p_object_struct_template_new(PString* name);
// allocates and returns a new PStructInstance.
PStructInstance* p_object_struct_instance_new(PStructTemplate* template);
// allocates and returns a new PList.
PList* p_object_list_new();
// prints the type of the given PObject.
void p_object_type_print(PObject* object);
// prints the given PObject.
void p_object_print(PObject* object);
// frees the given PObject.
void p_object_free(PObject* object);

// inline helper for macro
static inline bool _p_object_check(Value value, PObjectType type) {
  return value.type == VAL_OBJ &&
         ((PObject*)value.data.reference)->type == type;
}

#endif