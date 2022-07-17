#include "common.h"
#include "converter.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>

Error str_to_int(const char* str, int* out_int)
{
    int ret_int = 0;
    int sign    = 1;
    if (str == NULL)
    {
        LOG_ERROR("Null pointer found");
        return ERR_NULL;
    }
    if (str[0] == '-')
    {
        sign = -1;
        str++;
    }
    else if (str[0] == '+')
    {
        str++;
    }
    if (str[0] == '\0')
    {
        LOG_ERROR("Cannot convert the provided string into an integer");
        return ERR_PARSE_STRING_TO_INT;
    }
    while (*str != '\0')
    {
        if ((*str < '0') || (*str > '9'))
        {
            LOG_ERROR("Cannot convert string containing `%c`", *str);
            return ERR_PARSE_STRING_TO_INT;
        }
        ret_int = ret_int * 10 + (*str - '0');
        str++;
    }
    *out_int = ret_int * sign;
    return ERR_ALL_GOOD;
}

Error str_to_size_t(const char* str, size_t* out_size_t)
{
    size_t ret_uint = 0;
    if (str == NULL)
    {
        LOG_ERROR("Null pointer found");
        return ERR_NULL;
    }
    if (str[0] == '+')
    {
        str++;
    }
    if (str[0] == '\0')
    {

        LOG_ERROR("Cannot convert the provided string into a size_t");
        return ERR_PARSE_STRING_TO_LONG_INT;
    }
    while (*str != '\0')
    {
        if ((*str < '0') || (*str > '9'))
        {
            LOG_ERROR("Cannot convert string containing `%c`", *str);
            return ERR_PARSE_STRING_TO_LONG_INT;
        }
        ret_uint = ret_uint * 10 + (*str - '0');
        str++;
    }
    *out_size_t = ret_uint;
    return ERR_ALL_GOOD;
}

Error str_to_float(const char* str, float* out_float)
{
    float ret_float = 0.0f;
    int sign        = 1;
    if (str == NULL)
    {
        LOG_ERROR("Null pointer found");
        return ERR_NULL;
    }
    if (str[0] == '-')
    {
        sign = -1;
        str++;
    }
    else if (str[0] == '+')
    {
        str++;
    }
    if (str[0] == '\0')
    {
        LOG_ERROR("Cannot convert the provided string into an float");
        return ERR_PARSE_STRING_TO_FLOAT;
    }
    bool dot_found   = false;
    float multiplier = 1.0f;
    while (*str != '\0')
    {
        if (*str == '.')
        {
            if (dot_found)
            {
                // This is the second dot found. Throw an error.
                LOG_ERROR("Invalid string: too many decimal separators found");
                return ERR_PARSE_STRING_TO_FLOAT;
            }
            else
            {
                dot_found = true;
                str++;
                continue;
            }
        }
        if ((*str < '0') || (*str > '9'))
        {
            LOG_ERROR("Cannot convert string containing `%c`", *str);
            return ERR_PARSE_STRING_TO_FLOAT;
        }

        ret_float = ret_float * 10.0f + (float)(*str - '0');

        if (dot_found)
        {
            multiplier *= 0.1f;
        }
        str++;
    }
    *out_float = ret_float * sign * multiplier;
    return ERR_ALL_GOOD;
}

float rounder(float to_be_rounded, float step, size_t num_of_decimals)
{
    int to_be_rounded_int;
    int step_int;
    for (size_t i = 0; i < num_of_decimals; i++)
    {
        to_be_rounded *= 10.0f;
        step *= 10.0f;
    }
    to_be_rounded_int = (int)to_be_rounded;
    step_int          = (int)step;

    int remainder     = to_be_rounded_int % step_int;
    int adjuster;
    if (to_be_rounded > 0)
    {
        adjuster = (float)remainder / step > 0.5 ? step_int : (int)0;
    }
    else
    {
        adjuster = (float)remainder / step < -0.5 ? -step_int : (int)0;
    }
    int quotient_int     = (to_be_rounded_int / step_int) * step_int + adjuster;
    float quotient_float = (float)quotient_int;
    for (size_t i = 0; i < num_of_decimals; i++)
    {
        quotient_float /= 10.0f;
    }
    return quotient_float;
}

