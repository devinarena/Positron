
#ifndef DYN_LIST_H
#define DYN_LIST_H

#define INITIAL_CAPACITY 8
#define GROWTH_FACTOR 2

#define PRIMITIVE 0
#define QSTRING 1

typedef struct {
  void** data;
  size_t size;
  size_t capacity;
  void (*free_object)(void*);
} dyn_list;

// allocates and returns a pointer to a new dynamic list
dyn_list* new_dyn_list(void (*free_object)(void*));
// frees the memory allocated by a dynamic list
void dyn_list_free(dyn_list* list);
// adds an object to the end of the list
void dyn_list_add(dyn_list* list, void* data);
// inserts an object at the specified index
void dyn_list_insert(dyn_list* list, size_t index, void* data);
// removes an object at the specified index
void dyn_list_remove(dyn_list* list, size_t index);
// removes all objects from the list
void dyn_list_clear(dyn_list* list);
// returns the object at the specified index
void* dyn_list_get(dyn_list* list, size_t index);
// sets the object at the specified index
void dyn_list_set(dyn_list* list, size_t index, void* data);

#endif