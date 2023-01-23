
/**
 * @file table.c
 * @author Devin Arena
 * @brief Implementation file for hash table.
 * @since 5/26/2022
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

// max load before table resizes
#define TABLE_MAX_LOAD 0.75

/**
 * @brief Initializes a table by zeroing out all of its memory.
 *
 * @param table Table* the table to initialize.
 */
void hash_table_init(HashTable* table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

/**
 * @brief Frees a table by freeing its entries and zeroing its memory.
 *
 * @param table Table* the table to free.
 */
void hash_table_free(HashTable* table) {
  free(table->entries);
  hash_table_init(table);
}

/**
 * @brief Takes the hash of the key and searches the array for it, if it doesnt
 * find it, it linear searches from that position.
 *
 * @param entries Entry* the array of entries to search.
 * @param capacity int the capacity of the array.
 * @param key PdString* the key to search for.
 * @return Entry* the entry that was found.
 */
static Entry* findEntry(Entry* entries, int capacity, const char* key) {
  uint32_t index = (HASH_STRING(key) & (capacity - 1));
  Entry* tombstone = NULL;

  while (true) {
    Entry* entry = &entries[index];

    if (entry->key == NULL) {
      if (entry->value == NULL) {
        return tombstone != NULL ? tombstone : entry;
      } else {
        if (tombstone == NULL)
          tombstone = entry;
      }
    } else if (strcmp(entry->key, key) == 0) {
      return entry;
    }

    index = (index + 1) & (capacity - 1);
  }

  return NULL;
}

/**
 * @brief Adjusts the capacity of the table by creating a new entries array and
 * copying the old entries into it.
 *
 * @param table Table* the table to adjust.
 * @param capacity int the new capacity of the table.
 */
static void adjustCapacity(HashTable* table, int capacity) {
  Entry* entries = malloc(sizeof(Entry) * capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NULL;
  }

  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (entry->key == NULL)
      continue;

    Entry* dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  free(table->entries);

  table->entries = entries;
  table->capacity = capacity;
}

/**
 * @brief Gets a value from the table by searching for the key. Assigns the
 * value to the value pointer. If the key is not found, it returns false.
 *
 * @param table Table* the table to search.
 * @param key const char* the key to search for.
 * @return Value* a pointer to the value or NULL if the key is not found.
 */
Value* hash_table_get(HashTable* table, const char* key) {
  if (table->count == 0)
    return NULL;

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL)
    return NULL;

  return entry->value;
}

/**
 * @brief Gets a value from the table by searching for the key. Assigns the
 * value to the value pointer. If the key is not found, it returns false.
 *
 * @param table Table* the table to search.
 * @param key_start const char* the key to search for.
 * @param length size_t the length of the key
 * @return Value* a pointer to the value or NULL if the key is not found.
*/
Value* hash_table_get_n(HashTable* table, const char* key_start, size_t length) {
  char buffer[200];
  if (length > 200)
    return NULL;

  memcpy(buffer, key_start, length);
  buffer[length] = '\0';
  return hash_table_get(table, buffer);
}

/**
 * @brief Sets a value in the table, incrementing the count if the key is not in
 * the table. Adjusts the capacity if the load factor is greater than the max
 * load.
 *
 * @param table Table* the table to set the value in.
 * @param key PdString* the key to set the value for.
 * @param value Value the value to set.
 * @return bool true if the key is not found, false otherwise.
 */
bool hash_table_set(HashTable* table, const char* key, Value* value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = table->capacity < 8 ? 8 : table->capacity * 2;
    adjustCapacity(table, capacity);
  }

  Entry* entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = entry->key == NULL;
  if (isNewKey && entry->value == NULL)
    table->count++;

  if (isNewKey) {
    entry->key = strdup(key);
  }

  if (entry->value != NULL)
    value_free(entry->value);
  entry->value = value_clone(value);

  return isNewKey;
}

/**
 * @brief Deletes an entry from the table by setting it to be a tombstone.
 *
 * @param table Table* the table to delete the entry from.
 * @param key PdString* the key to delete.
 * @return bool true if the key was found, false otherwise.
 */
bool hash_table_delete(HashTable* table, const char* key) {
  if (table->count == 0)
    return false;

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL)
    return false;

  if (entry->value != NULL)
    value_free(entry->value);

  Value* copy = value_clone(&value_new_null());
  copy->data.integer_32 = -1;
  entry->value = copy;  // tombstone
  return true;
}

/**
 * @brief Copies all entires from one table to another.
 *
 * @param from Table* the table to copy from.
 * @param to Table* the table to copy to.
 */
void hash_table_add_all(HashTable* from, HashTable* to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry* entry = &from->entries[i];
    if (entry->key == NULL)
      continue;

    hash_table_set(to, entry->key, entry->value);
  }
}

/**
 * @brief Searches the table for a string with the specified hash, characters,
 * and length.
 *
 * @param table Table* the table to search.
 * @param chars char* the characters to search for.
 * @param length int the length of the string.
 * @param hash uint32_t the hash of the string.
 * @return PdString* the string that was found or NULL.
 */
char* hash_table_find_string(HashTable* table,
                                   const char* chars,
                                   size_t length,
                                   uint32_t hash) {
  if (table->count == 0)
    return NULL;

  uint32_t index = hash & (table->capacity - 1);

  while (true) {
    Entry* entry = &table->entries[index];

    if (entry->key == NULL) {
      if (entry->value == NULL)
        return NULL;
    } else if (strlen(entry->key) == length &&
               HASH_STRING(entry->key) == hash &&
               memcmp(entry->key, chars, length) == 0) {
      return entry->key;
    }

    index = (index + 1) & (table->capacity - 1);
  }
}

/**
 * @brief Prints a hash table.
 *
 * @param table Table* the table to print.
 */
void hash_table_print(HashTable* table) {
  printf("table: %p\n", table);
  printf(" count: %d\n", table->count);
  printf(" capacity: %d\n", table->capacity);
  printf(" entries: %p\n", table->entries);
  printf(" entries: [\n");
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (entry->key) {
      printf("  {\n");
      printf("  key (%p): %s\n", entry->key, entry->key);
      printf("  value (%p): ", entry->value);
      if (entry->value)
        value_print(entry->value);
      else
        printf("%p", entry->value);
      printf("\n  }\n");
    }
  }
  printf("]\n");
}