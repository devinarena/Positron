
/**
 * @file memory.c
 * @author Devin Arena
 * @brief Handles memory management for the language.
 * @since 1/6/2023
 **/

#include <stdio.h>
#include <stdlib.h>

#include "memory.h"

/**
 * @brief Reads a file into a newly allocated string.
 *
 * @param path the path to the file
 * @return const char* the file contents
 */
const char* read_file(const char* path) {
  FILE* file = fopen(path, "rb");
  // file may not exist or be readable
  if (!file) {
    printf("Could not open file '%s'\n", path);
    exit(74);
  }

  // get file size and go back to beginning
  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  // allocate memory for file contents
  char* buffer = (char*)malloc(sizeof(char) * (fileSize + 1));
  // may not have enough memory to read whole file
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read '%s'\n", path);
    exit(74);
  }

  // read file contents into buffer
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

  // if bytes read is not equal to file size, something went wrong
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file '%s'\n", path);
    exit(74);
  }

  // null terminate buffer
  buffer[bytesRead] = '\0';

  // close file
  fclose(file);

  return buffer;
}