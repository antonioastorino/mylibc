#include "common.h"
#include "my_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>

#ifdef MEMORY_CHECK
#define create_file(pointer, file, line)                                                           \
    {                                                                                              \
        char file_name[256];                                                                       \
        sprintf(file_name, "/tmp/pointers/%p", pointer);                                           \
        FILE* fh = fopen(file_name, "wx");                                                         \
        if (!fh)                                                                                   \
        {                                                                                          \
            LOG_ERROR("Cannot create %p file", pointer);                                           \
            exit(errno);                                                                           \
        }                                                                                          \
        fprintf(fh, "%s:%d", file, line);                                                          \
        fclose(fh);                                                                                \
    }

#define remove_file(pointer, file, line)                                                           \
    {                                                                                              \
        char file_name[256];                                                                       \
        sprintf(file_name, "/tmp/pointers/%p", pointer);                                           \
        if (remove(file_name))                                                                     \
        {                                                                                          \
            fprintf(stderr, "Cannot remove %p file called by %s:%d.", pointer, file, line);        \
            exit(errno);                                                                           \
        }                                                                                          \
    }

#else /* MEMORY_CHECK */

#define create_file(pointer, file, line)                                                           \
    {                                                                                              \
        (void)(pointer);                                                                           \
        (void)(file);                                                                              \
        (void)(line);                                                                              \
    }

#define remove_file(pointer, file, line)                                                           \
    {                                                                                              \
        (void)(pointer);                                                                           \
        (void)(file);                                                                              \
        (void)(line);                                                                              \
    }

#endif /* MEMORY_CHECK */

void* custom_malloc(size_t size, const char* file, const int line)
{
    void* ptr = malloc(size);
    create_file(ptr, file, line);
    return ptr;
}

void* custom_realloc(void* ptr, size_t size, const char* file, const int line)
{
    void* new_ptr = realloc(ptr, size);
    if (new_ptr != ptr)
    {
        // Realloc, when creating a new pointer, frees the old one.
        create_file(new_ptr, file, line);
        remove_file(ptr, file, line);
        ptr = NULL;
    }
    return new_ptr;
}

int custom_vasprintf(
    char** ptr_p,
    const char* format,
    va_list args,
    const char* file,
    const int line)
{
    int ret_val = vasprintf(ptr_p, format, args);
    create_file(*ptr_p, file, line);
    return ret_val;
}

int custom_asprintf(const char* file, const int line, char** ptr_p, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    return custom_vasprintf(ptr_p, format, args, file, line);
}

void custom_free(void* ptr, const char* file, const int line)
{
    remove_file(ptr, file, line);
    free(ptr);
}

#if TEST == 1
void test_my_memory()
{
    PRINT_BANNER();
    PRINT_TEST_TITLE("custom_asprintf");
    {
        char* char_p;
        ASSERT_EQ(ASPRINTF(&char_p, "%s %d", "hello", 1), 7, "It worked");
        ASSERT_EQ(char_p, "hello 1", "Correct string");
        FREE(char_p);
    }
}
#endif /* TEST == 1 */
