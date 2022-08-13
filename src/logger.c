#include "logger.h"
#include <time.h>
#include <stdlib.h>

#if LOG_LEVEL > LEVEL_NO_LOGS

static FILE* log_out_file_p = NULL;
static FILE* log_err_file_p = NULL;
static pthread_mutex_t log_out_mutex;
static pthread_mutex_t log_err_mutex;

FILE* get_log_out_file() { return log_out_file_p == NULL ? stdout : log_out_file_p; }
FILE* get_log_err_file() { return log_err_file_p == NULL ? stderr : log_err_file_p; }

pthread_mutex_t* logger_get_out_mut_p() { return &log_out_mutex; }
pthread_mutex_t* logger_get_err_mut_p() { return &log_err_mutex; }

void _logger_open_out_file(const char* log_out_file_path_str)
{
    log_out_file_p = fopen(log_out_file_path_str, "a");
    if (log_out_file_p == NULL)
    {
        perror("Fatal error: could not open logger out file.");
        exit(-1);
    }
    LOG_INFO("Logger OUT file opened.");
}
void _logger_open_err_file(const char* log_err_file_path_str)
{
    log_err_file_p = fopen(log_err_file_path_str, "a");
    if (log_err_file_p == NULL)
    {
        perror("Fatal error: could not open logger out file.");
        exit(-1);
    }
    LOG_INFO("Logger ERR file opened.");
}

void logger_init(const char* log_out_file_path_str, const char* log_err_file_path_str)
{
    static bool logger_initialized = false;
    if (logger_initialized)
    {
        LOG_ERROR("Logger already initialized.");
        return;
    }
    LOG_INFO("Initializing logger.");
    pthread_mutex_init(&log_out_mutex, NULL);
    pthread_mutex_init(&log_err_mutex, NULL);
    logger_initialized = true;

    if (log_out_file_path_str != NULL && log_err_file_path_str != NULL)
    {
        _logger_open_out_file(log_out_file_path_str);
        if (strcmp(log_out_file_path_str, log_err_file_path_str) == 0)
        {
            log_err_file_p = log_out_file_p;
            LOG_INFO("Logger ERR file matches logger OUT file.");
        }
        else
        {
            _logger_open_err_file(log_err_file_path_str);
        }
    }
    if (log_out_file_path_str == NULL)
    {
        log_out_file_p = stdout;
        LOG_INFO("Logger OUT set to standard out.");
    }
    else if (log_out_file_p == NULL)
    {
        _logger_open_out_file(log_out_file_path_str);
    }

    if (log_err_file_path_str == NULL)
    {
        log_err_file_p = stderr;
        LOG_INFO("Logger ERR set to standard error.");
    }
    else if (log_err_file_p == NULL)
    {
        _logger_open_err_file(log_err_file_path_str);
    }
}

void get_date_time(char* date_time_str)
{
    time_t ltime;
    struct tm result;
    ltime = time(NULL);
    localtime_r(&ltime, &result);
    // The string must be at least 26 character long. The returned value contains a \n\0 at
    // the end.
    asctime_r(&result, date_time_str);
    // Overwrite the \n to avoid a new line.
    date_time_str[24] = 0;
}
#endif
