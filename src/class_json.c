#include "class_json.h"
#include "converter.h"
#include "my_memory.h"
#include <stdio.h>
#include <string.h>

#define MAX_NUM_LEN (30)

// Used only for returning data in a convenient way. Not used for storage.
typedef struct JsonArray
{
    struct JsonItem* element;
} JsonArray;

typedef enum
{
    EMPTY,
    NUMBER,
    STRING,
    KEY,
    BOOLEAN_TRUE,
    BOOLEAN_FALSE,
    INVALID,
} ElementType;

bool is_token(const char c)
{
    char* token_list = "{}[],:\"";
    for (size_t i = 0; i < strlen(token_list); i++)
    {
        if (c == token_list[i])
            return true;
    }
    return false;
}

ElementType get_value_type(char* initial_char_p)
{
    if (strncmp(initial_char_p, "{\"", 2) == 0 || strncmp(initial_char_p, ",\"", 2) == 0)
    {
        return KEY;
    }
    else if (strncmp(initial_char_p, "{}", 2) == 0)
    {
        return EMPTY;
    }
    else if (initial_char_p[0] == '"')
    {
        return STRING;
    }
    else if (strncmp(initial_char_p, "true", 4) == 0)
    {
        return BOOLEAN_TRUE;
    }
    else if (strncmp(initial_char_p, "false", 5) == 0)
    {
        return BOOLEAN_FALSE;
    }
    else
    {
        return NUMBER;
    }
}

String strip_whitespace(const String* json_string_p)
{
    // The returned string cannot be longer than the input string (plus an termination char).
    char ret_cleaned_char_p[json_string_p->length + 1];

    size_t pos_out     = 0;
    bool inside_string = false;
    for (size_t pos_in = 0; pos_in < json_string_p->length; pos_in++)
    {
        char curr_char = json_string_p->str[pos_in];
        // "Open/Close" a string.
        if (curr_char == '\"')
        { // TODO: check how it works if the value contains an escaped '\"'
            inside_string = !inside_string;
        }
        // Ignore everything outside a string that is not printable or is a space (ascii 32).
        if (inside_string || (curr_char > 32))
        {
            ret_cleaned_char_p[pos_out++] = json_string_p->str[pos_in];
        }
    }
    ret_cleaned_char_p[pos_out] = '\0';
    return String_new(ret_cleaned_char_p);
}

char* terminate_str(char* char_p)
{
    while ((char_p != NULL) && (char_p + 1 != NULL))
    {
        if (*char_p == '"')
        {
            char_p++;
            if ((*char_p == ',') || (*char_p == '}') || (*char_p == ']') || (*char_p == ':'))
            {
                *(char_p - 1) = '\0';
                // return the position following the str termination
                return char_p;
            }
        }
        else
        {
            char_p++;
        }
    }
    return NULL;
}

String generate_tokens(String* json_string_p)
{
    char ret_tokens_char_p[json_string_p->length];

    size_t pos_out     = 0;
    bool inside_string = false;
    for (size_t pos_in = 0; pos_in < json_string_p->length; pos_in++)
    {
        char curr_char = json_string_p->str[pos_in];
        // "Open/Close" a string.
        if (curr_char == '\"')
        {
            inside_string = !inside_string;
        }
        // Ignore tokens found inside a string unless it's the '\"", hence end of the string.
        if ((is_token(curr_char) && !inside_string) || curr_char == '"')
        {
            ret_tokens_char_p[pos_out++] = json_string_p->str[pos_in];
        }
    }
    ret_tokens_char_p[pos_out] = '\0';

    return String_new(ret_tokens_char_p);
}

Error validate_tokens(char* json_char_p)
{
    Error ret_res        = ERR_ALL_GOOD;
    uint32_t obj_counter = 0;
    uint32_t arr_counter = 0;
    char curr_char;
    for (size_t index = 0; json_char_p[index] != 0; index++)
    {
        curr_char = json_char_p[index];
        if (curr_char == '[')
        {
            arr_counter++;
        }
        else if (curr_char == ']')
        {
            if (arr_counter > 0)
            {
                arr_counter--;
            }
            else
            {
                LOG_ERROR("Extra `]` detected");
                return ERR_JSON_INVALID;
            }
        }
        if (curr_char == '{')
        {
            obj_counter++;
        }
        else if (curr_char == '}')
        {
            if (obj_counter > 0)
            {
                obj_counter--;
            }
            else
            {
                LOG_ERROR("Extra `}` detected");
                return ERR_JSON_INVALID;
            }
        }
    }
    if (arr_counter)
    {
        LOG_ERROR("Missing `]` detected");
        return ERR_JSON_INVALID;
    }
    if (obj_counter)
    {
        LOG_ERROR("Missing `}` detected");
        return ERR_JSON_INVALID;
    }
    return ret_res;
}

