#ifndef CLASS_JSON_H
#define CLASS_JSON_H
#include "class_string.h"
#include "common.h"

// A air `key` `value`, plus a `parent` to make a double-linked list, and a `sibling`.
typedef struct JsonItem JsonItem;
// Any possible value, including another `JsonItem`.
typedef struct JsonValue JsonValue;

typedef enum
{
    VALUE_UNDEFINED,
    VALUE_INT,
    VALUE_UINT,
    VALUE_FLOAT,
    VALUE_STR,
    VALUE_ARRAY,
    VALUE_ITEM,
    VALUE_INVALID,
} ValueType;
 
/**
 *
 * JSON operators (if found outside a string):
 *     {"  => create an item and its key is starting
 *             :[  => create value
 *                  ,   => creates a sibling value (in array) of the same type the previous item
 *         ":  => create a single value
 *         ,"  => create a sibling key
 *     ]," => create sibling key
 *     },"  => create sibling item
 *     }   => go to the parent
 * Invalid sequences (outside strings):
 *     { not followed by "
 *     [[
 *     ,,
 *     ::
 *     :,
 *     ] not followed by , or }
 */

// Used only for returning data in a convenient way. Not used for storage.
typedef struct JsonArray
{
    struct JsonItem* element;
} JsonArray;

typedef struct JsonValue
{
    ValueType value_type;
    union
    {
        int value_int;                   // leaf int
        size_t value_uint;               // leaf size_t
        float value_float;               // leaf float
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
    JsonItem* root_p;
} JsonObj;

Error JsonObj_new_from_string_p(const String*, JsonObj*);
Error JsonObj_new_from_char_p(const char*, JsonObj*);
void JsonObj_destroy(JsonObj*);
void JsonObj_get_tokens(String*);

// Created to have a symmetry between GET_VALUE and GET_ARRAY_VALUE
Error invalid_request(const JsonArray*, size_t, const JsonArray**);

#define GET_VALUE_h(suffix, out_type) Error get_##suffix(const JsonItem*, const char*, out_type);
GET_VALUE_h(value_char_p, const char**);
GET_VALUE_h(value_child_p, JsonItem**);
GET_VALUE_h(value_array_p, JsonArray**);

#define GET_NUMBER_h(suffix, out_type) Error get_##suffix(const JsonItem*, const char*, out_type);
GET_VALUE_h(value_int, int*);
GET_VALUE_h(value_uint, size_t*);
GET_VALUE_h(value_float, float*);

#define GET_ARRAY_VALUE_h(suffix, out_type)                                                        \
    Error get_array_##suffix(const JsonArray*, size_t, out_type);
GET_ARRAY_VALUE_h(value_char_p, const char**);
GET_ARRAY_VALUE_h(value_int, int*);
GET_ARRAY_VALUE_h(value_uint, size_t*);
GET_ARRAY_VALUE_h(value_float, float*);
GET_ARRAY_VALUE_h(value_child_p, JsonItem**);

// clang-format off
#define JsonObj_new(in_json, out_json)                                                             \
    _Generic(in_json,                                                                              \
        const char*  : JsonObj_new_from_char_p,                                                    \
        String*     : JsonObj_new_from_string_p                                                   \
        )(in_json, out_json)

#define Json_get(json_stuff, needle, out_p)                                                        \
    _Generic ((json_stuff),                                                                        \
        JsonItem*: _Generic((out_p),                                                               \
            const char** : get_value_char_p,                                                       \
            int*         : get_value_int,                                                          \
            size_t*      : get_value_uint,                                                         \
            float*       : get_value_float,                                                        \
            JsonItem**   : get_value_child_p,                                                      \
            JsonArray**  : get_value_array_p                                                       \
            ),                                                                                     \
        JsonArray*: _Generic((out_p),                                                              \
            const char** : get_array_value_char_p,                                                 \
            int*         : get_array_value_int,                                                    \
            size_t*      : get_array_value_uint,                                                   \
            float*       : get_array_value_float,                                                  \
            JsonItem**   : get_array_value_child_p,                                                \
            JsonArray**  : invalid_request                                                         \
            )                                                                                      \
        )(json_stuff, needle, out_p)
// clang-format on
#if TEST == 1
void test_class_json(void);
#endif

#endif
