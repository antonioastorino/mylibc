#ifndef CLASS_JSON_H
#define CLASS_JSON_H
#include "class_string.h"
#include "common.h"
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

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

#if TEST == 1
    void test_class_json(void);
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