Error deserialize(const char* file, const int line, JsonItem* curr_item_p, char** start_pos_p)
{
    char* curr_pos_p = *start_pos_p;
    bool parent_set  = false;
    Error ret_result = ERR_ALL_GOOD;
    while ((curr_pos_p[0] != '\0') && (curr_pos_p[1] != '\0'))
    {

        if ((curr_pos_p[0] == '}') || (curr_pos_p[0] == ']'))
        {
            // Use continue to make sure the next 2 chars are checked.
            curr_pos_p++;
            curr_item_p = curr_item_p->parent;
            continue;
        }

        if (curr_pos_p[0] == '[')
        {
            LOG_TRACE("Found beginning of array.");
            JsonItem* new_item     = (JsonItem*)my_memory_malloc(file, line, sizeof(JsonItem));
            new_item->index        = 0;
            new_item->key_p        = NULL;
            new_item->parent       = curr_item_p;
            new_item->next_sibling = NULL;
            curr_item_p->value.value_type    = VALUE_ARRAY;
            curr_item_p->value.value_child_p = new_item;
            curr_pos_p++;

            curr_item_p = new_item;
            continue;
        }

        if ((curr_item_p->parent->value.value_type == VALUE_ARRAY) && (*curr_pos_p == ','))
        {
            // This is a sibling of an array.
            LOG_TRACE("Found sibling in array.");
            JsonItem* new_item        = (JsonItem*)my_memory_malloc(file, line, sizeof(JsonItem));
            new_item->index           = curr_item_p->index + 1;
            new_item->key_p           = NULL;
            new_item->parent          = curr_item_p->parent;
            new_item->next_sibling    = NULL;
            curr_item_p->next_sibling = new_item;
            curr_pos_p++;
            curr_item_p = new_item;
            continue;
        }

        // If we are here, it's an item. If its parent is of type VALUE_ARRAY, we need to increment
        // the index, somehow.
        switch (get_value_type(curr_pos_p))
        {
        case NUMBER:
        {
            // 23 digits should be sufficient.
            char num_buff[MAX_NUM_LEN];
            // Try to convert into an integer or a float, depending on the presence of a dot ('.').
            bool dot_found = false;
            size_t i       = 0;
            // Create a substring containing the number.
            for (; (i < MAX_NUM_LEN - 1) && (*curr_pos_p != ',') && (*curr_pos_p != '}')
                   && (*curr_pos_p != ']');
                 i++)
            {
                if (*curr_pos_p == '.')
                {
                    dot_found = true;
                }
                num_buff[i] = *curr_pos_p;
                curr_pos_p++;
            }
            // Null terminate the string.
            num_buff[i] = '\0';
            if (dot_found)
            {
                float parsed_float = 0.0f;
                ret_result         = str_to_float(num_buff, &parsed_float);
                if (is_ok(ret_result))
                {
                    curr_item_p->value.value_type  = VALUE_FLOAT;
                    curr_item_p->value.value_float = parsed_float;
                    LOG_TRACE("Found value %f", curr_item_p->value.value_float);
                }
            }
            else if (num_buff[0] == '-')
            { // Convert into an integer if it is negative.
                int parsed_int = 0;
                ret_result     = str_to_int(num_buff, &parsed_int);
                if (is_ok(ret_result))
                {
                    curr_item_p->value.value_type = VALUE_INT;
                    curr_item_p->value.value_int  = parsed_int;
                    LOG_TRACE("Found value %d", curr_item_p->value.value_int);
                }
            }
            else
            {
                // Convert any positive value into a size_t.
                size_t parsed_size_t = 0;
                ret_result           = str_to_size_t(num_buff, &parsed_size_t);
                if (is_ok(ret_result))
                {
                    curr_item_p->value.value_type = VALUE_UINT;
                    curr_item_p->value.value_uint = parsed_size_t;
                    LOG_TRACE("Found value %lu", curr_item_p->value.value_uint);
                }
            }
            break;
        }
        case STRING:
        {
            curr_item_p->value.value_type   = VALUE_STR;
            curr_item_p->value.value_char_p = curr_pos_p + 1; // Point after the quote
            curr_pos_p                      = terminate_str(curr_pos_p);
            LOG_TRACE("Found value \"%s\"", curr_item_p->value.value_char_p);
            break;
        }
        case KEY:
        {
            if (*curr_pos_p == '{')
            {
                if (parent_set)
                {
                    // It's a child
                    LOG_TRACE("Found new object");
                    JsonItem* new_item = (JsonItem*)my_memory_malloc(file, line, sizeof(JsonItem));
                    new_item->next_sibling           = NULL;
                    new_item->parent                 = curr_item_p;
                    curr_item_p->value.value_type    = VALUE_ITEM;
                    curr_item_p->value.value_child_p = new_item;
                    curr_item_p                      = new_item;
                }
                else
                {
                    parent_set = true;
                }
            }
            else if (*curr_pos_p == ',')
            {
                // It's a sibling - the parent must be in common.
                JsonItem* new_item     = (JsonItem*)my_memory_malloc(file, line, sizeof(JsonItem));
                new_item->next_sibling = NULL;
                curr_item_p->next_sibling = new_item;
                new_item->parent          = curr_item_p->parent;
                curr_item_p               = new_item;
            }
            // Extract the key for the new item.
            curr_pos_p         = curr_pos_p + 2; // That's where the key starts
            curr_item_p->key_p = curr_pos_p;
            curr_pos_p         = terminate_str(curr_pos_p); // We should be at ':' now.
            LOG_TRACE("Found key: \"%s\"", curr_item_p->key_p);
            if (*curr_pos_p != ':')
            {
                printf("something bad happened\n");
                exit(ERR_FATAL);
            }
            curr_pos_p++; // Skip the ':'.
            break;
        }
        case BOOLEAN_TRUE:
        {
            curr_pos_p += 4;
            // Null terminate the string.
            curr_item_p->value.value_type = VALUE_BOOL;
            curr_item_p->value.value_bool = true;
            LOG_TRACE("Found value TRUE");
            break;
        }
        case BOOLEAN_FALSE:
        {
            curr_pos_p += 5;
            // Null terminate the string.
            curr_item_p->value.value_type = VALUE_BOOL;
            curr_item_p->value.value_bool = false;
            LOG_TRACE("Found value FALSE");
            break;
        }
        case EMPTY:
        {
            LOG_TRACE("Found empty object - skipping");
            curr_pos_p += 2;
            break;
        }
        case INVALID:
        {
            printf("To be handled\n");
            exit(3);
        }

        default:
            break;
        }
        if (is_err(ret_result))
        {
            return ERR_JSON_INVALID;
        }
    }
    return ERR_ALL_GOOD;
}

