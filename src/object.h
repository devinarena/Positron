
/**
* @file object.h
* @author Devin Arena
* @brief Representation of objects in Positron.
* @since 1/8/2023
**/

#ifndef POSITRON_OBJECT_H
#define POSITRON_OBJECT_H

#define TO_STRING(val) ((PString*)(val).data.reference)

#include <stdint.h>

enum PObjectType {
    P_OBJ,
    P_OBJ_STRING,
};

typedef struct PObject {
    enum PObjectType type;
    struct PObject* next;
} PObject;

typedef struct PString {
    PObject base;
    char* value;
    size_t length;
} PString;

// allocates and returns a new PObject.
PObject* p_object_new(enum PObjectType type);
PString* p_object_string_new_n(const char* data, size_t length);
PString* p_object_string_new(const char* data);
// prints the type of the given PObject.
void p_object_type_print(enum PObjectType type);
// prints the given PObject.
void p_object_print(PObject* object);
// frees the given PObject.
void p_object_free(PObject* object);

#endif