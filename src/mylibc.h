#ifndef MYLIBC_H
#define MYLIBC_H
#include <stdbool.h>
#include <sys/types.h>
#include <sys/time.h>
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
#include <sys/file.h>
#endif /* __linux__ */

#define UNUSED(x) (void)(x)
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define my_strncmp(s1, literal) (strncmp(s1, literal, strlen(literal)) == 0)
#define sizeof_array(__arr__) sizeof(__arr__) / sizeof(__arr__[0])
#define __autofree__ __attribute__((cleanup(my_memory_free)))
#define __autofree_cstr__ __attribute__((cleanup(my_memory_free_cstr)))
#define __autodestroy_json__ __attribute__((cleanup(JsonObj_destroy)))

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

typedef unsigned long long llu_t;
typedef long long int lld_t;

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
    ERR_PARSE_STRING_TO_LLD,
    ERR_PARSE_STRING_TO_LLU,
    ERR_PARSE_STRING_TO_DOUBLE,
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
    llu_t length;
    // Allocated memory in number of chars.
    llu_t size;
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

Error String_replace_pattern_llu_t(String*, const char*, const char*, const llu_t, size_t*);
Error String_replace_pattern_lld_t(String*, const char*, const char*, const lld_t, size_t*);
Error String_replace_pattern_double(String*, const char*, const char*, const double, size_t*);

#define String_new(...) _String_new(__FILE__, __LINE__, __VA_ARGS__)
#define String_replace_pattern(haystack, needle, replacement, out_count) \
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
    _Generic((replacement),                            \
    unsigned short:     String_replace_pattern_llu_t,  \
    unsigned int:       String_replace_pattern_llu_t,  \
    unsigned long:      String_replace_pattern_llu_t,  \
    unsigned long long: String_replace_pattern_llu_t,  \
    short int:          String_replace_pattern_lld_t,  \
    int:                String_replace_pattern_lld_t,  \
    long int:           String_replace_pattern_lld_t,  \
    long long int:      String_replace_pattern_lld_t,  \
    float:              String_replace_pattern_double, \
    double:             String_replace_pattern_double  \
    )(haystack, needle, format, replacement, out_count)
// clang-format on

/* ------- String Array ------- */
typedef struct
{
    llu_t num_of_elements;
    char* str_char_p;
    char** str_array_char_p;
} StringArray;

StringArray StringArray_empty(void);
StringArray _StringArray_new(const char* file, const int line, const char*, const char*);
void StringArray_destroy(StringArray*);

#define StringArray_new(str, separator) _StringArray_new(__FILENAME__, __LINE__, str, separator)

#ifdef _TEST
#define log_out stdout
#define log_err stdout

void ASSERT_(bool, const char*, const char*, int);
void ASSERT_OK_(Error, const char*, const char*, int);
void ASSERT_ERR_(Error, const char*, const char*, int);

void ASSERT_EQ_lld(lld_t, lld_t, const char*, const char*, int);
void ASSERT_EQ_llu(llu_t, llu_t, const char*, const char*, int);
void ASSERT_EQ_bool(bool v1, bool v2, const char*, const char*, int);
void ASSERT_EQ_double(double, double, const char*, const char*, int);
void ASSERT_EQ_char_p(const char*, const char*, const char*, const char*, int);

void ASSERT_NE_lld(lld_t, lld_t, const char*, const char*, int);
void ASSERT_NE_llu(llu_t, llu_t, const char*, const char*, int);
void ASSERT_NE_bool(bool v1, bool v2, const char*, const char*, int);
void ASSERT_NE_double(double, double, const char*, const char*, int);
void ASSERT_NE_char_p(const char*, const char*, const char*, const char*, int);

