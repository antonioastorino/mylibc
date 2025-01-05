#ifndef CLASS_STRING_ARRAY_H
#define CLASS_STRING_ARRAY_H
#include "common.h"
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

    StringArray StringArray_empty(void);
    StringArray _StringArray_new(const char* file, const int line, const char*, const char*);
    void StringArray_destroy(StringArray*);

#define StringArray_new(str, separator) _StringArray_new(__FILE__, __LINE__, str, separator)
#if TEST == 1
    void test_class_string_array(void);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
