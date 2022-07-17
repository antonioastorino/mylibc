#ifndef MY_STRING_H
#define MY_STRING_H
#include "common.h"
#include <stdbool.h>
#include <sys/types.h>

typedef struct String
{
    // Length that would be returned by `strlen(str)`.
    size_t length;
    // Allocated memory in number of chars.
    size_t size;
    // Array of chars whose allocated length >= `len`.
    char* str;
} String;

/************************************* (De)Constructors ***************************************/
String String_new(const char*, ...);
String String_join(const char**, const char*);
String String_clone(const String*);
void String_destroy(String*);

/***************************************** Checkers *******************************************/
bool String_is_null(const String*);
bool String_starts_with(const String*, const char*);
Error String_between_patterns_in_string_p(String*, const char*, const char*, String*);
Error String_between_patterns_in_char_p(const char*, const char*, const char*, String*);
bool String_match(const String*, const String*);

/**************************************** Modifiers *******************************************/
Error String_replace_char(String*, const char, const char, size_t*);
Error String_replace_pattern(String*, const char*, const char*, size_t*);
Error String_replace_pattern_size_t(String*, const char*, const char*, const size_t, size_t*);
Error String_replace_pattern_float(String*, const char*, const char*, const float, size_t*);
Error String_replace_pattern_int(String*, const char*, const char*, const int, size_t*);

// clang-format off
#define String_between_patterns(in_value, prefix, suffix, out_string) \
    _Generic((in_value), \
     String*     : String_between_patterns_in_string_p, \
     const char* : String_between_patterns_in_char_p \
     )(in_value, prefix, suffix, out_string)

#define String_replace_pattern_with_format(haystack, needle, format, replacement, out_count) \
    _Generic((replacement), \
    size_t: String_replace_pattern_size_t, \
    float: String_replace_pattern_float, \
    int: String_replace_pattern_int \
    )(haystack, needle, format, replacement, out_count)
// clang-format on

#if TEST == 1
void test_class_string(void);
#endif
#endif