#define PRINT_BANNER()                                    \
    printf("\n");                                         \
    for (llu_t i = 0; i < strlen(__FUNCTION__) + 12; i++) \
    {                                                     \
        printf("=");                                      \
    }                                                     \
    printf("\n-- TEST: %s --\n", __FUNCTION__);           \
    for (llu_t i = 0; i < strlen(__FUNCTION__) + 12; i++) \
    {                                                     \
        printf("=");                                      \
    }                                                     \
    printf("\n");                                         \
    llu_t test_counter_ = 0;

#define ASSERT(value, message) ASSERT_(value, message, __FILENAME__, __LINE__)
#define ASSERT_OK(value, message) ASSERT_OK_(value, message, __FILENAME__, __LINE__)
#define ASSERT_ERR(value, message) ASSERT_ERR_(value, message, __FILENAME__, __LINE__)

// clang-format off
#define ASSERT_EQ(value_1, value_2, message)      \
    _Generic((value_1),                           \
        signed char        : ASSERT_EQ_lld,   \
        short              : ASSERT_EQ_lld,   \
        int                : ASSERT_EQ_lld,   \
        long               : ASSERT_EQ_lld,   \
        long long          : ASSERT_EQ_lld,   \
        unsigned char      : ASSERT_EQ_llu,  \
        unsigned short     : ASSERT_EQ_llu,  \
        unsigned int       : ASSERT_EQ_llu,  \
        unsigned long      : ASSERT_EQ_llu,  \
        unsigned long long : ASSERT_EQ_llu,  \
        bool               : ASSERT_EQ_bool,      \
        float              : ASSERT_EQ_double,     \
        double             : ASSERT_EQ_double,    \
        char*              : ASSERT_EQ_char_p,    \
        const char*        : ASSERT_EQ_char_p     \
    )(value_1, value_2, message, __FILENAME__, __LINE__)

#define ASSERT_NE(value_1, value_2, message)      \
    _Generic((value_1),                           \
        signed char        : ASSERT_NE_lld,   \
        short              : ASSERT_NE_lld,   \
        int                : ASSERT_NE_lld,   \
        long               : ASSERT_NE_lld,   \
        long long          : ASSERT_NE_lld,   \
        unsigned char      : ASSERT_NE_llu,  \
        unsigned short     : ASSERT_NE_llu,  \
        unsigned int       : ASSERT_NE_llu,  \
        unsigned long      : ASSERT_NE_llu,  \
        unsigned long long : ASSERT_NE_llu,  \
        bool               : ASSERT_NE_bool,      \
        float              : ASSERT_NE_double,     \
        double             : ASSERT_NE_double,    \
        char*              : ASSERT_NE_char_p,    \
        const char*        : ASSERT_NE_char_p     \
    )(value_1, value_2, message, __FILENAME__, __LINE__)

// clang-format on

#define PRINT_TEST_TITLE(title) printf("\n------- T:%llu < %s > -------\n", ++test_counter_, title);
void test_class_string(void);
void test_class_json(void);
void test_class_string_array(void);
void test_tcp_utils(void);
void test_logger(void);
void test_my_memory(void);
void test_fs(void);
void test_common(void);
void test_numparser(void);
void test_hashmap(void);
#else /* _TEST not defined */
#define log_out (log_out_file_p == NULL ? stdout : log_out_file_p)
#define log_err (log_err_file_p == NULL ? stderr : log_err_file_p)

#endif

#define SET_MISSING_ENTRY(result, bool_value, success_string) \
    if (is_err(result))                                       \
    {                                                         \
        LOG_ERROR("Missing entry.");                          \
        bool_value = true;                                    \
    }                                                         \
    LOG_TRACE("%s", success_string);

typedef struct JsonItem JsonItem;
typedef struct JsonValue JsonValue;
typedef struct JsonArray JsonArray;

typedef enum
{
    VALUE_ROOT,
    VALUE_UNDEFINED,
    VALUE_LLD,
    VALUE_BOOL,
    VALUE_LLU,
    VALUE_DOUBLE,
    VALUE_CSTR,
    VALUE_ARRAY,
    VALUE_ITEM,
    VALUE_INVALID,
} ValueType;

