#ifndef LOGGER_H
#define LOGGER_H
#include "common.h"
#include <pthread.h>
#include <unistd.h>
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
    pthread_mutex_t* logger_get_out_mut_p();
    pthread_mutex_t* logger_get_err_mut_p();

    FILE* get_log_out_file();
    FILE* get_log_err_file();

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
        pthread_mutex_lock(logger_get_err_mut_p());                                                \
        fprintf(log_out, "-------- <%d> %s --------\n", getpid(), date_time_str);                  \
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
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOGGER_H */
