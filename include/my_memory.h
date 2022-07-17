#ifndef MEM_H
#define MEM_H
#include "common.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>

#if TEST == 1
void* custom_malloc(size_t, const char*, const int);
void* custom_realloc(void*, size_t, const char*, const int);
int custom_vasprintf(char**, const char*, va_list, const char*, const int);
void custom_free(void*, const char*, const int);

#define MALLOC(size) custom_malloc(size, __FILE__, __LINE__)
#define REALLOC(ptr, size) custom_realloc(ptr, size, __FILE__, __LINE__)
#define VASPRINTF(ptr_p, format, args) custom_vasprintf(ptr_p, format, args, __FILE__, __LINE__)
#define FREE(ptr) custom_free(ptr, __FILE__, __LINE__)

#else

#define MALLOC(size) malloc(size)
#define REALLOC(ptr, size) realloc(ptr, size)
#define VASPRINTF(ptr_p, format, args) vasprintf(ptr_p, format, args)
#define FREE(ptr) free(ptr)
#endif

#endif /* MEM_H */
