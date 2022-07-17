#ifndef COMMON_H
#define COMMON_H
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEST 1

#include "error.h"
#include "logger.h"
#include "my_memory.h"

#define UNUSED(x) (void)(x)
#if TEST == 1

void ASSERT_(bool, const char*, const char*, int);

void ASSERT_EQ_int(int, int, const char*, const char*, int);
void ASSERT_EQ_uint8(uint8_t, uint8_t, const char*, const char*, int);
void ASSERT_EQ_uint16(uint16_t, uint16_t, const char*, const char*, int);
void ASSERT_EQ_uint(size_t, size_t, const char*, const char*, int);
void ASSERT_EQ_bool(bool v1, bool v2, const char*, const char*, int);
void ASSERT_EQ_float(float, float, const char*, const char*, int);
void ASSERT_EQ_double(double, double, const char*, const char*, int);
void ASSERT_EQ_char_p(const char*, const char*, const char*, const char*, int);

void ASSERT_NE_int(int, int, const char*, const char*, int);
void ASSERT_NE_uint8(uint8_t, uint8_t, const char*, const char*, int);
void ASSERT_NE_uint16(uint16_t, uint16_t, const char*, const char*, int);
void ASSERT_NE_uint(size_t, size_t, const char*, const char*, int);
void ASSERT_NE_bool(bool v1, bool v2, const char*, const char*, int);
void ASSERT_NE_float(float, float, const char*, const char*, int);
void ASSERT_NE_double(double, double, const char*, const char*, int);
void ASSERT_NE_char_p(const char*, const char*, const char*, const char*, int);

#define PRINT_BANNER()                                                                             \
    printf("\n");                                                                                  \
    for (size_t i = 0; i < strlen(__FUNCTION__) + 12; i++)                                         \
    {                                                                                              \
        printf("=");                                                                               \
    }                                                                                              \
    printf("\n-- TEST: %s --\n", __FUNCTION__);                                                    \
    for (size_t i = 0; i < strlen(__FUNCTION__) + 12; i++)                                         \
    {                                                                                              \
        printf("=");                                                                               \
    }                                                                                              \
    printf("\n");                                                                                  \
    size_t test_counter_ = 0;

#define ASSERT(value, message) ASSERT_(value, message, __FILE__, __LINE__)

// clang-format off
#define ASSERT_EQ(value_1, value_2, message)      \
    _Generic((value_1),                           \
        int           : ASSERT_EQ_int,            \
        int16_t       : ASSERT_EQ_int,            \
        uint8_t       : ASSERT_EQ_uint8,          \
        uint16_t      : ASSERT_EQ_uint16,         \
        size_t        : ASSERT_EQ_uint,           \
        bool          : ASSERT_EQ_bool,           \
        float         : ASSERT_EQ_float,          \
        double        : ASSERT_EQ_double,         \
        char*         : ASSERT_EQ_char_p,         \
        const char*   : ASSERT_EQ_char_p          \
    )(value_1, value_2, message, __FILE__, __LINE__)

#define ASSERT_NE(value_1, value_2, message)      \
    _Generic((value_1),                           \
        int           : ASSERT_NE_int,            \
        uint8_t       : ASSERT_NE_uint8,          \
        uint16_t      : ASSERT_NE_uint16,         \
        size_t        : ASSERT_NE_uint,           \
        bool          : ASSERT_NE_bool,           \
        float         : ASSERT_NE_float,          \
        double        : ASSERT_NE_double,         \
        char*         : ASSERT_NE_char_p,         \
        const char*   : ASSERT_NE_char_p          \
    )(value_1, value_2, message, __FILE__, __LINE__)

// clang-format on

#define PRINT_TEST_TITLE(title) printf("\n------- T:%lu < %s > -------\n", ++test_counter_, title);
#endif

#endif // COMMON_H