Error JsonObj_new_from_char_p(
    const char* file,
    const int line,
    const char* json_char_p,
    JsonObj* out_json_obj_p)
{
    String json_string = String_new(json_char_p);
    Error ret_result   = JsonObj_new_from_string_p(file, line, &json_string, out_json_obj_p);
    String_destroy(&json_string);
    return ret_result;
}

Error JsonObj_new_from_string_p(
    const char* file,
    const int line,
    const String* json_string_p,
    JsonObj* out_json_obj_p)
{
    if (json_string_p->length == 0)
    {
        LOG_ERROR("Empty JSON string detected");
        return ERR_EMPTY_STRING;
    }
    String trimmed_json_string = strip_whitespace(json_string_p);
    if ((trimmed_json_string.str[0] != '{') /*&& (*out_json_obj_pp->json_string.str[0] != '[')*/)
    {
        // TODO: Handle case in which the JSON string starts with [{ (array of objects).
        String_destroy(&trimmed_json_string);
        LOG_ERROR("Invalid JSON string.");
        return ERR_JSON_INVALID;
    }
    String tokens_string = generate_tokens(&trimmed_json_string);
    Error valid_json_res = validate_tokens(tokens_string.str);
    String_destroy(&tokens_string);
    if (is_err(valid_json_res))
    {
        String_destroy(&trimmed_json_string);
        LOG_ERROR("Invalid JSON string detected.");
        return valid_json_res;
    }
    String_destroy(&tokens_string);
    out_json_obj_p->json_string = trimmed_json_string;

    char* curr_pos_p = out_json_obj_p->json_string.str; // position analyzed (iterator)
    // Create a dummy root item as the entry point of the JSON object. The first actual item is the
    // first sibling of root. This prevents root's value type from being overwritten, hence causing
    // errors.
    out_json_obj_p->root.value.value_type = VALUE_ROOT;
    out_json_obj_p->root.parent
        = &out_json_obj_p->root; // Set the parent to itself to recognize 'root'.
    JsonItem* new_item                = (JsonItem*)my_memory_malloc(file, line, sizeof(JsonItem));
    new_item->value.value_type        = VALUE_UNDEFINED;
    new_item->next_sibling            = NULL;
    out_json_obj_p->root.next_sibling = new_item;
    new_item->parent                  = out_json_obj_p->root.parent;

    LOG_DEBUG("JSON deserialization started.");
    if (is_err(deserialize(file, line, out_json_obj_p->root.next_sibling, &curr_pos_p)))
    {
        JsonObj_destroy(out_json_obj_p);
        LOG_ERROR("Failed to deserialize JSON");
        return ERR_JSON_INVALID;
    }
    LOG_DEBUG("JSON deserialization ended successfully.")

    return ERR_ALL_GOOD;
}

void JsonItem_destroy(JsonItem* json_item)
{
    if (json_item == NULL)
    {
        return;
    }
    if (json_item->value.value_type != VALUE_UNDEFINED)
    {
        if (json_item->value.value_type == VALUE_ITEM)
        {
            JsonItem_destroy(json_item->value.value_child_p);
        }
        if (json_item->value.value_type == VALUE_ARRAY)
        {
            JsonItem_destroy(json_item->value.value_child_p);
        }
    }
    if (json_item->next_sibling != NULL)
    {
        JsonItem_destroy(json_item->next_sibling);
    }
    json_item->value.value_type = VALUE_UNDEFINED;
    if (json_item != json_item->parent)
    {
        my_memory_free(json_item);
    }
    json_item = NULL;
}

void JsonObj_destroy(JsonObj* json_obj_p)
{
    if (json_obj_p == NULL)
    {
        return;
    }
    if (json_obj_p->root.value.value_type != VALUE_UNDEFINED)
    {
        JsonItem_destroy(&json_obj_p->root);
    }
    String_destroy(&json_obj_p->json_string);
    json_obj_p = NULL;
}

