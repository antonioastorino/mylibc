#ifndef HEADER_H
#define HEADER_H
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <fts.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#ifdef __linux__
#include <sys/sendfile.h>
#endif /* __linux__ */

#define UNUSED(x) (void)(x)
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define TCP_MAX_MSG_LEN 65535
#define TCP_MAX_CONNECTIONS 1023

#define LEVEL_TRACE 5
#define LEVEL_DEBUG 4
#define LEVEL_INFO 3
#define LEVEL_WARNING 2
#define LEVEL_ERROR 1
#define LEVEL_NO_LOGS 0

#ifndef LOG_LEVEL
#define LOG_LEVEL LEVEL_TRACE
#endif

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

#define is_err(_expr) ((_expr) != ERR_ALL_GOOD)
#define is_ok(_expr) ((_expr) == ERR_ALL_GOOD)
/* ------- String ------- */
typedef struct String
{
    // Length that would be returned by `strlen(str)`.
    size_t length;
    // Allocated memory in number of chars.
    size_t size;
    // Array of chars whose allocated length >= `len`.
    char* str;
} String;

/************************************* (De)Constructors ***************************************/
String _String_new(const char* file, int line, ...);
String String_join(const char**, const char*);
String String_clone(const String*);
void String_destroy(String*);

/***************************************** Checkers *******************************************/
bool String_is_null(const String*);
bool String_starts_with(const String*, const char*);
Error _String_between_patterns_in_string_p(
    const char* file,
    const int line,
    String*,
    const char*,
    const char*,
    String*);
Error _String_between_patterns_in_char_p(
    const char* file,
    const int line,
    const char*,
    const char*,
    const char*,
    String*);
bool String_match(const String*, const String*);

/**************************************** Modifiers *******************************************/
Error String_replace_char(String*, const char, const char, size_t*);
Error _String_replace_pattern(
    const char* file,
    const int line,
    String*,
    const char*,
    const char*,
    size_t*);
Error String_replace_pattern_size_t(String*, const char*, const char*, const size_t, size_t*);
Error String_replace_pattern_float(String*, const char*, const char*, const float, size_t*);
Error String_replace_pattern_int(String*, const char*, const char*, const int, size_t*);

#define String_new(...) _String_new(__FILE__, __LINE__, __VA_ARGS__)
#define String_replace_pattern(haystack, needle, replacement, out_count)                           \
    _String_replace_pattern(__FILE__, __LINE__, haystack, needle, replacement, out_count)
#define String_empty(string_name) String string_name = {.length = 0, .size = 0, .str = NULL}
#define String_full(string_name, ...) String string_name = String_new(__VA_ARGS__)

// clang-format off
#define String_between_patterns(in_value, prefix, suffix, out_string) \
    _Generic((in_value), \
     String*     : _String_between_patterns_in_string_p, \
     const char* : _String_between_patterns_in_char_p \
     )(__FILE__, __LINE__, in_value, prefix, suffix, out_string)

#define String_replace_pattern_with_format(haystack, needle, format, replacement, out_count) \
    _Generic((replacement), \
    size_t: String_replace_pattern_size_t, \
    float: String_replace_pattern_float, \
    int: String_replace_pattern_int \
    )(haystack, needle, format, replacement, out_count)
// clang-format on

/* ------- String Array ------- */
typedef struct
{
    size_t num_of_elements;
    char* str_char_p;
    char** str_array_char_p;
} StringArray;

StringArray StringArray_empty(void);
StringArray _StringArray_new(const char* file, const int line, const char*, const char*);
void StringArray_destroy(StringArray*);

#define StringArray_new(str, separator) _StringArray_new(__FILE__, __LINE__, str, separator)

#ifdef _TEST
#define log_out stdout
#define log_err stdout
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
void test_class_string(void);
void test_class_json(void);
void test_class_string_array(void);
void test_tcp_utils(void);
void test_logger(void);
void test_my_memory(void);
void test_fs_utils(void);
void test_common(void);
void test_converter(void);
#else /* _TEST not defined */
#define log_out get_log_out_file()
#define log_err get_log_err_file()
#endif

