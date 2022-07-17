#include "class_string.h"
#include "my_memory.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_FACTOR 1.5

bool String_is_null(const String* string_obj_p)
{
    if (string_obj_p == NULL)
        return true;
    if (string_obj_p->str == NULL)
        return true;
    return false;
}

String String_new(const char* format, ...)
{
    va_list args;
    char* tmp_str_p = NULL;
    String out_string_obj;
    va_start(args, format);
    // Calculate how many bytes are needed (excluding the terminating '\0').
    if (VASPRINTF(&tmp_str_p, format, args) == -1)
    {
        LOG_ERROR("Out of memory", errno)
        exit(ERR_NULL);
    }
    size_t actual_size = strlen(tmp_str_p);
    // Allocate twice the required length
    size_t allocated_size = (size_t)(actual_size * SIZE_FACTOR);
    // printf("Allocated size: %zu\n", allocated_size);
    tmp_str_p = (char*)REALLOC(tmp_str_p, sizeof(char) * allocated_size);
    LOG_TRACE("Created string.")
    va_end(args);
    // Set the `.len` parameter as the length of the string, excluding the terminating '\0'.
    out_string_obj.str    = tmp_str_p;
    out_string_obj.length = actual_size;
    out_string_obj.size   = allocated_size;
    return out_string_obj;
}

// The array must end with a NULL value.
String String_join(const char** char_array, const char* joint)
{
    String out_string_obj = {.length = -1, .size = -1, .str = NULL};
    if (!char_array || !char_array[0])
    {
        LOG_ERROR("Please provide a valid input array.");
        return out_string_obj;
    }
    const char** curr_element = char_array;
    out_string_obj            = String_new(*curr_element);
    while (*(curr_element + 1) != NULL)
    {
        // TODO: Use String_renew() instead.
        String new_ret_string = String_new("%s%s%s", out_string_obj.str, joint, *(++curr_element));
        String_destroy(&out_string_obj);
        out_string_obj = new_ret_string;
    }
    return out_string_obj;
}

String String_clone(const String* origin) { return String_new(origin->str); }

void String_destroy(String* string_obj_p)
{
    FREE(string_obj_p->str);
    string_obj_p->str    = 0;
    string_obj_p->length = -1;
    string_obj_p->size   = -1;
}

Error _String_print(const String* string_obj_p)
{
    if (String_is_null(string_obj_p))
    {
        LOG_ERROR("Uninitialized string.");
        return ERR_NULL;
    }
    for (size_t i = 0; i < string_obj_p->length; i++)
    {
        printf("%c", string_obj_p->str[i]);
    }
    fflush(stdout);
    return ERR_ALL_GOOD;
}

Error _String_println(const String* string_obj_p)
{
    Error ret_res = _String_print(string_obj_p);
    return_on_err(ret_res);
    printf("\n");
    return ret_res;
}

bool String_starts_with(const String* string_p, const char* prefix)
{
    if (String_is_null(string_p))
    {
        LOG_ERROR("Trying to check the start of an empty string");
        return false;
    }
    if (strstr(string_p->str, prefix) == string_p->str)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool String_match(const String* a_string_p, const String* b_string_p)
{
    if (a_string_p == NULL || b_string_p == NULL)
    {
        LOG_WARNING("Input strings should not be NULL");
        return false;
    }
    if (strcmp(a_string_p->str, b_string_p->str))
    {
        return false;
    }
    return true;
}

Error String_replace_char(
    String* haystack_string_p,
    const char needle,
    const char replace,
    size_t* out_count)
{
    return_on_err(String_is_null(haystack_string_p)) char tmp_str[haystack_string_p->length + 1];
    size_t i = 0, j = 0, cnt = 0;
    while (i < haystack_string_p->length)
    {
        if (haystack_string_p->str[i] == needle)
        {
            cnt++;
            if (replace != '\0')
            {
                // Replace the current char with that provided.
                tmp_str[j++] = replace;
            }
        }
        else
        {
            tmp_str[j++] = haystack_string_p->str[i];
        }
        i++;
    }
    // Terminate.
    tmp_str[j] = '\0';
    // Update the string length in case some chars were removed.
    haystack_string_p->length = j;
    strncpy(haystack_string_p->str, tmp_str, j + 1);
    *out_count = cnt;
    return ERR_ALL_GOOD;
}

#define String_replace_pattern_c(suffix)                                                           \
    Error String_replace_pattern_##suffix(                                                         \
        String* haystack_string_p,                                                                 \
        const char* needle,                                                                        \
        const char* format,                                                                        \
        const suffix replacement,                                                                  \
        size_t* out_count)                                                                         \
    {                                                                                              \
        String replacement_string = String_new(format, replacement);                               \
        Error res_replace         = String_replace_pattern(                                        \
            haystack_string_p, needle, replacement_string.str, out_count);                 \
        String_destroy(&replacement_string);                                                       \
        return res_replace;                                                                        \
    }

