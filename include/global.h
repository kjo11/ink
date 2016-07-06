#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdlib.h>

#define MAX_PATHNAME_LENGTH 1024

#define DO_OR_DIE(x)                              \
  do {                                            \
    if ((x) != EXIT_SUCCESS) return EXIT_FAILURE; \
  } while (0);

#define NNULL_OR_DIE(x)                   \
  do {                                    \
    if ((x) == NULL) return EXIT_FAILURE; \
  } while (0);

#define FREE_NOT_NULL(x)      \
  do {                        \
    if ((x) != NULL) free(x); \
  } while (0);


#endif
