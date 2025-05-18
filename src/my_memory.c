#include "my_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>

#if MEMORY_CHECK == 1
#define create_file(pointer, file, line)                                                           \
    {                                                                                              \
        char file_name[256] = {0};                                                                 \
        snprintf(file_name, 255, "/tmp/pointers/%p", pointer);                                     \
        FILE* fh = fopen(file_name, "wx");                                                         \
        if (!fh)                                                                                   \
        {                                                                                          \
            LOG_PERROR("Cannot create %p file", pointer);                                          \
            exit(errno);                                                                           \
        }                                                                                          \
        fprintf(fh, "%s:%d", file, line);                                                          \
        fclose(fh);                                                                                \
    }

#define remove_file(pointer)                                                                       \
    {                                                                                              \
        char file_name[256] = {0};                                                                 \
        snprintf(file_name, 255, "/tmp/pointers/%p", pointer);                                     \
        if (remove(file_name))                                                                     \
        {                                                                                          \
            LOG_PERROR("Cannot remove %p file.", pointer);                                         \
            exit(errno);                                                                           \
        }                                                                                          \
    }

#else /* MEMORY_CHECK == 1 */

#define create_file(pointer, file, line)                                                           \
    {                                                                                              \
        UNUSED(pointer);                                                                           \
        UNUSED(file);                                                                              \
        UNUSED(line);                                                                              \
    }

#define remove_file(pointer)                                                                       \
    {                                                                                              \
        UNUSED(pointer);                                                                           \
    }

#endif /* MEMORY_CHECK == 1 */

void* my_memory_malloc(const char* file, const int line, size_t size)
{
    void* ptr = malloc(size);
    create_file(ptr, file, line);
    return ptr;
}

void* my_memory_realloc(const char* file, const int line, void* ptr, size_t size)
{
    void* new_ptr = realloc(ptr, size);
    if (new_ptr != ptr)
    {
        // Realloc, when creating a new pointer, frees the old one.
        create_file(new_ptr, file, line);
        remove_file(ptr);
        ptr = NULL;
    }
    return new_ptr;
}

int my_memory_vasprintf(
    const char* file,
    const int line,
    char** ptr_p,
    const char* format,
    va_list args)
{
    int ret_val = vasprintf(ptr_p, format, args);
    create_file((void*)*ptr_p, file, line);
    return ret_val;
}

int my_memory_asprintf(const char* file, const int line, char** ptr_p, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    return my_memory_vasprintf(file, line, ptr_p, format, args);
}

void my_memory_free(void* ptr)
{
    remove_file(ptr);
    free(ptr);
}

#if TEST == 1
void test_my_memory(void)
{
    PRINT_BANNER();
    PRINT_TEST_TITLE("my_memory_asprintf");
    {
        char* char_p;
        ASSERT_EQ(
            my_memory_asprintf(__FILE__, __LINE__, &char_p, "%s %d", "hello", 1), 7, "It worked");
        ASSERT_EQ(char_p, "hello 1", "Correct string");
        my_memory_free(char_p);
    }
}
#endif /* TEST == 1 */