#if TEST == 1
void test_converter()
{
    PRINT_BANNER();

    PRINT_TEST_TITLE("Valid to-int conversions");
    {
        int parsed_int;
        str_to_int("12345", &parsed_int);
        ASSERT_EQ(parsed_int, 12345, "Integer successfully converted");
        str_to_int("0", &parsed_int);
        ASSERT_EQ(parsed_int, 0, "Zero successfully converted");
        str_to_int("-0", &parsed_int);
        ASSERT_EQ(parsed_int, 0, "Negative zero successfully converted");
        str_to_int("-1", &parsed_int);
        ASSERT_EQ(parsed_int, -1, "Negative integer successfully converted");
        str_to_int("+1", &parsed_int);
        ASSERT_EQ(parsed_int, +1, "Positive integer with sign successfully converted");
        str_to_int("0002", &parsed_int);
        ASSERT_EQ(parsed_int, 2, "Positive integer with leading zeros converted");
    }
    PRINT_TEST_TITLE("Invalid to-int conversions");
    {
        int parsed_int;
        ASSERT(
            str_to_int("1f5", &parsed_int) == ERR_PARSE_STRING_TO_INT, "Invalid string detected");
        ASSERT(
            str_to_int("1f5", &parsed_int) == ERR_PARSE_STRING_TO_INT, "Invalid string detected");
        ASSERT(
            str_to_int("+-05", &parsed_int) == ERR_PARSE_STRING_TO_INT, "Invalid string detected");
        ASSERT(str_to_int(NULL, &parsed_int) == ERR_NULL, "Null string detected");
        ASSERT(str_to_int("", &parsed_int) == ERR_PARSE_STRING_TO_INT, "Empty string detected");
    }
    PRINT_TEST_TITLE("Valid to-size_t conversions");
    {
        size_t parsed_size_t;
        str_to_size_t("1234567890123", &parsed_size_t);
        ASSERT_EQ(parsed_size_t, 1234567890123, "size_t successfully converted");
        str_to_size_t("0", &parsed_size_t);
        ASSERT_EQ(parsed_size_t, 0, "Zero successfully converted");
        str_to_size_t("+1", &parsed_size_t);
        ASSERT_EQ(parsed_size_t, +1, "size_t with sign successfully converted");
        str_to_size_t("002", &parsed_size_t);
        ASSERT_EQ(parsed_size_t, 2, "size_t with leading zeros converted");
    }
    PRINT_TEST_TITLE("Invalid to-int conversions");
    {
        size_t parsed_size_t;
        ASSERT(
            str_to_size_t("1f5", &parsed_size_t) == ERR_PARSE_STRING_TO_LONG_INT,
            "Invalid string detected");
        ASSERT(
            str_to_size_t("-235", &parsed_size_t) == ERR_PARSE_STRING_TO_LONG_INT,
            "Invalid string detected");
        ASSERT(
            str_to_size_t("+-05", &parsed_size_t) == ERR_PARSE_STRING_TO_LONG_INT,
            "Invalid string detected");
        ASSERT(str_to_size_t(NULL, &parsed_size_t) == ERR_NULL, "Null string detected");
        ASSERT(
            str_to_size_t("", &parsed_size_t) == ERR_PARSE_STRING_TO_LONG_INT,
            "Empty string detected");
    }
    PRINT_TEST_TITLE("Valid to-float conversions");
    {
        float parsed_float;
        str_to_float("12.345", &parsed_float);
        ASSERT_EQ(parsed_float, 12.345f, "Float successfully converted");
        str_to_float("-12.345", &parsed_float);
        ASSERT_EQ(parsed_float, -12.345f, "Negative float successfully converted ");
        ASSERT(str_to_float("-12", &parsed_float) == ERR_ALL_GOOD, "Ok");
        ASSERT_EQ(parsed_float, -12.0f, " Negative float with no decimals successfully converted ");
        str_to_float("12", &parsed_float);
        ASSERT_EQ(parsed_float, 12.0f, " Positive float with no decimals successfully converted ");
        str_to_float("+12", &parsed_float);
        ASSERT_EQ(
            parsed_float,
            12.0f,
            "Positive float with explicit sign and no decimals successfully converted");
    }
    PRINT_TEST_TITLE("Invalid to-float conversions");
    {
        float parsed_float;
        ASSERT(
            str_to_float("1.5.4", &parsed_float) == ERR_PARSE_STRING_TO_FLOAT,
            "Too many dots detected.");
        ASSERT(
            str_to_float("1r5r4", &parsed_float) == ERR_PARSE_STRING_TO_FLOAT,
            "Invalid string detected");
    }
    PRINT_TEST_TITLE("Rounding");
    {
        ASSERT_EQ(rounder(12.34f, 0.1f, 2), 12.30f, "Rounding correct.");
        ASSERT_EQ(rounder(12.34f, 1.0f, 2), 12.00f, "Rounding correct.");
        ASSERT_EQ(rounder(12.34f, 0.2f, 2), 12.40f, "Rounding UP correct.");
        ASSERT_EQ(rounder(-12.34f, 0.1f, 2), -12.30f, "Rounding correct.");
        ASSERT_EQ(rounder(-12.34f, 1.0f, 2), -12.00f, "Rounding correct.");
        ASSERT_EQ(rounder(-12.34f, 0.2f, 2), -12.40f, "Rounding UP correct.");
    }
    /**/
}
#endif