#define SET_MISSING_ENTRY(result, bool_value, success_string)                                      \
    if (is_err(result))                                                                            \
    {                                                                                              \
        LOG_ERROR("Missing entry.");                                                               \
        bool_value = true;                                                                         \
    }                                                                                              \
    LOG_TRACE("%s", success_string);

typedef struct JsonItem JsonItem;
typedef struct JsonValue JsonValue;
typedef struct JsonArray JsonArray;

typedef enum
{
    VALUE_ROOT,
    VALUE_UNDEFINED,
    VALUE_INT,
    VALUE_BOOL,
    VALUE_UINT,
    VALUE_FLOAT,
    VALUE_STR,
    VALUE_ARRAY,
    VALUE_ITEM,
    VALUE_INVALID,
} ValueType;

typedef struct JsonValue
{
    ValueType value_type;
    union
    {
        int value_int;                   // leaf int
        size_t value_uint;               // leaf size_t
        float value_float;               // leaf float
        bool value_bool;                 // leaf bool
        const char* value_char_p;        // leaf c-string
        struct JsonItem* value_child_p;  // another item
        struct JsonArray* value_array_p; // the first item of an array
    };
} JsonValue;

typedef struct JsonItem
{
    const char* key_p;
    size_t index; // For arrays only
    JsonValue value;
    struct JsonItem* parent;
    struct JsonItem* next_sibling;
} JsonItem;

typedef struct JsonObj
{
    String json_string;
    JsonItem root;
} JsonObj;

Error JsonObj_new_from_string_p(const char* file, const int line, const String*, JsonObj*);
Error JsonObj_new_from_char_p(const char* file, const int line, const char*, JsonObj*);
void JsonObj_destroy(JsonObj*);
void JsonObj_get_tokens(String*);

// Created to have a symmetry between GET_VALUE and GET_ARRAY_VALUE
Error invalid_request(const JsonArray*, size_t, const JsonArray**);

// clang-format off
#define OBJ_GET_VALUE_h(suffix, out_type)                                                          \
    Error obj_get_##suffix(const JsonObj*, const char*, out_type);
    OBJ_GET_VALUE_h(value_char_p, const char**)
    OBJ_GET_VALUE_h(value_child_p, JsonItem**)
    OBJ_GET_VALUE_h(value_array_p, JsonArray**)

#define OBJ_GET_NUMBER_h(suffix, out_type)                                                         \
    Error obj_get_##suffix(const JsonObj*, const char*, out_type);
    OBJ_GET_VALUE_h(value_int, int*)
    OBJ_GET_VALUE_h(value_uint, size_t*)
    OBJ_GET_VALUE_h(value_float, float*)
    OBJ_GET_VALUE_h(value_bool, bool*)

#define GET_VALUE_h(suffix, out_type) Error get_##suffix(const JsonItem*, const char*, out_type);
    GET_VALUE_h(value_char_p, const char**)
    GET_VALUE_h(value_child_p, JsonItem**)
    GET_VALUE_h(value_array_p, JsonArray**)

#define GET_NUMBER_h(suffix, out_type) Error get_##suffix(const JsonItem*, const char*, out_type);
    GET_VALUE_h(value_int, int*)
    GET_VALUE_h(value_uint, size_t*)
    GET_VALUE_h(value_float, float*)
    GET_VALUE_h(value_bool, bool*)

#define GET_ARRAY_VALUE_h(suffix, out_type)                                                        \
    Error get_array_##suffix(const JsonArray*, size_t, out_type);
    GET_ARRAY_VALUE_h(value_char_p, const char**)
    GET_ARRAY_VALUE_h(value_int, int*)
    GET_ARRAY_VALUE_h(value_uint, size_t*)
    GET_ARRAY_VALUE_h(value_float, float*)
    GET_ARRAY_VALUE_h(value_bool, bool*)
    GET_ARRAY_VALUE_h(value_child_p, JsonItem**)

#define JsonObj_new(in_json, out_json)                                                             \
    _Generic(in_json,                                                                              \
        const char*  : JsonObj_new_from_char_p,                                                    \
        String*      : JsonObj_new_from_string_p                                                   \
        )(__FILE__, __LINE__,in_json, out_json)

