
/**
* @file standard_lib.h
* @author Devin Arena
* @brief Standard library for Positron.
* @since 2/27/2023
**/

#ifndef POSITRON_STANDARD_LIB_H
#define POSITRON_STANDARD_LIB_H

#include "object.h"
#include "value.h"

void init_standard_lib(HashTable* table);

Value p_list_size(PObject* parent, size_t argc, Value* args);
Value p_list_add(PObject* parent, size_t argc, Value* args);

#endif