typedef struct JsonValue
{
    ValueType value_type;
    union
    {
        lld_t value_lld;                 // leaf lld_t
        llu_t value_llu;                 // leaf llu_t
        double value_double;             // leaf double
        bool value_bool;                 // leaf bool
        const char* value_cstr;          // leaf c-string
        struct JsonItem* value_child_p;  // another item
        struct JsonArray* value_array_p; // the first item of an array
    };
} JsonValue;

typedef struct JsonItem
{
    const char* key_p;
    llu_t index; // For arrays only
    JsonValue value;
    struct JsonItem* parent;
    struct JsonItem* next_sibling;
} JsonItem;

typedef struct JsonObj
{
    char* json_cstr;
    JsonItem root;
} JsonObj;

Error JsonObj_new_from_string_p(const char* file, const int line, const String*, JsonObj*);
Error JsonObj_new_from_char_p(const char* file, const int line, const char*, JsonObj*);
void JsonObj_destroy(JsonObj*);
void JsonObj_get_tokens(String*);

// Created to have a symmetry between GET_VALUE and GET_ARRAY_VALUE
Error invalid_request(const JsonArray*, llu_t, const JsonArray**);

// clang-format off
#define OBJ_GET_VALUE_h(suffix, out_type)                            \
    Error obj_get_##suffix(const JsonObj*, const char*, out_type);
    OBJ_GET_VALUE_h(value_cstr, const char**)
    OBJ_GET_VALUE_h(value_child_p, JsonItem**)
    OBJ_GET_VALUE_h(value_array_p, JsonArray**)

#define OBJ_GET_NUMBER_h(suffix, out_type)                           \
    Error obj_get_##suffix(const JsonObj*, const char*, out_type);
    OBJ_GET_VALUE_h(value_lld, lld_t*)
    OBJ_GET_VALUE_h(value_llu, llu_t*)
    OBJ_GET_VALUE_h(value_double, double*)
    OBJ_GET_VALUE_h(value_bool, bool*)

#define GET_VALUE_h(suffix, out_type) Error get_##suffix(const JsonItem*, const char*, out_type);
    GET_VALUE_h(value_cstr, const char**)
    GET_VALUE_h(value_child_p, JsonItem**)
    GET_VALUE_h(value_array_p, JsonArray**)

#define GET_NUMBER_h(suffix, out_type) Error get_##suffix(const JsonItem*, const char*, out_type);
    GET_VALUE_h(value_lld, lld_t*)
    GET_VALUE_h(value_llu, llu_t*)
    GET_VALUE_h(value_double, double*)
    GET_VALUE_h(value_bool, bool*)

#define GET_ARRAY_VALUE_h(suffix, out_type)                       \
    Error get_array_##suffix(const JsonArray*, size_t, out_type);
    GET_ARRAY_VALUE_h(value_cstr, const char**)
    GET_ARRAY_VALUE_h(value_lld, lld_t*)
    GET_ARRAY_VALUE_h(value_llu, llu_t*)
    GET_ARRAY_VALUE_h(value_double, double*)
    GET_ARRAY_VALUE_h(value_bool, bool*)
    GET_ARRAY_VALUE_h(value_child_p, JsonItem**)

#define JsonObj_new(in_json, out_json)        \
    _Generic(in_json,                         \
        const char* : _JsonObj_new,           \
        char *      : _JsonObj_new            \
        )(__FILE__, __LINE__, in_json, out_json)