String_replace_pattern_c(size_t);
String_replace_pattern_c(float);
String_replace_pattern_c(int);

Error String_replace_pattern(
    String* haystack_string_p,
    const char* needle,
    const char* replacement,
    size_t* out_count)
{
    if (String_is_null(haystack_string_p))
    {
        LOG_ERROR("Uninitialized string.");
        return ERR_NULL;
    }
    if (strlen(needle) == 0)
    {
        LOG_ERROR("Empty needle not allowed.");
        return ERR_EMPTY_STRING;
    }
    int oldWlen = strlen(needle);
    int newWlen = strlen(replacement);
    size_t i;
    size_t cnt = 0;

    // Counting the number of times old word occur in the string
    const char* s = haystack_string_p->str;
    for (i = 0; i < haystack_string_p->length; i++)
    {
        if (strstr(&s[i], needle) == &s[i])
        {
            cnt++;

            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
    }
    if (cnt == 0)
    {
        LOG_DEBUG("Pattern %s not found", needle);
        *out_count = 0;
        return ERR_ALL_GOOD;
    }

    // Making new string of enough length
    size_t new_string_length         = i + cnt * (newWlen - oldWlen);
    char* result_char_p              = (char*)MALLOC(new_string_length + 1);
    result_char_p[new_string_length] = 0;

    i = 0;
    while (*s)
    {
        // compare the substring with the result
        if (strstr(s, needle) == s)
        {
            strcpy(&result_char_p[i], replacement);
            i += newWlen;
            s += oldWlen;
        }
        else
            result_char_p[i++] = *s++;
    }

    // Update string.
    if (new_string_length >= haystack_string_p->size)
    {
        // Increase the allocated size.
        haystack_string_p->size = (size_t)(new_string_length * SIZE_FACTOR);
        haystack_string_p->str  = (char*)REALLOC(haystack_string_p->str, haystack_string_p->size);
    }
    haystack_string_p->length = new_string_length;
    // Copy an extra byte for the NULL characther.
    strncpy(haystack_string_p->str, result_char_p, new_string_length + 1);
    haystack_string_p->str[new_string_length] = 0;
    FREE(result_char_p);
    result_char_p = NULL;

    *out_count = cnt;
    return ERR_ALL_GOOD;
}

Error String_between_patterns_in_char_p(
    const char* in_char_p,
    const char* prefix,
    const char* suffix,
    String* out_string_obj_p)
{
    if (in_char_p == NULL)
    {
        LOG_ERROR("Uninitialized input detected.");
        return ERR_NULL;
    }
    else if (strlen(in_char_p) < strlen(prefix) + strlen(suffix))
    {
        LOG_DEBUG("Input str length : %lu", strlen(in_char_p));
        LOG_DEBUG("Prefix length    : %lu", strlen(in_char_p));
        LOG_DEBUG("Suffix length    : %lu", strlen(in_char_p));

        LOG_ERROR("Input shorter than the input patterns.");
        return ERR_INVALID;
    }
    char* start = strstr(in_char_p, prefix);
    if (start == NULL)
    {
        LOG_ERROR("Prefix not found in input string");
        return ERR_NOT_FOUND;
    }
    start     = start + strlen(prefix);
    char* end = strstr(start, suffix);
    if (end == NULL)
    {
        LOG_ERROR("Suffix not found in input string");
        return ERR_NOT_FOUND;
    }
    char* tmp = (char*)MALLOC(end - start + 1);
    memcpy(tmp, start, end - start);
    tmp[end - start] = '\0';

    (*out_string_obj_p) = String_new(tmp);
    FREE(tmp);
    tmp = NULL;
    return ERR_ALL_GOOD;
}

Error String_between_patterns_in_string_p(
    String* in_string_p,
    const char* prefix,
    const char* suffix,
    String* out_string_obj_p)
{
    return String_between_patterns_in_char_p(in_string_p->str, prefix, suffix, out_string_obj_p);
}

#if TEST == 1
void test_class_string()
{
    PRINT_BANNER();

    PRINT_TEST_TITLE("New from string")
    {
        const char* str    = "Hello everybody";
        String test_string = String_new(str);
        ASSERT_EQ(test_string.length, strlen(str), "Length correct.");
        ASSERT_EQ(test_string.size, (size_t)(strlen(str) * 1.5), "Size correct.");
        ASSERT_EQ((int)_String_println(&test_string), ERR_ALL_GOOD, "Printing functions work.");
        String_destroy(&test_string);
        ASSERT_EQ(String_is_null(&test_string), true, "Already destroyed.");
    }
    PRINT_TEST_TITLE("New from formatter");
    {
        const char* format1   = "Old string content.";
        String test_string    = String_new(format1);
        size_t initial_length = strlen(test_string.str);
        size_t initial_size   = (size_t)(initial_length * 1.5);
        ASSERT_EQ(test_string.length, initial_length, "Length correct.");
        ASSERT_EQ(test_string.size, initial_size, "Size correct.");
        String_destroy(&test_string);
    }
    PRINT_TEST_TITLE("starts_with() function");
    {
        String test_string = String_new("Old string content.");
        ASSERT_EQ(
            String_starts_with(&test_string, "Old string"), true, "starts_with() works when true");
        ASSERT_EQ(String_starts_with(&test_string, "new"), false, "starts_with() works when false");
        ASSERT_EQ(
            String_starts_with(&test_string, ""), true, "starts_with() works when needle is empty");
        String_destroy(&test_string);
    }

    PRINT_TEST_TITLE("clone() function")
    {
        String test_origin = String_new("Original");
        String test_clone  = String_clone(&test_origin);
        size_t length      = test_origin.length;
        size_t size        = test_origin.size;
        ASSERT_EQ(test_origin.str, test_clone.str, "Strings match.");
        String_destroy(&test_origin); // The clone should still be alive.
        ASSERT_EQ(length, test_clone.length, "Size copied");
        ASSERT_EQ(size, test_clone.size, "Size copied");
        String_destroy(&test_clone);
    }
    PRINT_TEST_TITLE("replace_char() function - replace with nothing.");
    {
        String test_string = String_new("Some text to be modified.");
        size_t num_of_replacements;
        _String_println(&test_string);
        ASSERT(
            String_replace_char(&test_string, ' ', '\0', &num_of_replacements) == ERR_ALL_GOOD,
            "Replacement successful.");
        ASSERT_EQ(test_string.str, "Sometexttobemodified.", "Needles removed correctly.");
        ASSERT_EQ(num_of_replacements, 4, "Replacements counted correctly.");
        _String_println(&test_string);

        ASSERT(
            String_replace_char(&test_string, 't', '_', &num_of_replacements) == ERR_ALL_GOOD,
            "Replacement successful.");
        ASSERT_EQ(test_string.str, "Some_ex__obemodified.", "Needles replaced correctly.");
        _String_println(&test_string);

        ASSERT(
            String_replace_char(&test_string, '&', '^', &num_of_replacements) == ERR_ALL_GOOD,
            "Replacement successful");
        ASSERT_EQ(test_string.str, "Some_ex__obemodified.", "String unchanged - needle not found.");
        ASSERT_EQ(num_of_replacements, 0, "No replacements counted correctly.");
        _String_println(&test_string);

        String_replace_char(&test_string, 'S', '+', &num_of_replacements);
        String_replace_char(&test_string, 'o', '+', &num_of_replacements);
        String_replace_char(&test_string, 'm', '+', &num_of_replacements);
        String_replace_char(&test_string, '_', '+', &num_of_replacements);
        String_replace_char(&test_string, 'e', '+', &num_of_replacements);
        String_replace_char(&test_string, 'x', '+', &num_of_replacements);
        String_replace_char(&test_string, 'b', '+', &num_of_replacements);
        String_replace_char(&test_string, 'd', '+', &num_of_replacements);
        String_replace_char(&test_string, 'i', '+', &num_of_replacements);
        String_replace_char(&test_string, 'f', '+', &num_of_replacements);
        String_replace_char(&test_string, '.', '+', &num_of_replacements);
        ASSERT_EQ(
            test_string.str, "+++++++++++++++++++++", "Replaced all chars in string with '+'.");
        _String_println(&test_string);

        size_t initial_length = test_string.length;
        String_replace_char(&test_string, '+', '\0', &num_of_replacements);
        ASSERT_EQ(test_string.str, "", "Ths string is correctly emptied.");
        ASSERT_EQ(test_string.length, 0, "Ths length is correctly set to 0.");
        ASSERT_EQ(num_of_replacements, initial_length, "Number of replacements counted correctly.");
        _String_println(&test_string);
        String_destroy(&test_string);
    }

    PRINT_TEST_TITLE("Test String_between_patterns()");
    {
        const char* input_char_p = "This string contains a \":pattern:\" to be found";

        {
            String pattern_string;
            ASSERT(
                String_between_patterns(input_char_p, "\":", ":\"", &pattern_string)
                    == ERR_ALL_GOOD,
                "String between pattern success.");
            ASSERT_EQ(pattern_string.str, "pattern", "Pattern found in C string");
            String_destroy(&pattern_string);
        }
        {
            String pattern_string;
            String test_string = String_new(input_char_p);
            String_between_patterns(&test_string, "\":", ":\"", &pattern_string);
            ASSERT_EQ(pattern_string.str, "pattern", "Pattern found String Object");
            String_destroy(&pattern_string);
            String_destroy(&test_string);
        }
        {
            String pattern_string;
            String test_string = String_new(input_char_p);
            Error res_pattern_not_found
                = String_between_patterns(&test_string, "--", ":\"", &pattern_string);
            ASSERT(res_pattern_not_found == ERR_NOT_FOUND, "Prefix not found");

            res_pattern_not_found
                = String_between_patterns(&test_string, "\":", "--", &pattern_string);
            ASSERT(res_pattern_not_found == ERR_NOT_FOUND, "Suffix not found");

            String_destroy(&test_string);
        }
    }
    PRINT_TEST_TITLE("Pattern replacement with empty string");
    {
        size_t num_of_replacements;
        String test_string = String_new("This pattern contains \\r\\n to be removed");
        String_replace_pattern(&test_string, "\\r\\n ", "", &num_of_replacements);
        ASSERT_EQ("This pattern contains to be removed", test_string.str, "Pattern deleted");
        ASSERT_EQ(num_of_replacements, 1, "Number of replacements counted correctly.");
        String_destroy(&test_string);
    }
    PRINT_TEST_TITLE("Pattern replacement with short string");
    {
        size_t num_of_replacements;
        String test_string = String_new("This pattern contains \\r\\n to be replaced");
        String_replace_pattern(&test_string, "\\r\\n", "HELLO WORLD", &num_of_replacements);
        ASSERT_EQ(
            "This pattern contains HELLO WORLD to be replaced",
            test_string.str,
            "Pattern replaced");
        ASSERT_EQ(num_of_replacements, 1, "Number of replacements counted correctly.");
        String_destroy(&test_string);
    }
    PRINT_TEST_TITLE("Pattern replacement with float");
    {
        size_t num_of_replacements;
        String test_string = String_new("This pattern contains \\r\\n to be replaced");
        String_replace_pattern_with_format(
            &test_string, "\\r\\n", "%.4f", 10.3f, &num_of_replacements);
        ASSERT_EQ(
            "This pattern contains 10.3000 to be replaced", test_string.str, "Pattern replaced");
        ASSERT_EQ(num_of_replacements, 1, "Number of replacements counted correctly.");
        String_destroy(&test_string);
    }
    PRINT_TEST_TITLE("Pattern replacement with size_t");
    {
        size_t num_of_replacements;
        String test_string = String_new("This pattern contains \\r\\n to be replaced");
        String_replace_pattern_with_format(
            &test_string, "\\r\\n", "%lu", (size_t)1003, &num_of_replacements);
        ASSERT_EQ("This pattern contains 1003 to be replaced", test_string.str, "Pattern replaced");
        ASSERT_EQ(num_of_replacements, 1, "Number of replacements counted correctly.");
        String_destroy(&test_string);
    }

    PRINT_TEST_TITLE("Pattern replacement with int");
    {
        size_t num_of_replacements;
        String test_string = String_new("This pattern contains \\r\\n to be replaced");
        String_replace_pattern_with_format(
            &test_string, "\\r\\n", "%d", -1003, &num_of_replacements);
        ASSERT_EQ(
            "This pattern contains -1003 to be replaced", test_string.str, "Pattern replaced");
        ASSERT_EQ(num_of_replacements, 1, "Number of replacements counted correctly.");
        String_destroy(&test_string);
    }
    PRINT_TEST_TITLE("Pattern replacement with long string");
    {
        size_t num_of_replacements;
        String test_string = String_new("This pattern contains \\r\\n to be replaced");
        String_replace_pattern(
            &test_string,
            "\\r\\n",
            "HELLO WORLD! This is the replacement pattern",
            &num_of_replacements);
        ASSERT_EQ(
            "This pattern contains HELLO WORLD! This is the replacement pattern to be replaced",
            test_string.str,
            "Pattern replaced");
        ASSERT_EQ(num_of_replacements, 1, "Number of replacements counted correctly.");
        String_destroy(&test_string);
    }
    PRINT_TEST_TITLE("Pattern not replaced because missing");
    {
        size_t num_of_replacements;
        String test_string  = String_new("This string does not contain a pattern to be replaced");
        num_of_replacements = String_replace_pattern(
            &test_string,
            "missing pattern",
            "HELLO WORLD! This is the replacement pattern",
            &num_of_replacements);
        ASSERT_EQ(
            "This string does not contain a pattern to be replaced",
            test_string.str,
            "Pattern not found");
        ASSERT_EQ(num_of_replacements, 0, "Number of replacements counted correctly.");
        String_destroy(&test_string);
    }
    PRINT_TEST_TITLE("Trying to join an empty array");
    {
        const char* char_array[] = {NULL};
        String test_string       = String_join(char_array, "hello");
        ASSERT(test_string.str == NULL, "No string created");
    }

    PRINT_TEST_TITLE("Join with 1 element");
    {
        const char* one_element_array[] = {"element 1", NULL};
        String test_string              = String_join(one_element_array, "hello");
        ASSERT_EQ(test_string.str, "element 1", "No concatenation performed");
        String_destroy(&test_string);
    }

    PRINT_TEST_TITLE("Join with 2 elements");
    {
        const char* two_element_array[] = {"element 1", "element 2", NULL};
        String test_string              = String_join(two_element_array, "|||");
        ASSERT_EQ(test_string.str, "element 1|||element 2", "Concatenation correct");
        String_destroy(&test_string);
    }
    PRINT_TEST_TITLE("Join with empty separator");
    {
        const char* foo_bar_element_array[] = {"F", "O", "O", "B", "A", "R", NULL};
        String test_string                  = String_join(foo_bar_element_array, "");
        ASSERT_EQ(test_string.str, "FOOBAR", "Concatenation correct");
        String_destroy(&test_string);
    }
    /**/
}
#endif