#define Json_get(json_stuff, needle, out_p)                                                        \
    _Generic ((json_stuff),                                                                        \
        JsonObj*: _Generic((out_p),                                                                \
            const char** : obj_get_value_char_p,                                                   \
            int*         : obj_get_value_int,                                                      \
            size_t*      : obj_get_value_uint,                                                     \
            float*       : obj_get_value_float,                                                    \
            bool*        : obj_get_value_bool,                                                     \
            JsonItem**   : obj_get_value_child_p,                                                  \
            JsonArray**  : obj_get_value_array_p                                                   \
            ),                                                                                     \
         JsonItem*: _Generic((out_p),                                                              \
            const char** : get_value_char_p,                                                       \
            int*         : get_value_int,                                                          \
            size_t*      : get_value_uint,                                                         \
            float*       : get_value_float,                                                        \
            bool*        : get_value_bool,                                                         \
            JsonItem**   : get_value_child_p,                                                      \
            JsonArray**  : get_value_array_p                                                       \
            ),                                                                                     \
        JsonArray*: _Generic((out_p),                                                              \
            const char** : get_array_value_char_p,                                                 \
            int*         : get_array_value_int,                                                    \
            size_t*      : get_array_value_uint,                                                   \
            float*       : get_array_value_float,                                                  \
            bool*        : get_array_value_bool,                                                   \
            JsonItem**   : get_array_value_child_p,                                                \
            JsonArray**  : invalid_request                                                         \
            )                                                                                      \
        )(json_stuff, needle, out_p)
// clang-format on

// ---------- LOGGER ----------

// Ensure that errors are not printed to stderr during tests as they would cause the unit test
// to fail.

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

Error str_to_int(const char* str_p, int*);
Error str_to_size_t(const char* str_p, size_t*);
Error str_to_uint8_t(const char* str_p, uint8_t*);
Error str_to_float(const char* str_p, float*);
float rounder(float to_be_rounded, float step, size_t num_of_decimals);

// Folders only.
Error fs_utils_mkdir(const char*, mode_t);
Error fs_utils_mkdir_p(const char*, mode_t);
Error fs_utils_rmdir(const char*);
Error fs_utils_ls(const char*);
// Files only.
Error fs_utils_rm_from_path_as_char_p(const char*);
Error _fs_utils_read_to_string(const char* file, const int line, const char*, String*);
Error fs_utils_append(const char*, const char*);
Error fs_utils_create_with_content(const char*, const char*);
// Files and folders.
bool fs_utils_does_exist(const char*);
Error fs_utils_rm_r(const char*);
bool fs_utils_is_file(const char*);
bool fs_utils_is_folder(const char*);
Error fs_utils_get_file_size(const char*, off_t*);

// clang-format off
#define fs_utils_rm(file_path_p)                                        \
    _Generic((file_path_p),                                             \
        const char*   : fs_utils_rm_from_path_as_char_p,                \
        char*         : fs_utils_rm_from_path_as_char_p                 \
    )(file_path_p)

#define fs_utils_read_to_string(file_path_p, out_string)                \
    _Generic((file_path_p),                                             \
        const char* : _fs_utils_read_to_string,                         \
        char* : _fs_utils_read_to_string                                \
    )(__FILE__, __LINE__,file_path_p, out_string)
// clang-format on

void* my_memory_malloc(const char* file, const int line, size_t size);
void* my_memory_realloc(const char* file, const int line, void* ptr, size_t size);
int my_memory_vasprintf(const char* file, const int line, char**, const char*, va_list);
int my_memory_asprintf(const char* file, const int line, char** ptr_p, const char* format, ...);
void my_memory_free(void*);

Error tcp_utils_server_init(uint16_t port);
Error tcp_utils_accept(int*);
void tcp_utils_close_server_socket(void);
void tcp_utils_close_client_socket(int);
Error tcp_utils_read(char*, int);
Error tcp_utils_write(char*, int);
Error tcp_utils_send_file(char*, long, int);

#endif /* HEADER_H */
