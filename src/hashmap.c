bool __HASHMAP_PUT_LLU(const char* __file, int __line, HashMap** __hm_pp, const char* __key, llu_t __value);
bool __HASHMAP_PUT_LLD(const char* __file, int __line, HashMap** __hm_pp, const char* __key, lld_t __value);
bool __HashMap_put_cstr(const char* __file, int __line, HashMap** __hm_pp, const char* __key, char* __value);

llu_t __prime_vec[] = {
    2,
    5,
    11,
    23,
    47,
    97,
    197,
    397,
    797,
    1597,
    3203,
    6421,
    12853,
    25717,
    51437,
    102877,
    205759,
    411527,
    823117,
};

size_t __hm_cstr_to_index(char const* cstr, llu_t const max_size)
{
    size_t ret_val = 0;
    while (*cstr)
    {
        ret_val = (ret_val + *cstr) % max_size;
        cstr++;
    }
    return ret_val;
}

#define __HASHMAP_TRAVERSE(__suffix, __SUFFIX, __type, __ret_action)      \
    bool HashMap_traverse_##__suffix(                                     \
        HashMap* hm_p,                                                    \
        bool* restart,                                                    \
        char* out_key,                                                    \
        __type out_val_p)                                                 \
    {                                                                     \
        static size_t curr_index        = 0;                              \
        static HashMapEntry* hm_entry_p = NULL;                           \
        out_key[0]                      = 0;                              \
        *out_val_p                      = 0;                              \
        if (!hm_p)                                                        \
        {                                                                 \
            return false;                                                 \
        }                                                                 \
        if (hm_p->type != HM_TYPE_##__SUFFIX)                             \
        {                                                                 \
            LOG_ERROR("Wrong hashmap type: expected HM_TYPE_" #__SUFFIX); \
            return false;                                                 \
        }                                                                 \
        if (*restart)                                                     \
        {                                                                 \
            *restart   = false;                                           \
            hm_entry_p = NULL;                                            \
            curr_index = 0;                                               \
        }                                                                 \
        if (curr_index >= hm_p->capacity)                                 \
        {                                                                 \
            LOG_INFO("Reached the bottom");                               \
            return false;                                                 \
        }                                                                 \
                                                                          \
        /* hm_entry_p is set to NULL when we want to look at              \
         * the root element, otherwise it points to a child element */    \
        if (!hm_entry_p)                                                  \
        {                                                                 \
            hm_entry_p = &hm_p->entries[curr_index];                      \
        }                                                                 \
                                                                          \
        while (hm_entry_p->key[0] == 0)                                   \
        {                                                                 \
            curr_index++;                                                 \
            if (curr_index >= hm_p->capacity)                             \
            {                                                             \
                LOG_INFO("No more entries");                              \
                return false;                                             \
            }                                                             \
            hm_entry_p = &hm_p->entries[curr_index];                      \
        }                                                                 \
        strncpy(out_key, hm_entry_p->key, MAX_MAP_KEY_LEN - 1);           \
        __ret_action;                                                     \
        /* Set the next place to scan:                                    \
         * - the next child of the current element                        \
         * - the next element*/                                           \
        if (hm_entry_p->next_p)                                           \
        {                                                                 \
            hm_entry_p = hm_entry_p->next_p;                              \
        }                                                                 \
        else                                                              \
        {                                                                 \
            hm_entry_p = NULL;                                            \
            curr_index++;                                                 \
        }                                                                 \
        LOG_TRACE("Next index `%zu`", curr_index);                        \
        return true;                                                      \
    }

// clang-format off
__HASHMAP_TRAVERSE(lld, LLD, lld_t*, *out_val_p = hm_entry_p->value_lld)
__HASHMAP_TRAVERSE(llu, LLU, llu_t*, *out_val_p = hm_entry_p->value_llu)
__HASHMAP_TRAVERSE(cstr, CSTR, char**,
            size_t val_len = strlen(hm_entry_p->value_cstr) + 1;
            *out_val_p     = (char*)my_memory_malloc(__FILE__, __LINE__, val_len);
            strncpy(*out_val_p, hm_entry_p->value_cstr, val_len)
)

#define HashMap_traverse(__hm_p, __restart, __out_key_p, __out_val_p) \
    _Generic((__out_val_p),                                           \
    llu_t* : HashMap_traverse_llu,                                    \
    lld_t* : HashMap_traverse_lld,                                    \
    char** : HashMap_traverse_cstr                                    \
    )(__hm_p, __restart, __out_key_p, __out_val_p)
// clang-format on

HashMap* __HashMap_new_with_capacity(const char* __file, int __line, HashMapType hm_type, size_t capacity)
{
    HashMap* ret_hm_p  = my_memory_malloc(__file, __line, sizeof(HashMap));
    ret_hm_p->size     = 0;
    ret_hm_p->type     = hm_type;
    size_t prime_index = 0;
    while (capacity * 1.3 > __prime_vec[prime_index]
           && ((prime_index + 1) < sizeof_array(__prime_vec)))
    {
        prime_index++;
    }
    ret_hm_p->capacity = __prime_vec[prime_index];
    if (capacity >= ret_hm_p->capacity)
    {
        LOG_WARNING("HM capacity overflow");
    }
    ret_hm_p->entries = my_memory_malloc(__file, __line, sizeof(HashMapEntry) * ret_hm_p->capacity);
    for (size_t entry_index = 0; entry_index < ret_hm_p->capacity; entry_index++)
    {
        HashMapEntry* hm_entry_p = &ret_hm_p->entries[entry_index];
        hm_entry_p->prev_p       = NULL;
        hm_entry_p->next_p       = NULL;
        hm_entry_p->key[0]       = 0;
        switch (hm_type)
        {
        case HM_TYPE_LLU:
            hm_entry_p->value_llu = 0;
            break;
        case HM_TYPE_LLD:
            hm_entry_p->value_lld = 0;
            break;
        case HM_TYPE_CSTR:
            // Allocate 0 bytes to enable always realloc and free
            hm_entry_p->value_cstr = NULL;
            break;
        }
    }
    return ret_hm_p;
}

void HashMap_resize_if_needed(HashMap** src_hm_pp)
{
    if ((src_hm_pp) == NULL | (*src_hm_pp) == NULL)
    {
        return;
    }
    if ((*src_hm_pp)->size * 1.3 <= (*src_hm_pp)->capacity)
    {
        return;
    }
    LOG_WARNING("Size limit reached, creating bigger table");

    HashMap* dst_hm_p = HashMap_new_with_capacity((*src_hm_pp)->type, (*src_hm_pp)->capacity);
    for (size_t index = 0; index < (*src_hm_pp)->capacity; index++)
    {
        HashMapEntry* hm_entry_p = &(*src_hm_pp)->entries[index];
        do
        {
            switch ((*src_hm_pp)->type)
            {
            case HM_TYPE_LLU:
                HashMap_put(&dst_hm_p, hm_entry_p->key, hm_entry_p->value_llu);
                break;
            case HM_TYPE_LLD:
                HashMap_put(&dst_hm_p, hm_entry_p->key, hm_entry_p->value_lld);
                break;
            case HM_TYPE_CSTR:
                if (hm_entry_p->value_cstr)
                {
                    HashMap_put(&dst_hm_p, hm_entry_p->key, hm_entry_p->value_cstr);
                }
                break;
            }
            hm_entry_p = hm_entry_p->next_p;
        } while (hm_entry_p);
    }
    HashMap_delete(src_hm_pp);
    *src_hm_pp = dst_hm_p;
}

void __HashMapEntry_delete(HashMapEntry* hm_entry_p, HashMapType type)
{
    if (hm_entry_p->next_p)
    {
        __HashMapEntry_delete(hm_entry_p->next_p, type);
    }
    if (type == HM_TYPE_CSTR && hm_entry_p->value_cstr)
    {
        my_memory_free(hm_entry_p->value_cstr);
        hm_entry_p->value_cstr = NULL;
    }
    // cannot free the head
    if (hm_entry_p->prev_p)
    {
        my_memory_free(hm_entry_p);
        hm_entry_p = NULL;
    }
}

void HashMap_delete(HashMap** map_pp)
{
    if (map_pp == NULL)
    {
        LOG_WARNING("Cannot delete NULL hashmap pp");
        return;
    }
    if (*map_pp == NULL)
    {
        LOG_WARNING("Cannot delete NULL hashmap p");
        return;
    }
    for (size_t entry_index = 0; entry_index < (*map_pp)->capacity; entry_index++)
    {
        __HashMapEntry_delete(&(*map_pp)->entries[entry_index], (*map_pp)->type);
    }

    my_memory_free((*map_pp)->entries);
    my_memory_free(*map_pp);
    *map_pp = NULL;
}

#define __HASHMAP_GET_(__suffix, __type)                                                 \
    bool __HASHMAP_GET_##__suffix(HashMap* hm_p, const char* __key, __type* out_value_p) \
    {                                                                                    \
        size_t index             = __hm_cstr_to_index(__key, hm_p->capacity);            \
        HashMapEntry* hm_entry_p = &hm_p->entries[index];                                \
        if (hm_p->type == HM_TYPE_CSTR)                                                  \
        {                                                                                \
            LOG_ERROR("Cannot call this function using a hashmap of type cstr")          \
            return false;                                                                \
        }                                                                                \
        if (hm_p->size <= 0)                                                             \
        {                                                                                \
            LOG_WARNING("Cannot get `%s` from empty hashmap", __key);                    \
            return false;                                                                \
        }                                                                                \
        do                                                                               \
        {                                                                                \
            if (my_strncmp(hm_entry_p->key, __key))                                      \
            {                                                                            \
                if (hm_p->type == HM_TYPE_LLU)                                           \
                {                                                                        \
                    *out_value_p = hm_entry_p->value_llu;                                \
                }                                                                        \
                else if (hm_p->type == HM_TYPE_LLU)                                      \
                {                                                                        \
                    *out_value_p = hm_entry_p->value_lld;                                \
                }                                                                        \
                return true;                                                             \
            }                                                                            \
            hm_entry_p = hm_entry_p->next_p;                                             \
        } while (hm_entry_p);                                                            \
        return false;                                                                    \
    }

bool __HashMap_get_cstr_malloc(const char* file, int line, HashMap* hm_p, const char* key, char** out_value_pp)
{
    // This function allocates memory to prevent the output value from affecting the value stored in the hashmap and vice-versa.
    size_t index             = __hm_cstr_to_index(key, hm_p->capacity);
    HashMapEntry* hm_entry_p = &hm_p->entries[index];
    if (hm_p->size <= 0)
    {
        LOG_WARNING("Cannot get `%s` from empty hashmap", key);
        return false;
    }
    do
    {
        if (my_strncmp(hm_entry_p->key, key))
        {
            my_memory_asprintf(file, line, out_value_pp, "%s", hm_entry_p->value_cstr);
            return true;
        }
        hm_entry_p = hm_entry_p->next_p;
    } while (hm_entry_p);
    return false;
}

bool HashMap_remove(HashMap* hm_p, const char* key)
{
    size_t index             = __hm_cstr_to_index(key, hm_p->capacity);
    HashMapEntry* hm_entry_p = &hm_p->entries[index];
    if (hm_p->size <= 0)
    {
        LOG_WARNING("Cannot remove `%s` from empty hashmap", key);
        return false;
    }
    do
    {
        if (my_strncmp(hm_entry_p->key, key))
        {
            if (hm_p->type == HM_TYPE_CSTR)
            {
                my_memory_free(hm_entry_p->value_cstr);
                hm_entry_p->value_cstr = NULL;
            }
            if (hm_entry_p->prev_p)
            {
                // It's not the head
                hm_entry_p->prev_p->next_p = hm_entry_p->next_p;
                my_memory_free(hm_entry_p);
                hm_entry_p = NULL;
            }
            else
            {
                // This is the head and the tail
                hm_entry_p->key[0] = 0;
            }
            hm_p->size--;
            return true;
        }
        hm_entry_p = hm_entry_p->next_p;
    } while (hm_entry_p);
    return false;
}

#define __HASHMAP_PUT_(__suffix, __type)                                                                       \
    bool __HASHMAP_PUT_##__suffix(                                                                             \
        const char* __file,                                                                                    \
        int __line,                                                                                            \
        HashMap** __hm_pp,                                                                                     \
        const char* __key,                                                                                     \
        __type __value)                                                                                        \
    {                                                                                                          \
        size_t index             = __hm_cstr_to_index(__key, (*__hm_pp)->capacity);                            \
        HashMapEntry* hm_entry_p = &(*__hm_pp)->entries[index];                                                \
        if ((*__hm_pp)->type != HM_TYPE_##__suffix)                                                            \
        {                                                                                                      \
            LOG_ERROR("Cannot use HashMap of type `%d` for type `%d`.", (*__hm_pp)->type, HM_TYPE_##__suffix); \
            return false;                                                                                      \
        }                                                                                                      \
        while (true)                                                                                           \
        {                                                                                                      \
            if (my_strncmp(hm_entry_p->key, __key))                                                            \
            {                                                                                                  \
                /* Key match */                                                                                \
                if ((*__hm_pp)->type == HM_TYPE_LLU)                                                           \
                {                                                                                              \
                    LOG_TRACE("Putting `%s:%llu` at index `%zu`.", __key, __value, index)                      \
                    hm_entry_p->value_llu = __value;                                                           \
                }                                                                                              \
                if ((*__hm_pp)->type == HM_TYPE_LLD)                                                           \
                {                                                                                              \
                    LOG_TRACE("Putting `%s:%lld` at index `%zu`.", __key, __value, index)                      \
                    hm_entry_p->value_lld = __value;                                                           \
                }                                                                                              \
                break;                                                                                         \
            }                                                                                                  \
            else if (hm_entry_p->key[0] == 0)                                                                  \
            {                                                                                                  \
                /* Empty key -> unused entry */                                                                \
                LOG_TRACE("Adding key `%s`.", __key);                                                          \
                strncpy(hm_entry_p->key, __key, MAX_MAP_KEY_LEN - 1);                                          \
                hm_entry_p->key[MAX_MAP_KEY_LEN - 1] = 0; /* null terminate*/                                  \
                (*__hm_pp)->size++;                                                                            \
            }                                                                                                  \
            else                                                                                               \
            {                                                                                                  \
                /* Position used, trying next node*/                                                           \
                if (hm_entry_p->next_p == NULL)                                                                \
                {                                                                                              \
                    hm_entry_p->next_p         = my_memory_malloc(__file, __line, sizeof(HashMapEntry));       \
                    hm_entry_p->next_p->prev_p = hm_entry_p;                                                   \
                    hm_entry_p->next_p->next_p = NULL;                                                         \
                    hm_entry_p->next_p->key[0] = 0; /* This will trigger the empty-key case*/                  \
                }                                                                                              \
                hm_entry_p = hm_entry_p->next_p;                                                               \
            }                                                                                                  \
        }                                                                                                      \
        HashMap_resize_if_needed(__hm_pp);                                                                     \
        return true;                                                                                           \
    }

bool __HashMap_put_cstr(const char* __file, int __line, HashMap** __hm_pp, const char* __key, char* __value)
{
    size_t index             = __hm_cstr_to_index(__key, (*__hm_pp)->capacity);
    HashMapEntry* hm_entry_p = &(*__hm_pp)->entries[index];
    if ((*__hm_pp)->type != HM_TYPE_CSTR)
    {
        LOG_ERROR("Cannot use HashMap of type `%d` for type `%d`.", (*__hm_pp)->type, HM_TYPE_CSTR);
        return false;
    }
    while (true)
    {
        if (my_strncmp(hm_entry_p->key, __key))
        {
            // Key match
            LOG_TRACE("Putting `%s:%s` at index `%zu`.", __key, __value, index)
            if (hm_entry_p->value_cstr)
            {
                my_memory_free(hm_entry_p->value_cstr);
            }
            my_memory_asprintf(__file, __line, &hm_entry_p->value_cstr, __value);
            break;
        }
        else if (hm_entry_p->key[0] == 0)
        {
            // Empty key -> unused entry
            LOG_TRACE("Adding key `%s`.", __key);
            strncpy(hm_entry_p->key, __key, MAX_MAP_KEY_LEN - 1);
            hm_entry_p->key[MAX_MAP_KEY_LEN - 1] = 0; // null terminate
            (*__hm_pp)->size++;
        }
        else
        {
            // Position used, trying next node
            if (hm_entry_p->next_p == NULL)
            {
                hm_entry_p->next_p             = my_memory_malloc(__file, __line, sizeof(HashMapEntry));
                hm_entry_p->next_p->prev_p     = hm_entry_p;
                hm_entry_p->next_p->next_p     = NULL;
                hm_entry_p->next_p->value_cstr = NULL;
                hm_entry_p->next_p->key[0]     = 0; // This will trigger the empty-key case
            }
            hm_entry_p = hm_entry_p->next_p;
        }
    }
    HashMap_resize_if_needed(__hm_pp);
    return true;
}

void HashMap_print(HashMap* hm_p)
{
    if (!hm_p)
    {
        return;
    }
    for (size_t index = 0; index < hm_p->capacity; index++)
    {
        HashMapEntry* hm_entry_p = &hm_p->entries[index];
        int depth                = 1;
        do
        {
            printf("%*s", depth * 4, "|---> ");
            switch (hm_p->type)
            {
            case HM_TYPE_LLU:
                printf("%6s:%llu\n", hm_entry_p->key, hm_entry_p->value_llu);
                break;
            case HM_TYPE_LLD:
                printf("%6s:%lld\n", hm_entry_p->key, hm_entry_p->value_lld);
                break;
            case HM_TYPE_CSTR:
                printf("%6s:%s\n", hm_entry_p->key, hm_entry_p->value_cstr == NULL ? "(null)" : hm_entry_p->value_cstr);
                break;
            }
            hm_entry_p = hm_entry_p->next_p;
            depth++;
        } while (hm_entry_p);
    }
}

// clang-format off
__HASHMAP_PUT_(LLU, llu_t)
__HASHMAP_PUT_(LLD, lld_t)
__HASHMAP_GET_(LLU, llu_t)
__HASHMAP_GET_(LLD, lld_t)

// clang-format on

#ifdef _TEST
void test_hashmap(void)
{
    PRINT_BANNER();
    PRINT_TEST_TITLE("Hashing");
    __hm_cstr_to_index("TEST", 5);
    PRINT_TEST_TITLE("HasMap LLU create, put, get, remove");
    {
        const size_t capacity              = 4;
        llu_t value_llu                    = 0;
        __hm_autofree__ HashMap* test_hm_p = HashMap_new_with_capacity(HM_TYPE_LLU, capacity);
        ASSERT_EQ(test_hm_p->size, 0, "Initial size is 0");
        ASSERT_EQ(test_hm_p->type, HM_TYPE_LLU, "Type set correctly");
        ASSERT(test_hm_p->capacity > capacity * 1.3, "Initial capacity is sufficiently large");
        ASSERT_EQ(test_hm_p->capacity, 11, "Initial capacity correct");
        ASSERT(HashMap_put(&test_hm_p, "test key00", 99U), "Entry put");
        ASSERT_EQ(test_hm_p->size, 1, "Size increased");
        ASSERT(HashMap_put(&test_hm_p, "test key01", 1LU), "Entry put");
        ASSERT_EQ(test_hm_p->size, 2, "Size increased");
        ASSERT(HashMap_put(&test_hm_p, "test key0F", 5LLU), "Entry put");
        ASSERT_EQ(test_hm_p->size, 3, "Size increased");
        ASSERT(HashMap_put(&test_hm_p, "test keyFF", (unsigned short)55), "Entry put");
        ASSERT_EQ(test_hm_p->size, 4, "Size increased");
        ASSERT(HashMap_get_llu(test_hm_p, "test keyFF", &value_llu), "Key found");
        ASSERT(value_llu == 55, "Value correct");
        ASSERT(HashMap_remove(test_hm_p, "test key00"), "Head removed");
        ASSERT(!HashMap_get_llu(test_hm_p, "test key00", &value_llu), "Key not found after removal");
        ASSERT_EQ(test_hm_p->size, 3, "Size decreased");
        ASSERT(HashMap_put(&test_hm_p, "test key00", 98U), "Entry put");
        ASSERT_EQ(test_hm_p->size, 4, "Size increased");
        ASSERT(HashMap_get_llu(test_hm_p, "test key00", &value_llu), "Key found after removal");
        ASSERT_EQ(value_llu, 98, "Value updated");
        ASSERT(HashMap_put(&test_hm_p, "test key00", 100U), "Existing head entry updated");
        ASSERT_EQ(test_hm_p->size, 4, "Size unchanged");
        ASSERT(HashMap_get_llu(test_hm_p, "test key00", &value_llu), "Key still there");
        ASSERT_EQ(value_llu, 100, "Value updated");
        ASSERT(HashMap_put(&test_hm_p, "test key0F", 101U), "Existing node entry updated");
        ASSERT_EQ(test_hm_p->size, 4, "Size unchanged");
        ASSERT(HashMap_get_llu(test_hm_p, "test key0F", &value_llu), "Key still there");
        ASSERT_EQ(value_llu, 101, "Value updated");
        ASSERT(HashMap_put(&test_hm_p, "test keyFF", 102U), "Existing leaf entry updated");
        ASSERT_EQ(test_hm_p->size, 4, "Size unchanged");
        ASSERT(HashMap_get_llu(test_hm_p, "test keyFF", &value_llu), "Key still there");
        ASSERT_EQ(value_llu, 102, "Value updated");

        ASSERT(HashMap_remove(test_hm_p, "test keyFF"), "Leaf removed");
        ASSERT_EQ(test_hm_p->size, 3, "Size decreased");
        ASSERT(HashMap_put(&test_hm_p, "test keyFF", 3U), "Node re-added");
        ASSERT_EQ(test_hm_p->size, 4, "Size increased");
        ASSERT(HashMap_get_llu(test_hm_p, "test keyFF", &value_llu), "Key found");
        ASSERT_EQ(value_llu, 3U, "Value updated");

        HashMap_print(test_hm_p);
        ASSERT(HashMap_remove(test_hm_p, "test key0F"), "Node removed");
        ASSERT_EQ(test_hm_p->size, 3, "Size decreased");
        ASSERT(HashMap_put(&test_hm_p, "test key0F", 2U), "Node re-added (now become a leaf)");
        ASSERT_EQ(test_hm_p->size, 4, "Size increased");
        ASSERT(HashMap_get_llu(test_hm_p, "test key0F", &value_llu), "Key found");
        ASSERT_EQ(value_llu, 2U, "Value updated");
        HashMap_print(test_hm_p);
    }
    PRINT_TEST_TITLE("HasMap CSTR create, put, get, remove");
    {
        const size_t capacity               = 4;
        char* value1_cstr __autofree_cstr__ = NULL;
        char* value2_cstr __autofree_cstr__ = NULL;
        char* value3_cstr __autofree_cstr__ = NULL;
        __hm_autofree__ HashMap* test_hm_p  = NULL;
        test_hm_p                           = HashMap_new_with_capacity(HM_TYPE_CSTR, capacity);
        ASSERT_EQ(test_hm_p->size, 0, "Initial size is 0");
        ASSERT_EQ(test_hm_p->type, HM_TYPE_CSTR, "Type set correctly");
        ASSERT(!HashMap_put(&test_hm_p, "test key00", 99U), "Forbidden");
        ASSERT(HashMap_put(&test_hm_p, "test key00", "test val 00"), "Entry put");
        ASSERT(HashMap_put(&test_hm_p, "test key01", "something else"), "Entry put");
        ASSERT_EQ(test_hm_p->size, 2, "Size increased");
        ASSERT(HashMap_get_cstr_malloc(test_hm_p, "test key01", &value1_cstr), "Entry found");
        ASSERT(HashMap_put(&test_hm_p, "test key01", "something different"), "Entry put");
        ASSERT(HashMap_get_cstr_malloc(test_hm_p, "test key01", &value2_cstr), "Entry found");
        ASSERT_EQ("something else", value1_cstr, "Returned value unchanged after updating hashmap");
        ASSERT_EQ("something different", value2_cstr, "Value correct");
        ASSERT_EQ(test_hm_p->size, 2, "Size unchanged");
        ASSERT(HashMap_put(&test_hm_p, "test key0F", "node 1"), "Entry put");
        ASSERT_EQ(test_hm_p->size, 3, "Size increased");
        ASSERT(HashMap_put(&test_hm_p, "test keyFF", "node 2"), "Entry put");
        ASSERT_EQ(test_hm_p->size, 4, "Size increased");
        ASSERT(HashMap_get_cstr_malloc(test_hm_p, "test keyFF", &value3_cstr), "Key found");
        ASSERT_EQ("node 2", value3_cstr, "Value correct");
        ASSERT(HashMap_remove(test_hm_p, "test key00"), "Head removed");
        ASSERT(!HashMap_get_cstr_malloc(test_hm_p, "test key00", &value1_cstr), "Key not found after removal");
        ASSERT_EQ(test_hm_p->size, 3, "Size decreased");
        ASSERT(HashMap_put(&test_hm_p, "test key00", "New head"), "Entry put");
        ASSERT_EQ(test_hm_p->size, 4, "Size increased");
        HashMap_print(test_hm_p);
    }
    PRINT_TEST_TITLE("HasMap LLD create, put, get, remove");
    {
        const size_t capacity              = 4;
        lld_t value_lld                    = 0;
        __hm_autofree__ HashMap* test_hm_p = HashMap_new_with_capacity(HM_TYPE_LLD, capacity);
        ASSERT_EQ(test_hm_p->size, 0, "Initial size is 0");
        ASSERT_EQ(test_hm_p->type, HM_TYPE_LLD, "Type set correctly");
        ASSERT(test_hm_p->capacity > capacity * 1.3, "Initial capacity is sufficiently large");
        ASSERT_EQ(test_hm_p->capacity, 11, "Initial capacity correct");
        ASSERT(!HashMap_put(&test_hm_p, "test key00", 99U), "Entry not put");
        ASSERT(!HashMap_get_lld(test_hm_p, "test key00", &value_lld), "Entry not gotten");
        ASSERT(HashMap_put(&test_hm_p, "test key00", 99), "Entry put");
        ASSERT(HashMap_get_lld(test_hm_p, "test key00", &value_lld), "Entry gotten");
    }
    PRINT_TEST_TITLE("HasMap LLD create, put, and resize");
    {
        const size_t capacity              = 1;
        __hm_autofree__ HashMap* test_hm_p = HashMap_new_with_capacity(HM_TYPE_LLD, capacity);
        ASSERT_EQ(test_hm_p->size, 0, "Initial size is 0");
        ASSERT_EQ(test_hm_p->type, HM_TYPE_LLD, "Type set correctly");
        ASSERT(test_hm_p->capacity > capacity * 1.3, "Initial capacity is sufficiently large");
        ASSERT_EQ(test_hm_p->capacity, 2, "Initial capacity correct");
        ASSERT(HashMap_put(&test_hm_p, "test key00", 99), "Entry put");
        HashMap_print(test_hm_p);
        ASSERT(HashMap_put(&test_hm_p, "test key02", 99), "Entry put");
        ASSERT_EQ(test_hm_p->capacity, 5, "Capacity increased correctly");
        HashMap_print(test_hm_p);
    }
    PRINT_TEST_TITLE("HasMap CSRT create, put, and resize");
    {
        const size_t capacity              = 1;
        __hm_autofree__ HashMap* test_hm_p = HashMap_new_with_capacity(HM_TYPE_CSTR, capacity);
        ASSERT_EQ(test_hm_p->size, 0, "Initial size is 0");
        ASSERT_EQ(test_hm_p->type, HM_TYPE_CSTR, "Type set correctly");
        ASSERT(test_hm_p->capacity > capacity * 1.3, "Initial capacity is sufficiently large");
        ASSERT_EQ(test_hm_p->capacity, 2, "Initial capacity correct");
        ASSERT(HashMap_put(&test_hm_p, "test key00", "hello"), "Entry put");
        HashMap_print(test_hm_p);
        ASSERT(HashMap_put(&test_hm_p, "test key02", "there"), "Entry put");
        ASSERT_EQ(test_hm_p->capacity, 5, "Capacity increased correctly");
        HashMap_print(test_hm_p);
    }
    PRINT_TEST_TITLE("HashMap LLU traverse");
    {
        const size_t capacity              = 1;
        __hm_autofree__ HashMap* test_hm_p = HashMap_new_with_capacity(HM_TYPE_LLU, capacity);
        bool restart                       = true;
        char key[MAX_MAP_KEY_LEN]          = {0};
        llu_t value_llu                    = 0;
        ASSERT(!HashMap_traverse(test_hm_p, &restart, key, &value_llu), "Empty map");
        ASSERT(restart == false, "Restart reset");
        ASSERT(HashMap_put(&test_hm_p, "First entry", 5041U), "Entry put");
        restart = true;
        ASSERT(HashMap_traverse(test_hm_p, &restart, key, &value_llu), "Map ok");
        ASSERT_EQ(key, "First entry", "Entry found");
        ASSERT(!HashMap_traverse(test_hm_p, &restart, key, &value_llu), "No more entries");
        ASSERT(!HashMap_traverse(test_hm_p, &restart, key, &value_llu), "No more entries");
        ASSERT(HashMap_put(&test_hm_p, "Other entry", 4U), "Entry put");
        HashMap_print(test_hm_p);
        ASSERT(HashMap_put(&test_hm_p, "First entrx", 10U), "Entry put");
        restart = true;
        ASSERT(HashMap_traverse(test_hm_p, &restart, key, &value_llu), "Map ok");
        ASSERT_EQ(key, "Other entry", "Entry found");
        ASSERT_EQ(value_llu, 4U, "Value correct");
        ASSERT(HashMap_traverse(test_hm_p, &restart, key, &value_llu), "Map ok");
        ASSERT_EQ(key, "First entrx", "Entry found");
        ASSERT_EQ(value_llu, 10U, "Value correct");
        ASSERT(HashMap_traverse(test_hm_p, &restart, key, &value_llu), "Map ok");
        ASSERT_EQ(key, "First entry", "Entry found");
        ASSERT_EQ(value_llu, 5041U, "Value correct");
        HashMap_print(test_hm_p);
    }
    PRINT_TEST_TITLE("HashMap LLD traverse");
    {
        const size_t capacity              = 1;
        __hm_autofree__ HashMap* test_hm_p = HashMap_new_with_capacity(HM_TYPE_LLD, capacity);
        bool restart                       = true;
        char key[MAX_MAP_KEY_LEN]          = {0};
        lld_t value_lld                    = 0;
        ASSERT(!HashMap_traverse(test_hm_p, &restart, key, &value_lld), "Empty map");
        ASSERT(restart == false, "Restart reset");
        ASSERT(HashMap_put(&test_hm_p, "First entry", 1), "Entry put");
        restart = true;
        ASSERT(HashMap_traverse(test_hm_p, &restart, key, &value_lld), "Map ok");
        ASSERT_EQ(key, "First entry", "Entry found");
        ASSERT(!HashMap_traverse(test_hm_p, &restart, key, &value_lld), "No more entries");
        ASSERT(!HashMap_traverse(test_hm_p, &restart, key, &value_lld), "No more entries");
        ASSERT(HashMap_put(&test_hm_p, "Other entry", 4), "Entry put");
        HashMap_print(test_hm_p);
        ASSERT(HashMap_put(&test_hm_p, "First entrx", 5), "Entry put");
        restart = true;
        ASSERT(HashMap_traverse(test_hm_p, &restart, key, &value_lld), "Map ok");
        ASSERT_EQ(key, "Other entry", "Entry found");
        ASSERT_EQ(value_lld, 4, "Value correct");
        ASSERT(HashMap_traverse(test_hm_p, &restart, key, &value_lld), "Map ok");
        ASSERT_EQ(key, "First entrx", "Entry found");
        ASSERT_EQ(value_lld, 5, "Value correct");
        ASSERT(HashMap_traverse(test_hm_p, &restart, key, &value_lld), "Map ok");
        ASSERT_EQ(key, "First entry", "Entry found");
        ASSERT_EQ(value_lld, 1, "Value correct");
        HashMap_print(test_hm_p);
    }
    PRINT_TEST_TITLE("HashMap CSTR traverse");
    {
        const size_t capacity              = 1;
        __hm_autofree__ HashMap* test_hm_p = HashMap_new_with_capacity(HM_TYPE_CSTR, capacity);
        bool restart                       = true;
        char key[MAX_MAP_KEY_LEN]          = {0};
        char* value_cstr                   = NULL;
        ASSERT(!HashMap_traverse(test_hm_p, &restart, key, &value_cstr), "Empty map");
        ASSERT(restart == false, "Restart reset");
        ASSERT(HashMap_put(&test_hm_p, "Entry 1", "First entry val"), "Entry put");
        ASSERT(HashMap_put(&test_hm_p, "Entry 2", "Second entry val"), "Entry put");
        ASSERT(HashMap_put(&test_hm_p, "Entry 3", "Third entry val"), "Entry put");
        restart = true;
        while (HashMap_traverse(test_hm_p, &restart, key, &value_cstr))
        {
            printf("Key: `%s`, value: `%s`\n", key, value_cstr);
            my_memory_free_cstr(&value_cstr);
        }
        HashMap_print(test_hm_p);
    }
}
#endif /* _TEST */
