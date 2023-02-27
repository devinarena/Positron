
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

#include <stdint.h>

#include "block.h"
#include "value.h"
#include "hash_table.h"
#include "dyn_list.h"

typedef enum PObjectType {
  P_OBJ,
  P_OBJ_STRING,
  P_OBJ_FUNCTION,
  P_OBJ_BUILTIN
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

typedef Value (*BuiltinFn)(size_t argc, Value* args);

typedef struct PBuiltin {
  PObject base;
  PString* name;
  size_t arity;
  BuiltinFn function;
} PBuiltin;

// allocates and returns a new PString.
PString* p_object_string_new_n(const char* data, size_t length);
PString* p_object_string_new(const char* data);
// allocates and returns a new PFunction.
PFunction* p_object_function_new(PString* name);
// allocates and returns a new PBuiltin.
PBuiltin* p_object_builtin_new(PString* name, BuiltinFn function);
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