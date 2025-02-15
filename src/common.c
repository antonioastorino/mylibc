#include "common.h"
#include <stdlib.h>

// ---------- ERROR ----------

// ---------- LOGGER ----------
#if LOG_LEVEL > LEVEL_NO_LOGS

static FILE* log_out_file_p = NULL;
static FILE* log_err_file_p = NULL;
static pthread_mutex_t log_out_mutex;
static pthread_mutex_t log_err_mutex;

FILE* get_log_out_file(void) { return log_out_file_p == NULL ? stdout : log_out_file_p; }
FILE* get_log_err_file(void) { return log_err_file_p == NULL ? stderr : log_err_file_p; }

pthread_mutex_t* logger_get_out_mut_p(void) { return &log_out_mutex; }
pthread_mutex_t* logger_get_err_mut_p(void) { return &log_err_mutex; }

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

#endif /* LOG_LEVEL > LEVEL_NO_LOGS */
#if TEST == 1
void test_logger(void)
{
    PRINT_BANNER()
    PRINT_TEST_TITLE("Logging all levels");
    PRINT_SEPARATOR();
    LOG_TRACE("Log trace.");
    LOG_DEBUG("Log debug.");
    LOG_INFO("Log info.");
    LOG_WARNING("Log warning.");
    LOG_ERROR("Log error.");
}
#endif /* TEST == 1 */

// ---------- ASSERT ----------

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

void ASSERT_OK_(Error result, const char* message, const char* filename, int line_number)
{
    if (is_ok(result))
    {
        PRINT_PASS_MESSAGE(message);
    }
    else
    {
        PRINT_FAIL_MESSAGE_(message, filename, line_number);
        fprintf(stderr, "The value is `false`\n");
    }
}

void ASSERT_ERR_(Error result, const char* message, const char* filename, int line_number)
{
    if (is_err(result))
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

void ASSERT_EQ_uint32(
    uint32_t value_1,
    uint32_t value_2,
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