#define Json_get(json_stuff, needle, out_p)                \
    _Generic ((json_stuff),                                \
        JsonObj*: _Generic((out_p),                        \
            const char** : obj_get_value_cstr,             \
            lld_t*       : obj_get_value_lld,              \
            llu_t*       : obj_get_value_llu,              \
            double*      : obj_get_value_double,           \
            bool*        : obj_get_value_bool,             \
            JsonItem**   : obj_get_value_child_p,          \
            JsonArray**  : obj_get_value_array_p           \
            ),                                             \
         JsonItem*: _Generic((out_p),                      \
            const char** : get_value_cstr,                 \
            lld_t*       : get_value_lld,                  \
            llu_t*       : get_value_llu,                  \
            double*      : get_value_double,               \
            bool*        : get_value_bool,                 \
            JsonItem**   : get_value_child_p,              \
            JsonArray**  : get_value_array_p               \
            ),                                             \
        JsonArray*: _Generic((out_p),                      \
            const char** : get_array_value_cstr,           \
            lld_t*       : get_array_value_lld,            \
            llu_t*       : get_array_value_llu,            \
            double*      : get_array_value_double,         \
            bool*        : get_array_value_bool,           \
            JsonItem**   : get_array_value_child_p,        \
            JsonArray**  : invalid_request                 \
            )                                              \
        )(json_stuff, needle, out_p)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra-semi"
;
#pragma GCC diagnostic pop
// clang-format on

// ---------- LOGGER ----------

// Ensure that errors are not printed to stderr during tests as they would cause the unit test
// to fail.

#define return_on_err(_expr)                            \
    {                                                   \
        Error _res = _expr;                             \
        if (_res != ERR_ALL_GOOD)                       \
        {                                               \
            LOG_WARNING("Error propagated from here."); \
            return _res;                                \
        }                                               \
    }

#if LOG_LEVEL > LEVEL_NO_LOGS
#define DATE_TIME_STR_LEN 32
void logger_init(const char*, const char*);

FILE* log_out_file_p = NULL;
FILE* log_err_file_p = NULL;
pthread_mutex_t log_out_mutex;
pthread_mutex_t log_err_mutex;

void get_date_time(char* date_time_str);

