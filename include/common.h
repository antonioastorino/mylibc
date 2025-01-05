#ifndef COMMON_H
#define COMMON_H
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define TEST 0
#define MEMORY_CHECK 0

#define UNUSED(x) (void)(x)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
// ---------- ERROR ----------
#ifndef __APP_DEFINED__
#define __APP_DEFINED__
#endif /* __APP_DEFINED__ */

    typedef enum
    {
        ERR_ALL_GOOD,
        ERR_INVALID,
        ERR_UNDEFINED,
        ERR_INFALLIBLE,
        ERR_UNEXPECTED,
        ERR_FORBIDDEN,
        ERR_OUT_OF_RANGE,
        ERR_PERMISSION_DENIED,
        ERR_INTERRUPTION,
        ERR_NULL,
        ERR_PARSE_STRING_TO_INT,
        ERR_PARSE_STRING_TO_LONG_INT,
        ERR_PARSE_STRING_TO_FLOAT,
        ERR_EMPTY_STRING,
        ERR_JSON_INVALID,
        ERR_JSON_MISSING_ENTRY,
        ERR_TYPE_MISMATCH,
        ERR_FS_INTERNAL,
        ERR_TCP_INTERNAL,
        ERR_NOT_FOUND,
        ERR_FATAL,
        __APP_DEFINED__
    } Error;

#define is_err(_expr) (_expr != ERR_ALL_GOOD)
#define is_ok(_expr) (_expr == ERR_ALL_GOOD)

    // ---------- LOGGER ----------

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LEVEL_TRACE 5
#define LEVEL_DEBUG 4
#define LEVEL_INFO 3
#define LEVEL_WARNING 2
#define LEVEL_ERROR 1
#define LEVEL_NO_LOGS 0
#ifndef LOG_LEVEL
#define LOG_LEVEL LEVEL_TRACE
#endif

    // Ensure that errors are not printed to stderr during tests as they would cause the unit test
    // to fail.

#if TEST == 1
#define log_out stdout
#define log_err stdout
    void test_logger(void);
#else /* TEST == 1 */
#define log_out get_log_out_file()
#define log_err get_log_err_file()
#endif

#define return_on_err(_expr)                                                                       \
    {                                                                                              \
        Error _res = _expr;                                                                        \
        if (_res != ERR_ALL_GOOD)                                                                  \
        {                                                                                          \
            LOG_WARNING("Error propagated from here.");                                            \
            return _res;                                                                           \
        }                                                                                          \
    }

#if LOG_LEVEL > LEVEL_NO_LOGS
#define DATE_TIME_STR_LEN 26
    void logger_init(const char*, const char*);
    pthread_mutex_t* logger_get_out_mut_p(void);
    pthread_mutex_t* logger_get_err_mut_p(void);

    FILE* get_log_out_file(void);
    FILE* get_log_err_file(void);

    void get_date_time(char* date_time_str);

#define log_header_o(TYPE)                                                                         \
    char date_time_str[DATE_TIME_STR_LEN];                                                         \
    get_date_time(date_time_str);                                                                  \
    pthread_mutex_lock(logger_get_out_mut_p());                                                    \
    fprintf(                                                                                       \
        log_out,                                                                                   \
        "[%5s] <%d> %s %s:%d | ",                                                                  \
        #TYPE,                                                                                     \
        getpid(),                                                                                  \
        date_time_str,                                                                             \
        __FILENAME__,                                                                              \
        __LINE__);

#define log_header_e(TYPE)                                                                         \
    char date_time_str[DATE_TIME_STR_LEN];                                                         \
    get_date_time(date_time_str);                                                                  \
    pthread_mutex_lock(logger_get_err_mut_p());                                                    \
    fprintf(                                                                                       \
        log_err,                                                                                   \
        "[%5s] <%d> %s %s:%d | ",                                                                  \
        #TYPE,                                                                                     \
        getpid(),                                                                                  \
        date_time_str,                                                                             \
        __FILENAME__,                                                                              \
        __LINE__);

#define log_footer_o(...)                                                                          \
    fprintf(log_out, __VA_ARGS__);                                                                 \
    fprintf(log_out, "\n");                                                                        \
    fflush(log_out);                                                                               \
    pthread_mutex_unlock(logger_get_out_mut_p());

#define log_footer_e(...)                                                                          \
    fprintf(log_err, __VA_ARGS__);                                                                 \
    fprintf(log_err, "\n");                                                                        \
    fflush(log_err);                                                                               \
    pthread_mutex_unlock(logger_get_err_mut_p());

#define PRINT_SEPARATOR()                                                                          \
    {                                                                                              \
        char date_time_str[DATE_TIME_STR_LEN];                                                     \
        get_date_time(date_time_str);                                                              \
        pthread_mutex_lock(logger_get_out_mut_p());                                                \
        fprintf(log_out, "------- <%d> %s -------\n", getpid(), date_time_str);                    \
        pthread_mutex_unlock(logger_get_out_mut_p());                                              \
    }

#else /* LOG_LEVEL > LEVEL_NO_LOGS */
#define get_date_time(something)
#define PRINT_SEPARATOR()
#endif /* LOG_LEVEL > LEVEL_NO_LOGS */

#if LOG_LEVEL >= LEVEL_ERROR
#define LOG_ERROR(...)                                                                             \
    {                                                                                              \
        log_header_e(ERROR);                                                                       \
        log_footer_e(__VA_ARGS__);                                                                 \
    }

#define LOG_PERROR(...)                                                                            \
    {                                                                                              \
        log_header_e(ERROR);                                                                       \
        fprintf(log_err, "`%s` | ", strerror(errno));                                              \
        log_footer_e(__VA_ARGS__);                                                                 \
    }
#else
#define LOG_ERROR(...)
#endif

#if LOG_LEVEL >= LEVEL_WARNING
#define LOG_WARNING(...)                                                                           \
    {                                                                                              \
        log_header_e(WARN);                                                                        \
        log_footer_e(__VA_ARGS__);                                                                 \
    }
#else
#define LOG_WARNING(...)
#endif

#if LOG_LEVEL >= LEVEL_INFO
#define LOG_INFO(...)                                                                              \
    {                                                                                              \
        log_header_o(INFO);                                                                        \
        log_footer_o(__VA_ARGS__);                                                                 \
    }
#else
#define LOG_INFO(...)
#endif

#if LOG_LEVEL >= LEVEL_DEBUG
#define LOG_DEBUG(...)                                                                             \
    {                                                                                              \
        log_header_o(DEBUG);                                                                       \
        log_footer_o(__VA_ARGS__);                                                                 \
    }
#else
#define LOG_DEBUG(...)
#endif

#if LOG_LEVEL >= LEVEL_TRACE
#define LOG_TRACE(...)                                                                             \
    {                                                                                              \
        log_header_o(TRACE);                                                                       \
        log_footer_o(__VA_ARGS__);                                                                 \
    }
#else
#define LOG_TRACE(...)
#endif

// ---------- ASSERT ----------
#if TEST == 1

    void ASSERT_(bool, const char*, const char*, int);
    void ASSERT_OK_(Error, const char*, const char*, int);
    void ASSERT_ERR_(Error, const char*, const char*, int);

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
#define ASSERT_OK(value, message) ASSERT_OK_(value, message, __FILE__, __LINE__)
#define ASSERT_ERR(value, message) ASSERT_ERR_(value, message, __FILE__, __LINE__)
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // COMMON_H
