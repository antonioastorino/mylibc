#ifndef LOGGER_H
#define LOGGER_H
#include "common.h"
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

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

#define UNUSED(x) (void)(x)

// Ensure that errors are not printed to stderr during tests as they would cause the unit test to
// fail.
#ifndef TEST
#error "Undefined TEST"
#endif

#if TEST == 1
#define log_out stdout
#define log_err stdout
#else
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

    FILE* get_log_out_file();
    FILE* get_log_err_file();

#define PRINT_SEPARATOR()                                                                          \
    {                                                                                              \
        char date_time_str[DATE_TIME_STR_LEN];                                                     \
        get_date_time(date_time_str);                                                              \
        fprintf(log_out, "-------- %s --------\n", date_time_str);                                 \
    }

    void get_date_time(char* date_time_str);
#else /* LOG_LEVEL > LEVEL_NO_LOGS */
#define get_date_time(something)
#define PRINT_SEPARATOR()
#endif /* LOG_LEVEL > LEVEL_NO_LOGS */

#if LOG_LEVEL >= LEVEL_ERROR
#define LOG_ERROR(...)                                                                             \
    {                                                                                              \
        char date_time_str[DATE_TIME_STR_LEN];                                                     \
        get_date_time(date_time_str);                                                              \
        fprintf(log_err, "[ERROR] %s %s:%d", date_time_str, __FILENAME__, __LINE__);               \
        fprintf(log_err, " | " __VA_ARGS__);                                                       \
        fprintf(log_err, "\n");                                                                    \
        fflush(log_err);                                                                           \
    }
#define LOG_PERROR(...)                                                                            \
    {                                                                                              \
        char date_time_str[DATE_TIME_STR_LEN];                                                     \
        get_date_time(date_time_str);                                                              \
        fprintf(                                                                                   \
            log_err,                                                                               \
            "[ERROR] %s %s:%d | `%s`",                                                             \
            date_time_str,                                                                         \
            __FILENAME__,                                                                          \
            __LINE__,                                                                              \
            strerror(errno));                                                                      \
        fprintf(log_err, " | " __VA_ARGS__);                                                       \
        fprintf(log_err, "\n");                                                                    \
        fflush(log_err);                                                                           \
    }
#else
#define LOG_ERROR(...)
#endif

#if LOG_LEVEL >= LEVEL_WARNING
#define LOG_WARNING(...)                                                                           \
    {                                                                                              \
        char date_time_str[DATE_TIME_STR_LEN];                                                     \
        get_date_time(date_time_str);                                                              \
        fprintf(log_err, "[WARN ] %s %s:%d", date_time_str, __FILENAME__, __LINE__);               \
        fprintf(log_err, " | " __VA_ARGS__);                                                       \
        fprintf(log_err, "\n");                                                                    \
        fflush(log_err);                                                                           \
    }
#else
#define LOG_WARNING(...)
#endif

#if LOG_LEVEL >= LEVEL_INFO
#define LOG_INFO(...)                                                                              \
    {                                                                                              \
        char date_time_str[DATE_TIME_STR_LEN];                                                     \
        get_date_time(date_time_str);                                                              \
        fprintf(log_out, "[TNFO ] %s %s:%d", date_time_str, __FILENAME__, __LINE__);               \
        fprintf(log_out, " | " __VA_ARGS__);                                                       \
        fprintf(log_out, "\n");                                                                    \
        fflush(log_out);                                                                           \
    }
#else
#define LOG_INFO(...)
#endif

#if LOG_LEVEL >= LEVEL_DEBUG
#define LOG_DEBUG(...)                                                                             \
    {                                                                                              \
        char date_time_str[DATE_TIME_STR_LEN];                                                     \
        get_date_time(date_time_str);                                                              \
        fprintf(log_out, "[DEBUG] %s %s:%d", date_time_str, __FILENAME__, __LINE__);               \
        fprintf(log_out, " | " __VA_ARGS__);                                                       \
        fprintf(log_out, "\n");                                                                    \
        fflush(log_out);                                                                           \
    }
#else
#define LOG_DEBUG(...)
#endif

#if LOG_LEVEL >= LEVEL_TRACE
#define LOG_TRACE(...)                                                                             \
    {                                                                                              \
        char date_time_str[DATE_TIME_STR_LEN];                                                     \
        get_date_time(date_time_str);                                                              \
        fprintf(log_out, "[TRACE] %s %s:%d", date_time_str, __FILENAME__, __LINE__);               \
        fprintf(log_out, " | " __VA_ARGS__);                                                       \
        fprintf(log_out, "\n");                                                                    \
        fflush(log_out);                                                                           \
    }
#else
#define LOG_TRACE(...)
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOGGER_H */
