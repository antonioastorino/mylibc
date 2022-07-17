#include "class_string_array.h"
#include "my_memory.h"
#include <stdlib.h>
#include <string.h>

StringArray StringArray_new(const char* input_char_p, const char* pattern_char_p)
{
    StringArray ret_string_array
        = {.num_of_elements = 0, .str_char_p = NULL, .str_array_char_p = NULL};
    int origin_size = strlen(input_char_p);
    if (origin_size == 0)
    {
        return ret_string_array;
    }

    int i;
    size_t cnt         = 0;
    int pattern_length = strlen(pattern_char_p);

    // Counting the number of times patter occurs in the string
    const char* curr_char_p = input_char_p;
    for (i = 0; i < origin_size; i++)
    {
        if (strstr(&curr_char_p[i], pattern_char_p) == &curr_char_p[i])
        {
            cnt++;

            // Jumping to index after the old word.
            i += pattern_length - 1;
        }
    }
    LOG_TRACE("Found %lu occurrences of the pattern `%s`", cnt, pattern_char_p);

    // Making new string of enough length
    size_t new_string_length         = i + cnt * (1 - pattern_length);
    char* result_char_p              = (char*)MALLOC(new_string_length + 1);
    result_char_p[new_string_length] = 0;
    /*
    pointers[0] always points to the beginning of the string. The others point to the next split.
    pointers[last] points to NULL;
    */
    char** pointers   = (char**)MALLOC(sizeof(char*) * (cnt + 2));
    pointers[0]       = result_char_p;
    pointers[cnt + 1] = NULL;

    i              = 0;
    size_t counter = 0;
    while (*curr_char_p != 0)
    {
        // compare the substring with the result
        if (strstr(curr_char_p, pattern_char_p) == curr_char_p)
        {
            // Place terminator and add a pointer to next character
            result_char_p[i]    = '\0';
            pointers[++counter] = &result_char_p[i + 1];
            i += 1;
            curr_char_p += pattern_length;
        }
        else
        {
            result_char_p[i++] = *curr_char_p++;
        }
    }

    ret_string_array.str_char_p       = result_char_p;
    ret_string_array.str_array_char_p = pointers;
    ret_string_array.num_of_elements  = cnt + 1;

    return ret_string_array;
}

void StringArray_destroy(StringArray* string_array_p)
{
    if (string_array_p == NULL || string_array_p->num_of_elements == 0)
    {
        return;
    }
    char** ptr = string_array_p->str_array_char_p;
    while (*ptr != NULL)
    {
        *ptr = NULL;
        ptr++;
    }
    FREE(string_array_p->str_char_p);
    FREE(string_array_p->str_array_char_p);
    string_array_p->str_array_char_p = NULL;
    string_array_p->str_char_p       = NULL;
}

#if TEST == 1
void test_class_string_array()
{
    StringArray test_string_array;
    PRINT_BANNER();

    PRINT_TEST_TITLE("Split using spaces");
    test_string_array = StringArray_new("Hi you", " ");
    ASSERT_EQ(test_string_array.str_array_char_p[0], "Hi", "First element correct.");
    ASSERT_EQ(test_string_array.str_array_char_p[1], "you", "Second element correct.");
    ASSERT_EQ(test_string_array.str_array_char_p[2] == NULL, 1, "Last element (null) correct.");
    ASSERT_EQ(test_string_array.num_of_elements, 2, "Number of elements correct.");
    StringArray_destroy(&test_string_array);

    PRINT_TEST_TITLE("Split without matching pattern");
    test_string_array = StringArray_new("Hi you", "?");
    ASSERT_EQ(test_string_array.str_array_char_p[0], "Hi you", "First element correct.");
    ASSERT_EQ(test_string_array.str_array_char_p[1] == NULL, 1, "Last element (null) correct.");
    ASSERT_EQ(test_string_array.num_of_elements, 1, "Number of elements correct.");
    StringArray_destroy(&test_string_array);

    PRINT_TEST_TITLE("Empty string returns empty array");
    test_string_array = StringArray_new("", "whatever");
    ASSERT_EQ(test_string_array.str_char_p == NULL, 1, "Returned string correct.");
    ASSERT_EQ(test_string_array.str_array_char_p == NULL, 1, "Last element (null) correct.");
    ASSERT_EQ(test_string_array.num_of_elements, 0, "Number of elements correct.");

    PRINT_TEST_TITLE("Split using 2-char pattern");
    test_string_array = StringArray_new("Hi, you", ", ");
    ASSERT_EQ(test_string_array.str_array_char_p[0], "Hi", "First element correct.");
    ASSERT_EQ(test_string_array.str_array_char_p[1], "you", "Second element correct.");
    ASSERT_EQ(test_string_array.str_array_char_p[2] == NULL, 1, "Last element (null) correct.");
    ASSERT_EQ(test_string_array.num_of_elements, 2, "Number of elements correct.");
    StringArray_destroy(&test_string_array);

    PRINT_TEST_TITLE("Split using spaces - first space found at the beginning");
    test_string_array = StringArray_new(" Hi you", " ");
    ASSERT_EQ(test_string_array.str_array_char_p[0], "", "First element correct.");
    ASSERT_EQ(test_string_array.str_array_char_p[1], "Hi", "First element correct.");
    ASSERT_EQ(test_string_array.str_array_char_p[2], "you", "Second element correct.");
    ASSERT_EQ(test_string_array.num_of_elements, 3, "Number of elements correct.");
    ASSERT_EQ(test_string_array.str_array_char_p[3] == NULL, 1, "Last element (null) correct.");
    StringArray_destroy(&test_string_array);

    PRINT_TEST_TITLE("Split using spaces - last space found at the end");
    test_string_array = StringArray_new("Hi you ", " ");
    ASSERT_EQ(test_string_array.str_array_char_p[0], "Hi", "First element correct.");
    ASSERT_EQ(test_string_array.str_array_char_p[1], "you", "Second element correct.");
    ASSERT_EQ(test_string_array.str_array_char_p[2], "", "Third element correct.");
    ASSERT_EQ(test_string_array.num_of_elements, 3, "Number of elements correct.");
    ASSERT_EQ(test_string_array.str_array_char_p[3] == NULL, 1, "Last element (null) correct.");
    StringArray_destroy(&test_string_array);

    PRINT_TEST_TITLE("Bug fix - split an empty string");
    test_string_array = StringArray_new("", " ");
    ASSERT_EQ(test_string_array.num_of_elements, 0, "No split performed");
    StringArray_destroy(&test_string_array);
}
#endif
