#ifndef STRING_MANIPULATION_H
#define STRING_MANIPULATION_H

#include <stdlib.h>
#include <string.h>

// Allocate memory and copy string_in. Returns NULL in case of failure.
static inline char *allocate_and_copy_string(const char *string_in) {
  char *string_out = calloc(strlen(string_in) + 1, sizeof(char));
  if (string_out != NULL) strcpy(string_out, string_in);
  return string_out;
}

// Allocate memory and concatenate n_strings strings defined in strings_in to
// output
static inline char *concat_and_copy_strings(const char **strings_in,
                                            const size_t n_strings) {
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

// Allocate memory and concatenate 4 strings to output
static inline char *concat_and_copy_strings4(const char *string1,
                                             const char *string2,
                                             const char *string3,
                                             const char *string4) {
  const size_t n_strings = 4;
  const char *p_strings[] = {string1, string2, string3, string4};
  return concat_and_copy_strings(p_strings, n_strings);
}

// Allocate memory and concatenate 3 strings to output
static inline char *concat_and_copy_strings3(const char *string1,
                                             const char *string2,
                                             const char *string3) {
  const size_t n_strings = 3;
  const char *p_strings[] = {string1, string2, string3};
  return concat_and_copy_strings(p_strings, n_strings);
}

// Allocate memory and concatenate 2 strings to output
static inline char *concat_and_copy_strings2(const char *string1,
                                             const char *string2) {
  const size_t n_strings = 2;
  const char *p_strings[] = {string1, string2};
  return concat_and_copy_strings(p_strings, n_strings);
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
