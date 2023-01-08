
/**
 * @file table.h
 * @author Devin Arena
 * @brief header for hash table implementation.
 * @since 6/24/2022
 **/

#ifndef PALLADIUM_TABLE_H
#define PALLADIUM_TABLE_H

#include <stdbool.h>
#include <stdint.h>

#define HASH_STRING(str) hashString(str, strlen(str))

typedef struct {
  char* key;
  char* value;
} Entry;

typedef struct HashTable {
  int count;
  int capacity;
  Entry* entries;
} HashTable;

void hash_table_init(HashTable* table);
char* hash_table_get(HashTable* table, char* key);
bool hash_table_set(HashTable* table, char* key, char* value);
bool hash_table_delete(HashTable* table, char* key);
void hash_table_add_all(HashTable* from, HashTable* to);
char* hash_table_find_string(HashTable* table,
                             const char* chars,
                             size_t length,
                             uint32_t hash);
void hash_table_free(HashTable* table);

static inline uint32_t hashString(const char* key, size_t length) {
  uint32_t hash = 2166136261u;
  for (size_t i = 0; i < length; i++) {
    hash ^= key[i];
    hash *= 16777619;
  }
  return hash;
}

// void tableRemoveWhite(Table* table);
// void markTable(Table* table);

#endif