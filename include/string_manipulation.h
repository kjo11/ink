#ifndef STRING_MANIPULATION_H
#define STRING_MANIPULATION_H

#include <stdlib.h>
#include <string.h>

// Allocate a char* with length of string_in + extra space
// Copy string_in to beginning
static inline char *concat_and_copy_strings(const char **strings_in,
                                            size_t n_strings) {
  size_t n_chars = 0;
  for (size_t i = 0; i < n_strings; i++) {
    n_chars += strlen(strings_in[i]);
  }
  char *string_out = calloc(n_chars + 1, sizeof(char));
  if (string_out == NULL) return NULL;
  string_out[0] = '\0';

  for (size_t i = 0; i < n_strings; i++) {
    strncat(string_out, strings_in[i], n_chars - strlen(string_out));
  }
  return string_out;
}

// Initialize string with an initial string in plus some extra space
static inline char *initialize_string(const char *string_in,
                                      const size_t extra_space) {
  const size_t n = strlen(string_in);
  char *string_out = calloc(n + extra_space + 1, sizeof(char));
  if (string_out == NULL) return NULL;

  string_out[0] = '\0';
  strcat(string_out, string_in);
  return string_out;
}

#endif