#define OBJ_GET_VALUE_c(suffix, value_token, out_type, ACTION)                                     \
    Error obj_get_##suffix(const JsonObj* obj, const char* key, out_type out_value)                \
    {                                                                                              \
        return get_##suffix(obj->root.next_sibling, key, out_value);                               \
    }

#define GET_VALUE_c(suffix, value_token, out_type, ACTION)                                         \
    Error get_##suffix(const JsonItem* item, const char* key, out_type out_value)                  \
    {                                                                                              \
        if (item == NULL)                                                                          \
        {                                                                                          \
            *out_value = NULL;                                                                     \
            LOG_ERROR("Input item is NULL - key `%s`.", key);                                      \
            return ERR_JSON_MISSING_ENTRY;                                                         \
        }                                                                                          \
        if (!strcmp(item->key_p, key))                                                             \
        {                                                                                          \
            if (item->value.value_type == value_token)                                             \
            {                                                                                      \
                *out_value = item->value.suffix;                                                   \
                ACTION;                                                                            \
                return ERR_ALL_GOOD;                                                               \
            }                                                                                      \
            else                                                                                   \
            {                                                                                      \
                LOG_ERROR("Requested " #value_token " for a different value type.");               \
                return ERR_TYPE_MISMATCH;                                                          \
            }                                                                                      \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            return get_##suffix(item->next_sibling, key, out_value);                               \
        }                                                                                          \
    }

#define OBJ_GET_NUMBER_c(suffix, value_token, out_type, ACTION)                                    \
    Error obj_get_##suffix(const JsonObj* obj, const char* key, out_type out_value)                \
    {                                                                                              \
        return get_##suffix(obj->root.next_sibling, key, out_value);                               \
    }

#define GET_NUMBER_c(suffix, value_token, out_type, ACTION)                                        \
    Error get_##suffix(const JsonItem* item, const char* key, out_type out_value)                  \
    {                                                                                              \
        if (item == NULL)                                                                          \
        {                                                                                          \
            LOG_ERROR("Input item is NULL - key: `%s`.", key);                                     \
            return ERR_NULL;                                                                       \
        }                                                                                          \
        if (!strcmp(item->key_p, key))                                                             \
        {                                                                                          \
            if (item->value.value_type == value_token)                                             \
            {                                                                                      \
                *out_value = item->value.suffix;                                                   \
                ACTION;                                                                            \
                return ERR_ALL_GOOD;                                                               \
            }                                                                                      \
            else if ((item->value.value_type == VALUE_INT) && (value_token == VALUE_FLOAT))        \
            {                                                                                      \
                LOG_WARNING("Converting int to float");                                            \
                *out_value = (float)(1.0f * item->value.value_int);                                \
                ACTION;                                                                            \
                return ERR_ALL_GOOD;                                                               \
            }                                                                                      \
            else if ((item->value.value_type == VALUE_UINT) && (value_token == VALUE_FLOAT))       \
            {                                                                                      \
                LOG_WARNING("Converting size_t to float");                                         \
                *out_value = (float)(1.0f * item->value.value_uint);                               \
                ACTION;                                                                            \
                return ERR_ALL_GOOD;                                                               \
            }                                                                                      \
            else if ((item->value.value_type == VALUE_INT) && (value_token == VALUE_UINT))         \
            {                                                                                      \
                LOG_WARNING("Converting int to size_t");                                           \
                if (item->value.value_int < 0)                                                     \
                {                                                                                  \
                    LOG_ERROR(                                                                     \
                        "Impossible to convert negative int %d into size_t",                       \
                        item->value.value_int);                                                    \
                    LOG_ERROR("Failed to convert from INT to SIZE_T");                             \
                    return ERR_INVALID;                                                            \
                };                                                                                 \
                *out_value = (size_t)item->value.value_int;                                        \
                ACTION;                                                                            \
                return ERR_ALL_GOOD;                                                               \
            }                                                                                      \
            else if ((item->value.value_type == VALUE_UINT) && (value_token == VALUE_INT))         \
            {                                                                                      \
                LOG_WARNING("Converting size_t to int");                                           \
                *out_value = (int)item->value.value_uint;                                          \
                /* check for overflow */                                                           \
                if ((size_t)*out_value != item->value.value_uint)                                  \
                {                                                                                  \
                    LOG_ERROR(                                                                     \
                        "Overflow while converting %lu into an int", item->value.value_uint);      \
                    return ERR_INVALID;                                                            \
                };                                                                                 \
                ACTION;                                                                            \
                return ERR_ALL_GOOD;                                                               \
            }                                                                                      \
            else                                                                                   \
            {                                                                                      \
                LOG_ERROR("Requested " #value_token " for a different value type.")                \
                return ERR_TYPE_MISMATCH;                                                          \
            }                                                                                      \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            return get_##suffix(item->next_sibling, key, out_value);                               \
        }                                                                                          \
    }

#define GET_ARRAY_VALUE_c(suffix, value_token, out_type)                                           \
    Error get_array_##suffix(const JsonArray* json_array, size_t index, out_type out_value)        \
    {                                                                                              \
        if (json_array == NULL)                                                                    \
        {                                                                                          \
            LOG_ERROR("Input item is NULL");                                                       \
            return ERR_JSON_MISSING_ENTRY;                                                         \
        }                                                                                          \
        JsonItem* json_item = json_array->element;                                                 \
        while (true)                                                                               \
        {                                                                                          \
            if (json_item->index == index)                                                         \
            {                                                                                      \
                break;                                                                             \
            }                                                                                      \
            else if (json_item->next_sibling == NULL)                                              \
            {                                                                                      \
                LOG_WARNING("Index %lu out of boundaries.", index);                                \
                return ERR_NULL;                                                                   \
            }                                                                                      \
            json_item = json_item->next_sibling;                                                   \
        }                                                                                          \
        if (json_item->value.value_type != value_token)                                            \
        {                                                                                          \
            LOG_ERROR(                                                                             \
                "Incompatible data type - found %d, requested %d",                                 \
                json_item->value.value_type,                                                       \
                value_token);                                                                      \
            return ERR_TYPE_MISMATCH;                                                              \
        }                                                                                          \
        *out_value = json_item->value.suffix;                                                      \
        return ERR_ALL_GOOD;                                                                       \
    }

OBJ_GET_VALUE_c(value_char_p, VALUE_STR, const char**, );
OBJ_GET_VALUE_c(value_child_p, VALUE_ITEM, JsonItem**, );
OBJ_GET_VALUE_c(
    value_array_p,
    VALUE_ARRAY,
    JsonArray**,
    (*out_value)->element = item->value.value_child_p);

OBJ_GET_NUMBER_c(value_int, VALUE_INT, int*, );
OBJ_GET_NUMBER_c(value_uint, VALUE_UINT, size_t*, );
OBJ_GET_NUMBER_c(value_float, VALUE_FLOAT, float*, );
OBJ_GET_NUMBER_c(value_bool, VALUE_BOOL, bool*, );

GET_VALUE_c(value_char_p, VALUE_STR, const char**, );
GET_VALUE_c(value_child_p, VALUE_ITEM, JsonItem**, );
GET_VALUE_c(
    value_array_p,
    VALUE_ARRAY,
    JsonArray**,
    (*out_value)->element = item->value.value_child_p);

GET_NUMBER_c(value_int, VALUE_INT, int*, );
GET_NUMBER_c(value_uint, VALUE_UINT, size_t*, );
GET_NUMBER_c(value_float, VALUE_FLOAT, float*, );
GET_NUMBER_c(value_bool, VALUE_BOOL, bool*, );

GET_ARRAY_VALUE_c(value_char_p, VALUE_STR, const char**);
GET_ARRAY_VALUE_c(value_int, VALUE_INT, int*);
GET_ARRAY_VALUE_c(value_uint, VALUE_UINT, size_t*);
GET_ARRAY_VALUE_c(value_float, VALUE_FLOAT, float*);
GET_ARRAY_VALUE_c(value_bool, VALUE_BOOL, bool*);
GET_ARRAY_VALUE_c(value_child_p, VALUE_ITEM, JsonItem**);

#if TEST == 1

String load_file(char* filename)
{
    FILE* json_file = fopen(filename, "r");
    if (json_file == NULL)
    {
        LOG_PERROR("Could not read file");
    }
    int c;
    size_t chars_read = 0;
    size_t size       = 4096;
    char* buf         = my_memory_malloc(__FILE__, __LINE__, size);
    if (buf == NULL)
    {
        LOG_PERROR("out of memory");
        exit(1);
    }
    while ((c = fgetc(json_file)) != EOF)
    {
        if (chars_read >= size - 1)
        {
            /* time to make it bigger */
            size = (size_t)(size * 1.5);
            buf  = my_memory_realloc(__FILE__, __LINE__, buf, size);
            if (buf == NULL)
            {
                LOG_PERROR("out of memory");
                exit(1);
            }
        }
        buf[chars_read++] = c;
    }
    buf[chars_read++] = '\0';
    fclose(json_file);
    String json_string = String_new(buf);
    my_memory_free(buf);
    buf = NULL;
    return json_string;
}

void test_class_json()
{
    PRINT_BANNER();
    PRINT_TEST_TITLE("Validate tokens")
    {
        { // TODO: this should fail
            char json_char_p[] = "{[}]";
            ASSERT_OK(validate_tokens(json_char_p), "Valid JSON");
        }
        {
            char json_char_p[] = "{[][[]]{}{{}}}";
            ASSERT_OK(validate_tokens(json_char_p), "Valid JSON");
        }
        {
            char json_char_p[] = "{";
            ASSERT_ERR(validate_tokens(json_char_p), "Missing }.");
        }
        {
            char json_char_p[] = "}";
            ASSERT_ERR(validate_tokens(json_char_p), "Extra }.");
        }
        {
            char json_char_p[] = "[}";
            ASSERT_ERR(validate_tokens(json_char_p), "Extra }.");
        }
        {
            char json_char_p[] = "[";
            ASSERT_ERR(validate_tokens(json_char_p), "Missing ].");
        }
        {
            char json_char_p[] = "]";
            ASSERT_ERR(validate_tokens(json_char_p), "Extra ].");
        }
        {
            char json_char_p[] = "{]";
            ASSERT_ERR(validate_tokens(json_char_p), "Extra ].");
        }
    }
    PRINT_TEST_TITLE("Empty object")
    {
        JsonObj json_obj;
        const char* json_char_p = "{}";
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Empty JSON created");
        JsonObj_destroy(&json_obj);
    }
    PRINT_TEST_TITLE("Wrong object")
    {
        JsonObj json_obj;
        const char* json_char_p = "{:}";
        ASSERT_ERR(JsonObj_new(json_char_p, &json_obj), "Invalid JSON");
    }
    PRINT_TEST_TITLE("Empty nested object")
    {
        JsonObj json_obj;
        const char* value_str;
        const char* json_char_p = "{\"key\":{}}";
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Empty nested JSON");
        ASSERT_ERR(
            Json_get(&json_obj, "key", &value_str), "Key found but value cannot be retrieved");
        JsonObj_destroy(&json_obj);
    }
    PRINT_TEST_TITLE("Key-value pair");
    {
        JsonObj json_obj;
        const char* value_str;
        const char* json_char_p = " {\"key\": \"value string\"}";
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Json object created");
        Json_get(&json_obj, "key", &value_str);
        ASSERT_EQ("value string", value_str, "Key for root found with correct value STRING");
        ASSERT_ERR(Json_get(&json_obj, "missing key", &value_str), "Missing key detected.");
        ASSERT_EQ(value_str == NULL, true, "Returned null due to missing key.");
        JsonObj_destroy(&json_obj);
    }
    PRINT_TEST_TITLE("Sibling key-value pair");
    {
        JsonObj json_obj;
        const char* value_str;
        size_t value_uint;
        const char* json_char_p = " {\"key\": \"value string\", \"sibling\": 56}";
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Json object created");
        Json_get(&json_obj, "key", &value_str);
        Json_get(&json_obj, "sibling", &value_uint);
        ASSERT_EQ("value string", value_str, "Key for root value STRING");
        ASSERT_EQ(56, value_uint, "Key for root found with correct value INT");
        JsonObj_destroy(&json_obj);
    }
    PRINT_TEST_TITLE("Simple array");
    {
        JsonObj json_obj;
        const char* value_str;
        size_t value_uint;
        JsonArray* json_array;
        const char* json_char_p = " {\"key\": [\"array value\", 56]}";
        printf("\n%s\n", json_char_p);
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Json object created");
        Json_get(&json_obj, "key", &json_array);
        Json_get(json_array, 0, &value_str);
        ASSERT_EQ("array value", value_str, "Array STRING element retrieved.");
        Json_get(json_array, 1, &value_uint);
        ASSERT_EQ(56, value_uint, "Array SIZE_T element retrieved.");
        JsonObj_destroy(&json_obj);
    }
    PRINT_TEST_TITLE("Array of objects");
    {
        JsonObj json_obj;
        JsonItem* json_item;
        size_t value_uint;
        JsonArray* json_array;
        const char* json_char_p = " {\"key\": [ {\"array key\": 56}]}";
        printf("\n%s\n", json_char_p);
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Json object created");
        Json_get(&json_obj, "key", &json_array);
        Json_get(json_array, 0, &json_item);
        ASSERT_EQ(json_item->key_p, "array key", "Array STRING element retrieved.");
        printf("%d\n", json_item->value.value_type);
        Json_get(json_item, "array key", &value_uint);
        ASSERT_EQ(value_uint, 56, "Value found an item that is also array element.");
        JsonObj_destroy(&json_obj);
    }
    PRINT_TEST_TITLE("object and array");
    {
        JsonObj json_obj;
        JsonItem* json_item;
        JsonArray* json_array;
        const char* json_char_p = " {\"object\": {\"array\": [ {\"array key\": 56}]}}";
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Json object created");
        ASSERT_OK(Json_get(&json_obj, "object", &json_item), "Object retrieved");
        ASSERT_OK(Json_get(json_item, "array", &json_array), "Array retrieved");
        JsonObj_destroy(&json_obj);
        JsonObj_destroy(&json_obj);
    }
    PRINT_TEST_TITLE("test_json_array_1.json");
    {
        JsonObj json_obj;
        JsonItem* json_item;
        size_t value_uint;
        const char* value_str;
        float value_float;
        bool value_bool;
        JsonArray* json_array;
        String json_string = load_file("test/assets/test_json_array_1.json");
        ASSERT_OK(JsonObj_new(&json_string, &json_obj), "Json object created");
        Json_get(&json_obj, "array_key", &json_array);
        ASSERT_EQ(json_array != NULL, true, "Array found as root element.");
        Json_get(json_array, 0, &json_item);
        ASSERT_EQ(json_item != NULL, true, "First array element is an item.");
        Json_get(json_item, "object 1", &value_uint);
        ASSERT_EQ(value_uint, 56, "Value SIZE_T found");
        Json_get(json_array, 1, &json_item);
        ASSERT_EQ(json_item != NULL, true, "Second array element is an item.");
        Json_get(json_item, "object 2", &value_float);
        ASSERT_EQ(value_float, 404.5f, "Value FLOAT found");
        Json_get(json_array, 2, &json_item);
        Json_get(json_item, "object 3", &value_str);
        ASSERT_EQ(value_str, "SOME STRING", "Array element STRING found.");
        Json_get(json_array, 3, &value_uint);
        ASSERT_EQ(value_uint, 32, "Array element INT found.");
        Json_get(json_array, 4, &value_bool);
        ASSERT_EQ(value_bool, false, "Array element BOOL found.");
        JsonObj_destroy(&json_obj);
        String_destroy(&json_string);
    }
    PRINT_TEST_TITLE("test_json_array_2.json");
    {
        String json_string = load_file("test/assets/test_json_array_2.json");
        JsonItem* json_item;
        JsonObj json_obj;
        size_t value_uint;
        const char* value_str;
        float value_float;
        bool value_bool;
        JsonArray* json_array;
        JsonArray* json_array_2;
        ASSERT_OK(JsonObj_new(&json_string, &json_obj), "Json object created");
        Json_get(&json_obj, "array_key", &json_array);
        ASSERT_EQ(json_array != NULL, true, "Array found as root element.");
        Json_get(json_array, 0, &json_item);
        ASSERT_EQ(json_item != NULL, true, "First array element is an item.");
        Json_get(json_item, "inner array 1", &json_array_2);
        Json_get(json_array_2, 0, &value_uint);
        ASSERT_EQ(value_uint, 12314, "Value SIZE_T found");
        Json_get(json_array_2, 1, &value_float);
        ASSERT_EQ(value_float, -32.4f, "Value FLOAT found");
        Json_get(json_array_2, 2, &value_bool);
        ASSERT_EQ(value_bool, true, "Value TRUE found");
        Json_get(json_array, 1, &json_item);
        ASSERT_EQ(json_item != NULL, true, "Second array element is an item.");
        Json_get(json_item, "inner array 2", &json_array_2);
        Json_get(json_array_2, 0, &value_float);
        ASSERT_EQ(value_float, 1.4f, "Value FLOAT found");
        Json_get(json_array_2, 1, &value_str);
        ASSERT_EQ(value_str, "hello", "Value STRING found");
        Json_get(json_array_2, 2, &value_bool);
        ASSERT_EQ(value_bool, false, "Value FALSE found");
        JsonObj_destroy(&json_obj);
        String_destroy(&json_string);
    }
    PRINT_TEST_TITLE("test_json_array_3.json");
    {
        JsonObj json_obj;
        JsonItem* json_item;
        const char* value_str;
        size_t value_uint;
        JsonArray* json_array;
        String json_string = load_file("test/assets/test_json_array_3.json");
        ASSERT_OK(JsonObj_new(&json_string, &json_obj), "Json object created");
        ASSERT(Json_get(&json_obj, "Snapshot", &json_item) == ERR_ALL_GOOD, "Ok");
        ASSERT(Json_get(json_item, "Value", &value_uint) == ERR_ALL_GOOD, "Ok");
        ASSERT(Json_get(json_item, "Data", &json_array) == ERR_ALL_GOOD, "Ok");
        ASSERT(Json_get(json_array, 0, &json_item) == ERR_ALL_GOOD, "Ok");
        ASSERT(Json_get(json_item, "Time", &value_str) == ERR_ALL_GOOD, "Ok");
        ASSERT_EQ(value_str, "2021-07-23T08:09:00.000000Z", "Time correct.");
        JsonObj_destroy(&json_obj);
        String_destroy(&json_string);
    }
    PRINT_TEST_TITLE("test_json_array_4.json");
    {
        JsonObj json_obj;
        size_t value_uint;
        JsonArray* json_array;
        bool value_bool;
        String json_string = load_file("test/assets/test_json_array_4.json");
        ASSERT_OK(JsonObj_new(&json_string, &json_obj), "Json object created");
        ASSERT_OK(Json_get(&json_obj, "array_key1", &json_array), "Ok");
        ASSERT_OK(Json_get(json_array, 0, &value_uint), "Ok");
        ASSERT_EQ(value_uint, 32, "Ok");
        ASSERT_OK(Json_get(json_array, 1, &value_bool), "Ok");
        ASSERT_EQ(value_bool, false, "Ok");
        ASSERT_OK(Json_get(&json_obj, "array_key2", &json_array), "Ok");
        ASSERT_OK(Json_get(json_array, 0, &value_uint), "Ok");
        ASSERT_EQ(value_uint, 33, "Ok");
        ASSERT_OK(Json_get(json_array, 1, &value_bool), "Ok");
        ASSERT_EQ(value_bool, true, "Ok");
        ASSERT_OK(Json_get(&json_obj, "key", &value_uint), "Ok");
        ASSERT_EQ(value_uint, 34, "Ok");
        JsonObj_destroy(&json_obj);
        String_destroy(&json_string);
    }
    PRINT_TEST_TITLE("Testing test/assets/test_json.json");
    {
        JsonObj json_obj;
        JsonItem* json_item;
        const char* value_str;
        size_t value_uint;
        float value_float;
        bool value_bool;
        JsonArray* json_array;
        String json_string = load_file("test/assets/test_json.json");
        ASSERT_OK(JsonObj_new(&json_string, &json_obj), "Json object created");
        String_destroy(&json_string); // We can delete it.
        Json_get(&json_obj, "text_key", &value_str);
        ASSERT_EQ("text_value", value_str, "String*value found in first item");

        Json_get(&json_obj, "text_sibling", &value_str);
        ASSERT_EQ("sibling_value", value_str, "String*value found in sibling");

        Json_get(&json_obj, "nested_1", &json_item);
        ASSERT_EQ(json_item->key_p, "object_1.1", "Found nested object key");

        Json_get(json_item, "object_1.1", &value_str);
        ASSERT_EQ(value_str, "item_1.1", "Found nested object value");
        Json_get(json_item, "object_1.2", &value_str);
        ASSERT_EQ(value_str, "item_1.2", "Found nested sibling object value");
        ASSERT(
            Json_get(json_item, "object_32", &value_str) == ERR_JSON_MISSING_ENTRY,
            "Object not found");
        ASSERT(value_str == NULL, "Null returned.");

        Json_get(&json_obj, "nested_2", &json_item);
        Json_get(json_item, "object_2.1", &value_str);
        ASSERT_EQ(value_str, "item_2.1", "Found nested object value");
        Json_get(json_item, "object_2.2", &json_item);
        ASSERT_EQ(json_item->key_p, "item_2.2", "Found nested object key");
        Json_get(json_item, "item_2.2", &value_str);
        ASSERT_EQ(value_str, "value_2.2.1", "Found nested sibling object value");

        PRINT_TEST_TITLE("Test integer");
        Json_get(&json_obj, "test_integer", &value_uint);
        ASSERT_EQ(value_uint, 435234, "Integer found and read correctly");

        PRINT_TEST_TITLE("Test float");
        Json_get(&json_obj, "test_float", &value_float);
        ASSERT_EQ(value_float, 435.234f, "Float found and read correctly");

        PRINT_TEST_TITLE("Test bool true");
        Json_get(&json_obj, "test_bool_true", &value_bool);
        ASSERT_EQ(value_bool, true, "Boolean found and read correctly");

        PRINT_TEST_TITLE("Test bool false");
        Json_get(&json_obj, "test_bool_false", &value_bool);
        ASSERT_EQ(value_bool, false, "Boolean found and read correctly");

        Json_get(&json_obj, "test_array", &json_array);
        Json_get(json_array, 0, &value_uint);
        ASSERT_EQ(value_uint, 14352, "Array element of type INT read correctly");
        Json_get(json_array, 1, &value_float);
        ASSERT_EQ(value_float, 2.15f, "Array element of type FLOAT read correctly");
        Json_get(json_array, 2, &value_str);
        ASSERT_EQ(value_str, "string_element", "Array element of type C-string read correctly");
        JsonObj_destroy(&json_obj);
    }
    PRINT_TEST_TITLE("Invalid JSON string - empty")
    {
        JsonObj json_obj;
        String json_string = String_new("");
        ASSERT(
            JsonObj_new(&json_string, &json_obj) == ERR_EMPTY_STRING,
            "Empty JSON fails to initialize.");
        String_destroy(&json_string);
    }
    PRINT_TEST_TITLE("Invalid JSON string - string not starting with '{'")
    { // TODO: crate token analyzer and add TC's.
        JsonObj json_obj;
        String json_string = String_new("This is not a JSON file");
        ASSERT(
            JsonObj_new(&json_string, &json_obj) == ERR_JSON_INVALID,
            "Invalid JSON fails to initialize.");
        String_destroy(&json_string);
    }
    PRINT_TEST_TITLE("Fixed memory leak");
    {
        JsonArray* json_array;
        const char* value_str;
        const char* json_char_p = "{\"request\":[\"Required parameter is missing\"]}";
        JsonObj json_with_vector_obj;
        ASSERT(is_ok(JsonObj_new(json_char_p, &json_with_vector_obj)), "Json object created");
        Json_get(&json_with_vector_obj, "request", &json_array);
        Json_get(json_array, 0, &value_str);
        ASSERT_EQ(value_str, "Required parameter is missing", "");
        JsonObj_destroy(&json_with_vector_obj);
    }
    PRINT_TEST_TITLE("Data conversion");
    {
        JsonObj json_obj;
        size_t value_uint;
        int value_int;
        Error ret_res;
        String json_string = load_file("test/assets/test_json_numbers.json");
        ASSERT_OK(JsonObj_new(&json_string, &json_obj), "Json object created");
        Json_get(&json_obj, "value_int", &value_uint);
        ASSERT_EQ((size_t)23, value_uint, "Conversion from INT to SIZE_T successfull");
        Json_get(&json_obj, "value_small_uint", &value_int);
        ASSERT_EQ((int)43, value_int, "Conversion from SIZE_T to INT successfull");
        ret_res = Json_get(&json_obj, "value_negative_int", &value_uint);
        ASSERT(ret_res == ERR_INVALID, "Conversion from negative INT to SIZE_T failed");
        ret_res = Json_get(&json_obj, "value_uint", &value_int);
        ASSERT(ret_res == ERR_INVALID, "Conversion from large SIZE_T to INT failed");
        JsonObj_destroy(&json_obj);
        String_destroy(&json_string);
    }
    /**/
}
#endif
