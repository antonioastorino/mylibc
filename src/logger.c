#include "logger.h"
#include <time.h>
#include <unistd.h>

#if LOG_LEVEL > LEVEL_NO_LOGS

static FILE* log_out_file_p = NULL;
static FILE* log_err_file_p = NULL;

FILE* get_log_out_file() { return log_out_file_p == NULL ? stdout : log_out_file_p; }
FILE* get_log_err_file() { return log_err_file_p == NULL ? stderr : log_err_file_p; }

void logger_init(const char* log_out_file_path_str, const char* log_err_file_path_str)
{
    printf("PID `%d` - initializing logger.\n", getpid());
    if ((log_out_file_p != NULL) || (log_err_file_p != NULL))
    {
        perror("Fatal error: logger already initialized!");
        exit(-1);
    }

    log_out_file_p = fopen(log_out_file_path_str, "w");
    if (log_out_file_p == NULL)
    {
        perror("Fatal error: could not open logger out file.");
        exit(-1);
    }
    fprintf(log_out_file_p, "PID `%d` - logger out file opened.\n", getpid());
    if (strcmp(log_out_file_path_str, log_err_file_path_str) != 0)
    {
        log_err_file_p = fopen(log_err_file_path_str, "w");
        if (log_err_file_p == NULL)
        {
            perror("Fatal error: could not open logger err file");
            exit(-1);
        }
        fprintf(log_err_file_p, "PID `%d` - logger err file opened.\n", getpid());
    }
    else
    {
        log_err_file_p = log_out_file_p;
    }
}

void get_date_time(char* date_time_str)
{
    time_t ltime;
    struct tm result;
    ltime = time(NULL);
    localtime_r(&ltime, &result);
    // The string must be at least 26 character long. The returned value contains a \n\0 at the end.
    asctime_r(&result, date_time_str);
    // Overwrite the \n to avoid a new line.
    date_time_str[24] = 0;
}
#endif
