#ifndef CLASS_STRING_ARRAY_H
#define CLASS_STRING_ARRAY_H
#include "common.h"

typedef struct
{
    size_t num_of_elements;
    char* str_char_p;
    char** str_array_char_p;
} StringArray;
StringArray StringArray_new(const char*, const char*);
void StringArray_destroy(StringArray*);

#if TEST == 1
void test_class_string_array();
#endif

#endif
