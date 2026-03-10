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

JsonItem* _JsonItem_new(const char* file, const int line)
{
    JsonItem* new_item         = (JsonItem*)my_memory_malloc(file, line, sizeof(JsonItem));
    new_item->key_p            = NULL;
    new_item->index            = 0;
    new_item->value.value_type = VALUE_UNDEFINED;
    new_item->parent           = NULL;
    new_item->next_sibling     = NULL;
    return new_item;
}

bool _is_token(const char c)
{
    char* token_list = "{}[],:\"";
    for (llu_t i = 0; i < strlen(token_list); i++)
    {
        if (c == token_list[i])
            return true;
    }
    return false;
}

ElementType _get_value_type(char* initial_char_p)
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

static char* _strip_whitespace_malloc(const char* json_cstr)
{
    // The returned string cannot be longer than the input string (plus an termination char).
    char* ret_str;
    const size_t str_len = strlen(json_cstr);
    ret_str              = my_memory_malloc(__FILENAME__, __LINE__, str_len + 1);

    size_t pos_out     = 0;
    bool inside_string = false;
    for (size_t pos_in = 0; pos_in < str_len; pos_in++)
    {
        char curr_char = json_cstr[pos_in];
        // "Open/Close" a string.
        if (curr_char == '\"')
        { // TODO: check how it works if the value contains an escaped '\"'
            inside_string = !inside_string;
        }
        // Ignore everything outside a string that is not printable or is a space (ascii 32).
        if (inside_string || (curr_char > 32))
        {
            ret_str[pos_out++] = json_cstr[pos_in];
        }
    }
    ret_str[pos_out] = '\0';
    return ret_str;
}

static char* _terminate_str(char* char_p)
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

static char* _generate_tokens_malloc(char* json_cstr)
{
    char* ret_str;
    size_t str_len     = strlen(json_cstr);
    ret_str            = my_memory_malloc(__FILENAME__, __LINE__, str_len + 1);
    size_t pos_out     = 0;
    bool inside_string = false;
    for (size_t pos_in = 0; pos_in < str_len; pos_in++)
    {
        char curr_char = json_cstr[pos_in];
        // "Open/Close" a string.
        if (curr_char == '\"')
        {
            inside_string = !inside_string;
        }
        // Ignore tokens found inside a string unless it's the '\"", hence end of the string.
        if ((_is_token(curr_char) && !inside_string) || curr_char == '"')
        {
            ret_str[pos_out++] = json_cstr[pos_in];
        }
    }
    ret_str[pos_out] = '\0';
    return ret_str;
}

