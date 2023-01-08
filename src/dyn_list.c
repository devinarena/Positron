
#include <stdlib.h>
#include <string.h>

#include "dyn_list.h"

/**
 * @brief Creates a new dynamic list.
 * 
 * @param free_object the function to call when an object is removed from the list
 * @return dyn_list* a pointer to the newly allocated dynamic list
 */
dyn_list* dyn_list_new(void (*free_object)(void*)) {
  dyn_list* list = malloc(sizeof(dyn_list));
  list->data = malloc(sizeof(void*) * 8);
  list->size = 0;
  list->capacity = 8;
  list->free_object = free_object;
  return list;
}

/**
 * @brief Frees the memory allocated by a dynamic list.
 * 
 * @param list the dynamic list to free
 */
void dyn_list_free(dyn_list* list) {
  dyn_list_clear(list);
  free(list->data);
  free(list);
}

/**
 * @brief Adds an object to the end of the list.
 * 
 * @param list the list to add to
 * @param data the object to add
 */
void dyn_list_add(dyn_list* list, void* data) {
  if (list->size == list->capacity) {
    list->capacity *= 2;
    list->data = realloc(list->data, sizeof(void*) * list->capacity);
  }
  list->data[list->size++] = data;
}

/**
 * @brief inserts an object at the specified index.
 * 
 * @param list the list to insert into
 * @param index the index to insert at
 * @param data the object to insert
 */
void dyn_list_insert(dyn_list* list, size_t index, void* data) {
  if (index >= list->capacity) {
    return;
  }

  if (list->size == list->capacity) {
    list->capacity *= 2;
    list->data = realloc(list->data, sizeof(void*) * list->capacity);
  }

  for (size_t i = list->size; i > index; i--) {
    list->data[i] = list->data[i - 1];
  }
  list->data[index] = data;
  list->size++;
}

/**
 * @brief Removes an object at the specified index.
 * 
 * @param list the list to remove from
 * @param index the index to remove at
 */
void dyn_list_remove(dyn_list* list, size_t index) {
  if (index >= list->size) {
    return;
  }
  if (list->free_object)
    (*list->free_object)(list->data[index]);
  for (size_t i = index; i < list->size - 1; i++) {
    list->data[i] = list->data[i + 1];
  }
  list->size--;
}

/**
 * @brief Clears all objects from the list.
 * 
 * @param list the list to clear
 */
void dyn_list_clear(dyn_list* list) {
  if (list->free_object) {
    for (size_t i = 0; i < list->size; i++) {
      (*list->free_object)(list->data[i]);
    }
  }
  list->size = 0;
}

/**
 * @brief Returns the object at the specified index.
 * 
 * @param list the list to get from
 * @param index the index to get at
 * @return void* the object at the specified index
 */
void* dyn_list_get(dyn_list* list, size_t index) {
  if (index >= list->size) {
    return NULL;
  }
  return list->data[index];
}

/**
 * @brief Sets the object at the specified index.
 * 
 * @param list the list to set in
 * @param index the index to set at
 * @param data the object to set
 */
void dyn_list_set(dyn_list* list, size_t index, void* data) {
  if (index >= list->size) {
    return;
  }

  if (list->free_object && index < list->size)
    (*list->free_object)(list->data[index]);
  list->data[index] = data;
}