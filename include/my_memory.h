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

    void* custom_malloc(const char* file, const int line, size_t size);
    void* custom_realloc(const char* file, const int line, void* ptr, size_t size);
    int custom_vasprintf(const char* file, const int line, char**, const char*, va_list);
    int custom_asprintf(const char* file, const int line, char** ptr_p, const char* format, ...);
    void custom_free(void*);

#if TEST == 1
    void test_my_memory();
#endif /* TEST == 1 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MY_MEMORY_H */