static Error _validate_tokens(char* json_char_p)
{
    Error ret_res                  = ERR_ALL_GOOD;
    unsigned long long obj_counter = 0;
    unsigned long long arr_counter = 0;
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

Error _deserialize(const char* file, const int line, JsonItem* curr_item_p, char** start_pos_p)
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
            JsonItem* new_item               = _JsonItem_new(file, line);
            new_item->parent                 = curr_item_p;
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
            JsonItem* new_item        = _JsonItem_new(file, line);
            new_item->index           = curr_item_p->index + 1;
            new_item->parent          = curr_item_p->parent;
            curr_item_p->next_sibling = new_item;
            curr_pos_p++;
            curr_item_p = new_item;
            continue;
        }
        // If we are here, it's an item. If its parent is of type VALUE_ARRAY, we need to increment
        // the index, somehow.
        switch (_get_value_type(curr_pos_p))
        {
        case NUMBER:
        {
            // 23 digits should be sufficient.
            char num_buff[MAX_NUM_LEN];
            // Try to convert into an integer or a double, depending on the presence of a dot ('.').
            bool dot_found = false;
            llu_t i        = 0;
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
                double parsed_double = 0.0f;
                ret_result           = numparser_cstr_to_double(num_buff, &parsed_double, '\0');
                if (is_ok(ret_result))
                {
                    curr_item_p->value.value_type   = VALUE_DOUBLE;
                    curr_item_p->value.value_double = parsed_double;
                    LOG_TRACE("Found value %lf", curr_item_p->value.value_double);
                }
            }
            else if (num_buff[0] == '-')
            { // Convert into an integer if it is negative.
                lld_t parsed_lld = 0;
                ret_result       = numparser_cstr_to_lld(num_buff, &parsed_lld, '\0');
                if (is_ok(ret_result))
                {
                    curr_item_p->value.value_type = VALUE_LLD;
                    curr_item_p->value.value_lld  = parsed_lld;
                    LOG_TRACE("Found value %lld", parsed_lld);
                }
            }
            else
            {
                // Convert any positive value into a size_t.
                llu_t parsed_llu = 0;
                ret_result       = numparser_cstr_to_llu(num_buff, &parsed_llu, '\0');
                if (is_ok(ret_result))
                {
                    curr_item_p->value.value_type = VALUE_LLU;
                    curr_item_p->value.value_llu  = parsed_llu;
                    LOG_TRACE("Found LLU value %llu", parsed_llu);
                }
            }
            break;
        }
        case STRING:
        {
            curr_item_p->value.value_type = VALUE_CSTR;
            curr_item_p->value.value_cstr = curr_pos_p + 1; // Point after the quote
            curr_pos_p                    = _terminate_str(curr_pos_p);
            LOG_TRACE("Found value \"%s\"", curr_item_p->value.value_cstr);
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
                    JsonItem* new_item               = _JsonItem_new(file, line);
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
                JsonItem* new_item        = _JsonItem_new(file, line);
                curr_item_p->next_sibling = new_item;
                new_item->parent          = curr_item_p->parent;
                curr_item_p               = new_item;
            }
            // Extract the key for the new item.
            curr_pos_p         = curr_pos_p + 2; // That's where the key starts
            curr_item_p->key_p = curr_pos_p;
            curr_pos_p         = _terminate_str(curr_pos_p); // We should be at ':' now.
            LOG_TRACE("Found key: \"%s\"", curr_item_p->key_p);
            if (*curr_pos_p != ':')
            {
                LOG_ERROR("Something bad happened");
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
            LOG_ERROR("TODO: handle this");
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

Error _JsonObj_new(
    const char* file,
    const int line,
    const char* json_cstr,
    JsonObj* out_json_obj_p)
{
    Error ret_err                      = ERR_ALL_GOOD;
    char* trimmed_json_cstr            = NULL;
    char* curr_pos_p                   = NULL;
    __autofree_cstr__ char* token_cstr = NULL;
    if (strlen(json_cstr) == 0)
    {
        LOG_ERROR("Empty JSON string detected");
        return ERR_EMPTY_STRING;
    }
    trimmed_json_cstr = _strip_whitespace_malloc(json_cstr);
    if ((trimmed_json_cstr[0] != '{') /*&& (*out_json_obj_pp->json_cstr.str[0] != '[')*/)
    {
        // TODO: Handle case in which the JSON string starts with [{ (array of objects).
        LOG_ERROR("Invalid JSON string.");
        my_memory_free(trimmed_json_cstr);
        return ERR_JSON_INVALID;
    }
    token_cstr = _generate_tokens_malloc(trimmed_json_cstr);
    ret_err    = _validate_tokens(token_cstr);
    if (is_err(ret_err))
    {
        my_memory_free(trimmed_json_cstr);
        LOG_ERROR("Invalid JSON string detected.");
        return ret_err;
    }
    out_json_obj_p->json_cstr = trimmed_json_cstr;

    curr_pos_p = out_json_obj_p->json_cstr; // position analyzed (iterator)
    // Create a dummy root item as the entry point of the JSON object. The first actual item is the
    // first sibling of root. This prevents root's value type from being overwritten, hence causing
    // errors.
    out_json_obj_p->root.value.value_type = VALUE_ROOT;
    out_json_obj_p->root.parent
        = &out_json_obj_p->root; // Set the parent to itself to recognize 'root'.
    JsonItem* new_item                = _JsonItem_new(file, line);
    out_json_obj_p->root.next_sibling = new_item;
    new_item->parent                  = out_json_obj_p->root.parent;

    LOG_DEBUG("JSON deserialization started.");
    if (is_err(_deserialize(file, line, out_json_obj_p->root.next_sibling, &curr_pos_p)))
    {
        JsonObj_destroy(out_json_obj_p);
        LOG_ERROR("Failed to deserialize JSON");
        return ERR_JSON_INVALID;
    }
    LOG_DEBUG("JSON deserialization ended successfully.")

    return ERR_ALL_GOOD;
}

void _JsonItem_destroy(JsonItem* json_item)
{
    if (json_item == NULL)
    {
        return;
    }
    if (json_item->value.value_type != VALUE_UNDEFINED)
    {
        if (json_item->value.value_type == VALUE_ITEM)
        {
            _JsonItem_destroy(json_item->value.value_child_p);
        }
        if (json_item->value.value_type == VALUE_ARRAY)
        {
            _JsonItem_destroy(json_item->value.value_child_p);
        }
    }
    if (json_item->next_sibling != NULL)
    {
        _JsonItem_destroy(json_item->next_sibling);
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
        _JsonItem_destroy(&json_obj_p->root);
    }
    my_memory_free(json_obj_p->json_cstr);
    json_obj_p->json_cstr = NULL;
    json_obj_p            = NULL;
}

#define OBJ_GET_VALUE_c(suffix, value_token, out_type, ACTION)                      \
    Error obj_get_##suffix(const JsonObj* obj, const char* key, out_type out_value) \
    {                                                                               \
        if (obj)                                                                    \
        {                                                                           \
            return get_##suffix(obj->root.next_sibling, key, out_value);            \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            return ERR_NULL;                                                        \
        }                                                                           \
    }

#define GET_VALUE_c(suffix, value_token, out_type, ACTION)                           \
    Error get_##suffix(const JsonItem* item, const char* key, out_type out_value)    \
    {                                                                                \
        if (item == NULL)                                                            \
        {                                                                            \
            *out_value = NULL;                                                       \
            LOG_ERROR("Input item is NULL - key `%s`.", key);                        \
            return ERR_JSON_MISSING_ENTRY;                                           \
        }                                                                            \
        if (!item->key_p)                                                            \
        {                                                                            \
            return ERR_NULL;                                                         \
        }                                                                            \
        if (!strcmp(item->key_p, key))                                               \
        {                                                                            \
            if (item->value.value_type == value_token)                               \
            {                                                                        \
                *out_value = item->value.suffix;                                     \
                ACTION;                                                              \
                return ERR_ALL_GOOD;                                                 \
            }                                                                        \
            else                                                                     \
            {                                                                        \
                LOG_ERROR("Requested " #value_token " for a different value type."); \
                return ERR_TYPE_MISMATCH;                                            \
            }                                                                        \
        }                                                                            \
        else                                                                         \
        {                                                                            \
            return get_##suffix(item->next_sibling, key, out_value);                 \
        }                                                                            \
    }

#define OBJ_GET_NUMBER_c(suffix, value_token, out_type, ACTION)                     \
    Error obj_get_##suffix(const JsonObj* obj, const char* key, out_type out_value) \
    {                                                                               \
        return get_##suffix(obj->root.next_sibling, key, out_value);                \
    }

#define GET_NUMBER_c(suffix, value_token, out_type, ACTION)                                   \
    Error get_##suffix(const JsonItem* item, const char* key, out_type out_value)             \
    {                                                                                         \
        if (item == NULL)                                                                     \
        {                                                                                     \
            LOG_ERROR("Input item is NULL - key: `%s`.", key);                                \
            return ERR_NULL;                                                                  \
        }                                                                                     \
        if (!item->key_p)                                                                     \
        {                                                                                     \
            return ERR_NULL;                                                                  \
        }                                                                                     \
        if (!strcmp(item->key_p, key))                                                        \
        {                                                                                     \
            if (item->value.value_type == value_token)                                        \
            {                                                                                 \
                *out_value = item->value.suffix;                                              \
                ACTION;                                                                       \
                return ERR_ALL_GOOD;                                                          \
            }                                                                                 \
            else if ((item->value.value_type == VALUE_LLD) && (value_token == VALUE_DOUBLE))  \
            {                                                                                 \
                LOG_WARNING("Converting int to double");                                      \
                *out_value = (double)(1.0 * item->value.value_lld);                           \
                ACTION;                                                                       \
                return ERR_ALL_GOOD;                                                          \
            }                                                                                 \
            else if ((item->value.value_type == VALUE_LLU) && (value_token == VALUE_DOUBLE))  \
            {                                                                                 \
                LOG_WARNING("Converting size_t to double");                                   \
                *out_value = (double)(1.0 * item->value.value_llu);                           \
                ACTION;                                                                       \
                return ERR_ALL_GOOD;                                                          \
            }                                                                                 \
            else if ((item->value.value_type == VALUE_LLD) && (value_token == VALUE_LLU))     \
            {                                                                                 \
                LOG_WARNING("Converting int to size_t");                                      \
                if (item->value.value_lld < 0)                                                \
                {                                                                             \
                    LOG_ERROR(                                                                \
                        "Impossible to convert negative int %lld into size_t",                \
                        item->value.value_lld);                                               \
                    LOG_ERROR("Failed to convert from INT to LLU");                           \
                    return ERR_INVALID;                                                       \
                };                                                                            \
                *out_value = (llu_t)item->value.value_lld;                                    \
                ACTION;                                                                       \
                return ERR_ALL_GOOD;                                                          \
            }                                                                                 \
            else if ((item->value.value_type == VALUE_LLU) && (value_token == VALUE_LLD))     \
            {                                                                                 \
                LOG_WARNING("Converting size_t to int");                                      \
                *out_value = (lld_t)item->value.value_llu;                                    \
                /* check for overflow */                                                      \
                if (*out_value < 0)                                                           \
                {                                                                             \
                    LOG_ERROR(                                                                \
                        "Overflow while converting %llu into an lld", item->value.value_llu); \
                    return ERR_INVALID;                                                       \
                };                                                                            \
                ACTION;                                                                       \
                return ERR_ALL_GOOD;                                                          \
            }                                                                                 \
            else                                                                              \
            {                                                                                 \
                LOG_ERROR("Requested " #value_token " for a different value type.")           \
                return ERR_TYPE_MISMATCH;                                                     \
            }                                                                                 \
        }                                                                                     \
        else                                                                                  \
        {                                                                                     \
            return get_##suffix(item->next_sibling, key, out_value);                          \
        }                                                                                     \
    }

#define GET_ARRAY_VALUE_c(suffix, value_token, out_type)                                    \
    Error get_array_##suffix(const JsonArray* json_array, size_t index, out_type out_value) \
    {                                                                                       \
        if (json_array == NULL)                                                             \
        {                                                                                   \
            LOG_ERROR("Input item is NULL");                                                \
            return ERR_JSON_MISSING_ENTRY;                                                  \
        }                                                                                   \
        JsonItem* json_item = json_array->element;                                          \
        while (true)                                                                        \
        {                                                                                   \
            if (json_item->index == index)                                                  \
            {                                                                               \
                break;                                                                      \
            }                                                                               \
            else if (json_item->next_sibling == NULL)                                       \
            {                                                                               \
                LOG_WARNING("Index %lu out of boundaries.", index);                         \
                return ERR_NULL;                                                            \
            }                                                                               \
            json_item = json_item->next_sibling;                                            \
        }                                                                                   \
        if (json_item->value.value_type != value_token)                                     \
        {                                                                                   \
            LOG_ERROR(                                                                      \
                "Incompatible data type - found %d, requested %d",                          \
                json_item->value.value_type,                                                \
                value_token);                                                               \
            return ERR_TYPE_MISMATCH;                                                       \
        }                                                                                   \
        *out_value = json_item->value.suffix;                                               \
        return ERR_ALL_GOOD;                                                                \
    }

// clang-format off
OBJ_GET_VALUE_c(value_cstr, VALUE_CSTR, const char**, )
OBJ_GET_VALUE_c(value_child_p, VALUE_ITEM, JsonItem**, )
OBJ_GET_VALUE_c(
    value_array_p,
    VALUE_ARRAY,
    JsonArray**,
    (*out_value)->element = item->value.value_child_p)

OBJ_GET_NUMBER_c(value_lld, VALUE_LLD, lld_t*, )
OBJ_GET_NUMBER_c(value_llu, VALUE_LLU, llu_t*, )
OBJ_GET_NUMBER_c(value_double, VALUE_DOUBLE, double*, )
OBJ_GET_NUMBER_c(value_bool, VALUE_BOOL, bool*, )

GET_VALUE_c(value_cstr, VALUE_CSTR, const char**, )
GET_VALUE_c(value_child_p, VALUE_ITEM, JsonItem**, )
GET_VALUE_c(
    value_array_p,
    VALUE_ARRAY,
    JsonArray**,
    (*out_value)->element = item->value.value_child_p)

GET_NUMBER_c(value_lld, VALUE_LLD, lld_t*, )
GET_NUMBER_c(value_llu, VALUE_LLU, llu_t *, )
GET_NUMBER_c(value_double, VALUE_DOUBLE, double*, )
GET_NUMBER_c(value_bool, VALUE_BOOL, bool*, )

GET_ARRAY_VALUE_c(value_cstr, VALUE_CSTR, const char**)
GET_ARRAY_VALUE_c(value_lld, VALUE_LLD, lld_t*)
GET_ARRAY_VALUE_c(value_llu, VALUE_LLU, llu_t*)
GET_ARRAY_VALUE_c(value_double, VALUE_DOUBLE, double*)
GET_ARRAY_VALUE_c(value_bool, VALUE_BOOL, bool*)
GET_ARRAY_VALUE_c(value_child_p, VALUE_ITEM, JsonItem**)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-semi"
; // ensure clang-format works when turned on again
#pragma clang diagnostic pop
                                                                                  // clang-format on

#ifdef _TEST
static char* load_file_alloc(char* filename)
{
    FILE* json_file = fopen(filename, "r");
    if (json_file == NULL)
    {
        LOG_PERROR("Could not read file");
    }
    int c;
    size_t chars_read = 0;
    size_t size       = 4096;
    char* ret_str     = my_memory_malloc(__FILENAME__, __LINE__, size);
    if (ret_str == NULL)
    {
        LOG_PERROR("out of memory");
        exit(1);
    }
    while ((c = fgetc(json_file)) != EOF)
    {
        if (chars_read >= size - 1)
        {
            /* time to make it bigger */
            size    = (size_t)(size * 1.5);
            ret_str = realloc(ret_str, size);
            if (ret_str == NULL)
            {
                LOG_PERROR("out of memory");
                exit(1);
            }
        }
        ret_str[chars_read++] = c;
    }
    ret_str[chars_read++] = '\0';
    fclose(json_file);
    return ret_str;
}

void test_class_json(void)
{
    PRINT_BANNER();
    PRINT_TEST_TITLE("Validate tokens");
    {
        { // TODO: this should fail
            char json_char_p[] = "{[}]";
            ASSERT_OK(_validate_tokens(json_char_p), "Valid JSON");
        }
        {
            char json_char_p[] = "{[][[]]{}{{}}}";
            ASSERT_OK(_validate_tokens(json_char_p), "Valid JSON");
        }
        {
            char json_char_p[] = "{";
            ASSERT_ERR(_validate_tokens(json_char_p), "Missing }.");
        }
        {
            char json_char_p[] = "}";
            ASSERT_ERR(_validate_tokens(json_char_p), "Extra }.");
        }
        {
            char json_char_p[] = "[}";
            ASSERT_ERR(_validate_tokens(json_char_p), "Extra }.");
        }
        {
            char json_char_p[] = "[";
            ASSERT_ERR(_validate_tokens(json_char_p), "Missing ].");
        }
        {
            char json_char_p[] = "]";
            ASSERT_ERR(_validate_tokens(json_char_p), "Extra ].");
        }
        {
            char json_char_p[] = "{]";
            ASSERT_ERR(_validate_tokens(json_char_p), "Extra ].");
        }
    }
    PRINT_TEST_TITLE("Empty object");
    {
        __autodestroy_json__ JsonObj json_obj;
        JsonItem* json_item_p;
        const char* json_char_p = "{}";
        lld_t a;
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Empty JSON created");
        ASSERT_ERR(Json_get(&json_obj, "missing key", &json_item_p), "Fix NULL on key");
        ASSERT_ERR(Json_get(&json_obj, "missing key", &a), "Fix NULL on key");
    }
    PRINT_TEST_TITLE("Wrong object");
    {
        JsonObj json_obj;
        const char* json_char_p = "{:}";
        ASSERT_ERR(JsonObj_new(json_char_p, &json_obj), "Invalid JSON");
        // This does not cause an error, even though the object was not created because of parsing errors.
        JsonObj_destroy(&json_obj);
    }
    PRINT_TEST_TITLE("Empty nested object");
    {
        __autodestroy_json__ JsonObj json_obj;
        const char* value_cstr;
        const char* json_char_p = "{\"key\":{}}";
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Empty nested JSON");
        ASSERT_ERR(Json_get(&json_obj, "key", &value_cstr), "Key found but value cannot be retrieved");
    }
    PRINT_TEST_TITLE("Key-value pair");
    {
        __autodestroy_json__ JsonObj json_obj;
        const char* value_cstr;
        const char* json_char_p = " {\"key\": \"value string\"}";
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Json object created");
        Json_get(&json_obj, "key", &value_cstr);
        ASSERT_EQ("value string", value_cstr, "Key for root found with correct value STRING");
        ASSERT_ERR(Json_get(&json_obj, "missing key", &value_cstr), "Missing key detected.");
        ASSERT_EQ(value_cstr == NULL, true, "Returned null due to missing key.");
    }
    PRINT_TEST_TITLE("Sibling key-value pair");
    {
        __autodestroy_json__ JsonObj json_obj;
        const char* value_cstr;
        llu_t value_llu;
        const char* json_char_p = " {\"key\": \"value string\", \"sibling\": 56}";
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Json object created");
        Json_get(&json_obj, "key", &value_cstr);
        Json_get(&json_obj, "sibling", &value_llu);
        ASSERT_EQ("value string", value_cstr, "Key for root value STRING");
        ASSERT_EQ(56, value_llu, "Key for root found with correct value INT");
    }
    PRINT_TEST_TITLE("Simple array");
    {
        __autodestroy_json__ JsonObj json_obj;
        const char* value_cstr;
        llu_t value_llu;
        JsonArray* json_array;
        const char* json_char_p = " {\"key\": [\"array value\", 56]}";
        printf("\n%s\n", json_char_p);
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Json object created");
        Json_get(&json_obj, "key", &json_array);
        Json_get(json_array, 0, &value_cstr);
        ASSERT_EQ("array value", value_cstr, "Array STRING element retrieved.");
        Json_get(json_array, 1, &value_llu);
        ASSERT_EQ(56, value_llu, "Array LLU element retrieved.");
    }
    PRINT_TEST_TITLE("Array of objects");
    {
        __autodestroy_json__ JsonObj json_obj;
        JsonItem* json_item;
        llu_t value_llu;
        JsonArray* json_array;
        const char* json_char_p = " {\"key\": [ {\"array key\": 56}]}";
        printf("\n%s\n", json_char_p);
        ASSERT_OK(JsonObj_new(json_char_p, &json_obj), "Json object created");
        Json_get(&json_obj, "key", &json_array);
        Json_get(json_array, 0, &json_item);
        ASSERT_EQ(json_item->key_p, "array key", "Array STRING element retrieved.");
        printf("%d\n", json_item->value.value_type);
        Json_get(json_item, "array key", &value_llu);
        ASSERT_EQ(value_llu, 56, "Value found an item that is also array element.");
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
        // Destroying multiple times is safe
        JsonObj_destroy(&json_obj);
        JsonObj_destroy(&json_obj);
    }
    PRINT_TEST_TITLE("test_json_array_1.json");
    {
        JsonObj json_obj;
        JsonItem* json_item;
        llu_t value_llu;
        const char* value_cstr;
        double value_double;
        bool value_bool;
        JsonArray* json_array;
        __autofree_cstr__ char* json_cstr = load_file_alloc("test/assets/test_json_array_1.json");
        ASSERT_OK(JsonObj_new(json_cstr, &json_obj), "Json object created");
        Json_get(&json_obj, "array_key", &json_array);
        ASSERT_EQ(json_array != NULL, true, "Array found as root element.");
        Json_get(json_array, 0, &json_item);
        ASSERT_EQ(json_item != NULL, true, "First array element is an item.");
        Json_get(json_item, "object 1", &value_llu);
        ASSERT_EQ(value_llu, 56, "Value LLU found");
        Json_get(json_array, 1, &json_item);
        ASSERT_EQ(json_item != NULL, true, "Second array element is an item.");
        Json_get(json_item, "object 2", &value_double);
        ASSERT_EQ(value_double, 404.5f, "Value DOUBLE found");
        Json_get(json_array, 2, &json_item);
        Json_get(json_item, "object 3", &value_cstr);
        ASSERT_EQ(value_cstr, "SOME STRING", "Array element STRING found.");
        Json_get(json_array, 3, &value_llu);
        ASSERT_EQ(value_llu, 32, "Array element INT found.");
        Json_get(json_array, 4, &value_bool);
        ASSERT_EQ(value_bool, false, "Array element BOOL found.");
        JsonObj_destroy(&json_obj);
    }
    PRINT_TEST_TITLE("test_json_array_2.json");
    {
        __autofree_cstr__ char* json_cstr = load_file_alloc("test/assets/test_json_array_2.json");
        __autodestroy_json__ JsonObj json_obj;
        JsonItem* json_item;
        JsonArray* json_array;
        JsonArray* json_array_2;
        llu_t value_llu;
        const char* value_cstr;
        double value_double;
        bool value_bool;
        ASSERT_OK(JsonObj_new(json_cstr, &json_obj), "Json object created");
        Json_get(&json_obj, "array_key", &json_array);
        ASSERT_EQ(json_array != NULL, true, "Array found as root element.");
        Json_get(json_array, 0, &json_item);
        ASSERT_EQ(json_item != NULL, true, "First array element is an item.");
        Json_get(json_item, "inner array 1", &json_array_2);
        Json_get(json_array_2, 0, &value_llu);
        ASSERT_EQ(value_llu, 12314, "Value LLU found");
        Json_get(json_array_2, 1, &value_double);
        ASSERT_EQ(value_double, -32.4, "Value DOUBLE found");
        Json_get(json_array_2, 2, &value_bool);
        ASSERT_EQ(value_bool, true, "Value TRUE found");
        Json_get(json_array, 1, &json_item);
        ASSERT_EQ(json_item != NULL, true, "Second array element is an item.");
        Json_get(json_item, "inner array 2", &json_array_2);
        Json_get(json_array_2, 0, &value_double);
        ASSERT_EQ(value_double, 1.4, "Value DOUBLE found");
        Json_get(json_array_2, 1, &value_cstr);
        ASSERT_EQ(value_cstr, "hello", "Value STRING found");
        Json_get(json_array_2, 2, &value_bool);
        ASSERT_EQ(value_bool, false, "Value FALSE found");
    }
    PRINT_TEST_TITLE("test_json_array_3.json");
    {
        __autodestroy_json__ JsonObj json_obj;
        JsonItem* json_item;
        const char* value_cstr;
        llu_t value_llu;
        JsonArray* json_array;
        __autofree_cstr__ char* json_cstr = load_file_alloc("test/assets/test_json_array_3.json");
        ASSERT_OK(JsonObj_new(json_cstr, &json_obj), "Json object created");
        ASSERT(Json_get(&json_obj, "Snapshot", &json_item) == ERR_ALL_GOOD, "Ok");
        ASSERT(Json_get(json_item, "Value", &value_llu) == ERR_ALL_GOOD, "Ok");
        ASSERT(Json_get(json_item, "Data", &json_array) == ERR_ALL_GOOD, "Ok");
        ASSERT(Json_get(json_array, 0, &json_item) == ERR_ALL_GOOD, "Ok");
        ASSERT(Json_get(json_item, "Time", &value_cstr) == ERR_ALL_GOOD, "Ok");
        ASSERT_EQ(value_cstr, "2021-07-23T08:09:00.000000Z", "Time correct.");
    }
    PRINT_TEST_TITLE("test_json_array_4.json");
    {
        JsonObj json_obj;
        llu_t value_llu;
        JsonArray* json_array;
        bool value_bool;
        __autofree_cstr__ char* json_cstr = load_file_alloc("test/assets/test_json_array_4.json");
        ASSERT_OK(JsonObj_new(json_cstr, &json_obj), "Json object created");
        ASSERT_OK(Json_get(&json_obj, "array_key1", &json_array), "Ok");
        ASSERT_OK(Json_get(json_array, 0, &value_llu), "Ok");
        ASSERT_EQ(value_llu, 32, "Ok");
        ASSERT_OK(Json_get(json_array, 1, &value_bool), "Ok");
        ASSERT_EQ(value_bool, false, "Ok");
        ASSERT_OK(Json_get(&json_obj, "array_key2", &json_array), "Ok");
        ASSERT_OK(Json_get(json_array, 0, &value_llu), "Ok");
        ASSERT_EQ(value_llu, 33, "Ok");
        ASSERT_OK(Json_get(json_array, 1, &value_bool), "Ok");
        ASSERT_EQ(value_bool, true, "Ok");
        ASSERT_OK(Json_get(&json_obj, "key", &value_llu), "Ok");
        ASSERT_EQ(value_llu, 34, "Ok");
        JsonObj_destroy(&json_obj);
    }
    PRINT_TEST_TITLE("Testing test/assets/test_json.json");
    {
        __autodestroy_json__ JsonObj json_obj;
        JsonItem* json_item;
        const char* value_cstr;
        llu_t value_llu;
        double value_double;
        bool value_bool;
        JsonArray* json_array;
        __autofree_cstr__ char* json_cstr = load_file_alloc("test/assets/test_json.json");
        ASSERT_OK(JsonObj_new(json_cstr, &json_obj), "Json object created");
        Json_get(&json_obj, "text_key", &value_cstr);
        ASSERT_EQ("text_value", value_cstr, "String*value found in first item");

        Json_get(&json_obj, "text_sibling", &value_cstr);
        ASSERT_EQ("sibling_value", value_cstr, "String*value found in sibling");

        Json_get(&json_obj, "nested_1", &json_item);
        ASSERT_EQ(json_item->key_p, "object_1.1", "Found nested object key");

        Json_get(json_item, "object_1.1", &value_cstr);
        ASSERT_EQ(value_cstr, "item_1.1", "Found nested object value");
        Json_get(json_item, "object_1.2", &value_cstr);
        ASSERT_EQ(value_cstr, "item_1.2", "Found nested sibling object value");
        ASSERT(
            Json_get(json_item, "object_32", &value_cstr) == ERR_JSON_MISSING_ENTRY,
            "Object not found");
        ASSERT(value_cstr == NULL, "Null returned.");

        Json_get(&json_obj, "nested_2", &json_item);
        Json_get(json_item, "object_2.1", &value_cstr);
        ASSERT_EQ(value_cstr, "item_2.1", "Found nested object value");
        Json_get(json_item, "object_2.2", &json_item);
        ASSERT_EQ(json_item->key_p, "item_2.2", "Found nested object key");
        Json_get(json_item, "item_2.2", &value_cstr);
        ASSERT_EQ(value_cstr, "value_2.2.1", "Found nested sibling object value");

        PRINT_TEST_TITLE("Test integer");
        Json_get(&json_obj, "test_integer", &value_llu);
        ASSERT_EQ(value_llu, 435234, "Integer found and read correctly");

        PRINT_TEST_TITLE("Test double");
        Json_get(&json_obj, "test_double", &value_double);
        ASSERT_EQ(value_double, 435.234, "Float found and read correctly");

        PRINT_TEST_TITLE("Test bool true");
        Json_get(&json_obj, "test_bool_true", &value_bool);
        ASSERT_EQ(value_bool, true, "Boolean found and read correctly");

        PRINT_TEST_TITLE("Test bool false");
        Json_get(&json_obj, "test_bool_false", &value_bool);
        ASSERT_EQ(value_bool, false, "Boolean found and read correctly");

        Json_get(&json_obj, "test_array", &json_array);
        Json_get(json_array, 0, &value_llu);
        ASSERT_EQ(value_llu, 14352, "Array element of type INT read correctly");
        Json_get(json_array, 1, &value_double);
        ASSERT_EQ(value_double, 2.15, "Array element of type DOUBLE read correctly");
        Json_get(json_array, 2, &value_cstr);
        ASSERT_EQ(value_cstr, "string_element", "Array element of type C-string read correctly");
    }
    PRINT_TEST_TITLE("Invalid JSON string - empty");
    {
        JsonObj json_obj;
        char* json_cstr = "";
        ASSERT(JsonObj_new(json_cstr, &json_obj) == ERR_EMPTY_STRING, "Empty JSON fails to initialize.");
    }
    PRINT_TEST_TITLE("Invalid JSON string - string not starting with '{'");
    { // TODO: crate token analyzer and add TC's.
        JsonObj json_obj;
        char* json_cstr = "This is not a JSON file";
        ASSERT(JsonObj_new(json_cstr, &json_obj) == ERR_JSON_INVALID, "Invalid JSON fails to initialize.");
    }
    PRINT_TEST_TITLE("Fixed memory leak");
    {
        JsonArray* json_array;
        const char* value_cstr;
        const char* json_char_p = "{\"request\":[\"Required parameter is missing\"]}";
        __autodestroy_json__ JsonObj json_with_vector_obj;
        ASSERT(is_ok(JsonObj_new(json_char_p, &json_with_vector_obj)), "Json object created");
        Json_get(&json_with_vector_obj, "request", &json_array);
        Json_get(json_array, 0, &value_cstr);
        ASSERT_EQ(value_cstr, "Required parameter is missing", "");
    }
    PRINT_TEST_TITLE("Data conversion");
    {
        __autodestroy_json__ JsonObj json_obj;
        llu_t value_llu;
        lld_t value_lld;
        __autofree_cstr__ char* json_cstr = load_file_alloc("test/assets/test_json_numbers.json");
        ASSERT_OK(JsonObj_new(json_cstr, &json_obj), "Json object created");
        Json_get(&json_obj, "value_positive_lld", &value_llu);
        ASSERT_EQ((llu_t)23, value_llu, "Conversion from INT to LLU successfull");
        Json_get(&json_obj, "value_small_llu", &value_lld);
        ASSERT_EQ((lld_t)43, value_lld, "Conversion from LLU to INT successfull");
        ASSERT(Json_get(&json_obj, "value_negative_lld", &value_llu) == ERR_INVALID, "Conversion from negative INT to LLU failed");
        ASSERT(Json_get(&json_obj, "value_large_llu", &value_lld) == ERR_INVALID, "Conversion from large LLU to INT failed");
    }
    /**/
}
#endif /* _TEST */
