#ifndef MY_MEMORY_H
#define MY_MEMORY_H
#include "common.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    void* custom_malloc(size_t, const char*, const int);
    void* custom_realloc(void*, size_t, const char*, const int);
    int custom_vasprintf(char**, const char*, va_list, const char*, const int);
    int custom_asprintf(const char* file, const int line, char** ptr_p, const char* format, ...);
    void custom_free(void*, const char*, const int);

#if TEST == 1

#define MALLOC(size) custom_malloc(size, __FILE__, __LINE__)
#define REALLOC(ptr, size) custom_realloc(ptr, size, __FILE__, __LINE__)
#define VASPRINTF(ptr_p, format, args) custom_vasprintf(ptr_p, format, args, __FILE__, __LINE__)
#define ASPRINTF(ptr_p, format, ...) custom_asprintf(__FILE__, __LINE__, ptr_p, format, __VA_ARGS__)
#define FREE(ptr) custom_free(ptr, __FILE__, __LINE__)

    void test_my_memory();
#else /* TEST == 1 */

#define MALLOC(size) malloc(size)
#define REALLOC(ptr, size) realloc(ptr, size)
#define VASPRINTF(ptr_p, format, args) vasprintf(ptr_p, format, args)
#define ASPRINTF(ptr_p, format, ...) asprintf(ptr_p, format, __VA_ARGS__)
#define FREE(ptr) free(ptr)
#endif /* TEST == 1 */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MY_MEMORY_H */