#define log_formatter(out_or_err, TYPE, fmt, ...)  \
    char date_time_str[DATE_TIME_STR_LEN];         \
    get_date_time(date_time_str);                  \
    pthread_mutex_lock(&log_##out_or_err##_mutex); \
    fprintf(                                       \
        log_##out_or_err,                          \
        "[%5s] <%d> %s %s:%d | " fmt "\n",         \
        #TYPE,                                     \
        getpid(),                                  \
        date_time_str,                             \
        __FILENAME__,                              \
        __LINE__ __VA_OPT__(, ) __VA_ARGS__);      \
    fflush(log_##out_or_err);                      \
    pthread_mutex_unlock(&log_##out_or_err##_mutex);

#define PRINT_SEPARATOR()                                                       \
    {                                                                           \
        char date_time_str[DATE_TIME_STR_LEN];                                  \
        get_date_time(date_time_str);                                           \
        pthread_mutex_lock(&log_out_mutex);                                     \
        fprintf(log_out, "------- <%d> %s -------\n", getpid(), date_time_str); \
        pthread_mutex_unlock(&log_out_mutex);                                   \
    }

#else /* LOG_LEVEL > LEVEL_NO_LOGS */
#define get_date_time(something)
#define PRINT_SEPARATOR()
#endif /* LOG_LEVEL > LEVEL_NO_LOGS */

#if LOG_LEVEL >= LEVEL_ERROR
#define LOG_ERROR(fmt, ...) {log_formatter(err, ERROR, fmt, __VA_ARGS__)}

#define LOG_PERROR(fmt, ...) {log_formatter(err, ERROR, fmt "`%s` | ", __VA_ARGS__ __VA_OPT__(, ) strerror(errno))}
#else /* U_TEST */
#define LOG_ERROR(...)
#endif /* U_TEST */

#if LOG_LEVEL >= LEVEL_WARNING
#define LOG_WARNING(fmt, ...) {log_formatter(err, WARN, fmt, __VA_ARGS__)}
#else
#define LOG_WARNING(...)
#endif

#if LOG_LEVEL >= LEVEL_INFO
#define LOG_INFO(fmt, ...) {log_formatter(out, INFO, fmt, __VA_ARGS__)}
#else
#define LOG_INFO(...)
#endif

#if LOG_LEVEL >= LEVEL_DEBUG
#define LOG_DEBUG(fmt, ...) {log_formatter(out, DEBUG, fmt, __VA_ARGS__)}
#else
#define LOG_DEBUG(...)
#endif

#if LOG_LEVEL >= LEVEL_TRACE
#define LOG_TRACE(fmt, ...) {log_formatter(out, TRACE, fmt, __VA_ARGS__)}
#else
#define LOG_TRACE(...)
#endif
// ---------- LOGGER END ----------
#define MAX_MAP_KEY_LEN (256)

typedef enum
{
    HM_TYPE_LLU,
    HM_TYPE_LLD,
    HM_TYPE_CSTR,
} HashMapType;

typedef struct _HashMapEntry HashMapEntry;

typedef struct _HashMapEntry
{
    char key[MAX_MAP_KEY_LEN];
    union
    {
        llu_t value_llu;
        lld_t value_lld;
        char* value_cstr;
    };
    HashMapEntry* prev_p;
    HashMapEntry* next_p;
} HashMapEntry;

typedef struct
{
    size_t capacity;
    size_t size;
    HashMapType type;
    HashMapEntry* entries;
} HashMap;

#define __hm_autofree__ __attribute__((cleanup(HashMap_delete)))
#define HashMap_new_with_capacity(__hm_type, __capacity) __HashMap_new_with_capacity(__FILE__, __LINE__, __hm_type, __capacity)
#define HashMap_get_cstr_malloc(__hm_p, __key, __out_value_pp) __HashMap_get_cstr_malloc(__FILE__, __LINE__, __hm_p, __key, __out_value_pp)

Error numparser_cstr_to_lld(const char* str_p, lld_t* out_lld_p, char terminator);
Error numparser_cstr_to_llu(const char* str_p, llu_t* out_llu_p, char terminator);
Error numparser_cstr_to_double(const char* str_p, double*, char terminator);
double numparser_rounder(double to_be_rounded, double step, llu_t num_of_decimals);

// Folders only.
Error fs_mkdir(const char*, mode_t);
Error fs_mkdir_with_parents(const char*, mode_t);
Error fs_rmdir(const char*);
Error fs_ls(const char*);
// Files only.
Error fs_rm_from_path_as_char_p(const char*);
Error fs_append(const char*, const char*);
Error fs_create_with_content(const char*, const char*);
// Files and folders.
bool fs_does_exist(const char*);
Error fs_rm_r(const char*);
bool fs_is_file(const char*);
bool fs_is_folder(const char*);
Error fs_get_file_size(const char*, off_t*);

// clang-format off
#define fs_rm(file_path_p)                                        \
    _Generic((file_path_p),                                       \
        const char*   : fs_rm_from_path_as_char_p,                \
        char*         : fs_rm_from_path_as_char_p                 \
    )(file_path_p)

#define fs_read_to_string(file_path_p, out_string)                \
    _Generic((file_path_p),                                       \
        const char* : _fs_read_to_string,                         \
        char* : _fs_read_to_string                                \
    )(__FILE__, __LINE__,file_path_p, out_string)
// clang-format on

void* my_memory_malloc(const char* file, const int line, size_t size);
void* my_memory_realloc(const char* file, const int line, void* ptr, size_t size);
int my_memory_vasprintf(const char* file, const int line, char**, const char*, va_list);
int my_memory_asprintf(const char* file, const int line, char** ptr_p, const char* format, ...);
void my_memory_free(void*);
void my_memory_free_cstr(char**);

Error tcp_utils_server_init(uint16_t port);
Error tcp_utils_accept(int*);
void tcp_utils_close_server_socket(void);
void tcp_utils_close_client_socket(int);
Error tcp_utils_read(char*, int);
Error tcp_utils_write(char*, int);
Error tcp_utils_send_file(char*, long, int);

#endif /* MYLIBC_H */
