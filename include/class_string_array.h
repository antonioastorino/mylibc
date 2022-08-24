#ifndef CLASS_STRING_ARRAY_H
#define CLASS_STRING_ARRAY_H
#include "common.h"
#define StringArray_new(str, separator) _StringArray_new(str, separator, __FILE__, __LINE__)
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    typedef struct
    {
        size_t num_of_elements;
        char* str_char_p;
        char** str_array_char_p;
    } StringArray;

    StringArray StringArray_empty();
    StringArray _StringArray_new(const char*, const char*, const char* file, const int line);
    void StringArray_destroy(StringArray*);

#if TEST == 1
    void test_class_string_array();
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
