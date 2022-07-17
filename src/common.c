#include "common.h"

#define PRINT_PASS_MESSAGE(message) printf("> \x1B[32mPASS\x1B[0m\t %s\n", message)

#define PRINT_FAIL_MESSAGE_(message, filename, line_number)                                        \
    fprintf(stderr, "> \x1B[31mFAIL\x1B[0m\t %s\n", message);                                      \
    fprintf(stderr, "> Err - Test failed.\n%s:%d : false assertion\n", filename, line_number)

#define PRINT_FAIL_MESSAGE_EQ(message, filename, line_number)                                      \
    fprintf(stderr, "> \x1B[31mFAIL\x1B[0m\t %s\n", message);                                      \
    fprintf(stderr, "> Err - Test failed.\n%s:%d : left != right\n", filename, line_number)

#define PRINT_FAIL_MESSAGE_NE(message, filename, line_number)                                      \
    fprintf(stderr, "> \x1B[31mFAIL\x1B[0m\t %s\n", message);                                      \
    fprintf(stderr, "> Err - Test failed.\n%s:%d : left == right\n", filename, line_number)

void ASSERT_(bool value, const char* message, const char* filename, int line_number)
{
    if (value)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_(message, filename, line_number);
        fprintf(stderr, "The value is `false`\n");
    }
}

void ASSERT_EQ_int(
    int value_1,
    int value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(stderr, "Left : `%d`\nRight: `%d`\n", value_1, value_2);
    }
}

void ASSERT_EQ_uint8(
    uint8_t value_1,
    uint8_t value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(stderr, "Left : `%u`\nRight: `%u`\n", value_1, value_2);
    }
}

void ASSERT_EQ_uint16(
    uint16_t value_1,
    uint16_t value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(stderr, "Left : `%hu`\nRight: `%hu`\n", value_1, value_2);
    }
}

void ASSERT_EQ_uint(
    size_t value_1,
    size_t value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(stderr, "Left : `%lu`\nRight: `%lu`\n", value_1, value_2);
    }
}

void ASSERT_EQ_bool(
    bool value_1,
    bool value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(
            stderr,
            "Left : `%s`\nRight: `%s`\n",
            value_1 ? "true" : "false",
            value_2 ? "true" : "false");
    }
}
void ASSERT_EQ_float(
    float value_1,
    float value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(stderr, "Left : `%f`\nRight: `%f`\n", value_1, value_2);
    }
}

void ASSERT_EQ_double(
    double value_1,
    double value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(stderr, "Left : `%lf`\nRight: `%lf`\n", value_1, value_2);
    }
}

void ASSERT_EQ_char_p(
    const char* value_1,
    const char* value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (!strcmp(value_1, value_2))
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_EQ(message, filename, line_number);
        fprintf(stderr, "Left : `%s`\nRight: `%s`\n", value_1, value_2);
    }
}

void ASSERT_NE_int(
    int value_1,
    int value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 != value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(stderr, "Left : `%d`\nRight: `%d`\n", value_1, value_2);
    }
}

void ASSERT_NE_uchar(
    uint8_t value_1,
    uint8_t value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 != value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(stderr, "Left : `%d`\nRight: `%d`\n", value_1, value_2);
    }
}

void ASSERT_NE_uint(
    size_t value_1,
    size_t value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 != value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(stderr, "Left : `%lu`\nRight: `%lu`\n", value_1, value_2);
    }
}

void ASSERT_NE_bool(
    bool value_1,
    bool value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 != value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(
            stderr,
            "Left : `%s`\nRight: `%s`\n",
            value_1 ? "true" : "false",
            value_2 ? "true" : "false");
    }
}
void ASSERT_NE_float(
    float value_1,
    float value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(stderr, "Left : `%f`\nRight: `%f`\n", value_1, value_2);
    }
}

void ASSERT_NE_double(
    double value_1,
    double value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (value_1 == value_2)
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(stderr, "Left : `%lf`\nRight: `%lf`\n", value_1, value_2);
    }
}

void ASSERT_NE_char_p(
    const char* value_1,
    const char* value_2,
    const char* message,
    const char* filename,
    int line_number)
{
    if (!strcmp(value_1, value_2))
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_NE(message, filename, line_number);
        fprintf(stderr, "Left : `%s`\nRight: `%s`\n", value_1, value_2);
    }
}
