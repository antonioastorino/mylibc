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

HashMap* HashMap_new_with_capacity(HashMapType hm_type, size_t capacity)
{
    HashMap* ret_hm_p  = my_memory_malloc(__FILENAME__, __LINE__, sizeof(HashMap));
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
    ret_hm_p->entries = my_memory_malloc( //
        __FILENAME__,
        __LINE__,
        sizeof(HashMapEntry) * ret_hm_p->capacity);
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

#define __HASHMAP_GET_(__suffix, __type)                                               \
    bool __HASHMAP_GET_##__suffix(HashMap* hm_p, const char* key, __type* out_value_p) \
    {                                                                                  \
        size_t index             = __hm_cstr_to_index(key, hm_p->capacity);            \
        HashMapEntry* hm_entry_p = &hm_p->entries[index];                              \
        if (hm_p->type == HM_TYPE_CSTR)                                                \
        {                                                                              \
            LOG_ERROR("Cannot call this function using a hashmap of type cstr")        \
            return false;                                                              \
        }                                                                              \
        if (hm_p->size <= 0)                                                           \
        {                                                                              \
            LOG_WARNING("Cannot get `%s` from empty hashmap", key);                    \
            return false;                                                              \
        }                                                                              \
        do                                                                             \
        {                                                                              \
            if (my_strncmp(hm_entry_p->key, key))                                      \
            {                                                                          \
                if (hm_p->type == HM_TYPE_LLU)                                         \
                {                                                                      \
                    *out_value_p = hm_entry_p->value_llu;                              \
                }                                                                      \
                else if (hm_p->type == HM_TYPE_LLU)                                    \
                {                                                                      \
                    *out_value_p = hm_entry_p->value_lld;                              \
                }                                                                      \
                return true;                                                           \
            }                                                                          \
            hm_entry_p = hm_entry_p->next_p;                                           \
        } while (hm_entry_p);                                                          \
        return false;                                                                  \
    }

bool HashMap_get_cstr_malloc(HashMap* hm_p, const char* key, char** out_value_pp)
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
            my_memory_asprintf(__FILENAME__, __LINE__, out_value_pp, "%s", hm_entry_p->value_cstr);
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

#define __HASHMAP_PUT_(__suffix, __type)                                                                         \
    bool __HASHMAP_PUT_##__suffix(HashMap* hm_p, const char* key, __type value)                                  \
    {                                                                                                            \
        size_t index             = __hm_cstr_to_index(key, hm_p->capacity);                                      \
        HashMapEntry* hm_entry_p = &hm_p->entries[index];                                                        \
        if (hm_p->type != HM_TYPE_##__suffix)                                                                    \
        {                                                                                                        \
            LOG_ERROR("Cannot use HashMap of type `%d` for type `%d`.", hm_p->type, HM_TYPE_##__suffix);         \
            return false;                                                                                        \
        }                                                                                                        \
        while (true)                                                                                             \
        {                                                                                                        \
            if (my_strncmp(hm_entry_p->key, key))                                                                \
            {                                                                                                    \
                /* Key match */                                                                                  \
                if (hm_p->type == HM_TYPE_LLU)                                                                   \
                {                                                                                                \
                    LOG_INFO("Putting `%s:%llu` at index `%zu`.", key, value, index)                             \
                    hm_entry_p->value_llu = value;                                                               \
                }                                                                                                \
                if (hm_p->type == HM_TYPE_LLD)                                                                   \
                {                                                                                                \
                    LOG_INFO("Putting `%s:%lld` at index `%zu`.", key, value, index)                             \
                    hm_entry_p->value_lld = value;                                                               \
                }                                                                                                \
                break;                                                                                           \
            }                                                                                                    \
            else if (strlen(hm_entry_p->key) == 0)                                                               \
            {                                                                                                    \
                /* Empty key -> unused entry */                                                                  \
                LOG_INFO("Adding key `%s`.", key);                                                               \
                strncpy(hm_entry_p->key, key, MAX_MAP_KEY_LEN - 1);                                              \
                hm_entry_p->key[MAX_MAP_KEY_LEN - 1] = 0; /* null terminate*/                                    \
                hm_p->size++;                                                                                    \
            }                                                                                                    \
            else                                                                                                 \
            {                                                                                                    \
                /* Position used, trying next node*/                                                             \
                if (hm_entry_p->next_p == NULL)                                                                  \
                {                                                                                                \
                    hm_entry_p->next_p         = my_memory_malloc(__FILENAME__, __LINE__, sizeof(HashMapEntry)); \
                    hm_entry_p->next_p->prev_p = hm_entry_p;                                                     \
                    hm_entry_p->next_p->next_p = NULL;                                                           \
                    hm_entry_p->next_p->key[0] = 0; /* This will trigger the empty-key case*/                    \
                }                                                                                                \
                hm_entry_p = hm_entry_p->next_p;                                                                 \
            }                                                                                                    \
        }                                                                                                        \
        /* TODO: check if we are out of available spots and return false*/                                       \
        return true;                                                                                             \
    }

bool __HashMap_put_cstr(HashMap* hm_p, const char* key, char* value)
{
    size_t index             = __hm_cstr_to_index(key, hm_p->capacity);
    HashMapEntry* hm_entry_p = &hm_p->entries[index];
    if (hm_p->type != HM_TYPE_CSTR)
    {
        LOG_ERROR("Cannot use HashMap of type `%d` for type `%d`.", hm_p->type, HM_TYPE_CSTR);
        return false;
    }
    while (true)
    {
        if (my_strncmp(hm_entry_p->key, key))
        {
            // Key match
            LOG_INFO("Putting `%s:%s` at index `%zu`.", key, value, index)
            if (hm_entry_p->value_cstr)
            {
                my_memory_free(hm_entry_p->value_cstr);
            }
            my_memory_asprintf(__FILENAME__, __LINE__, &hm_entry_p->value_cstr, value);
            break;
        }
        else if (strlen(hm_entry_p->key) == 0)
        {
            // Empty key -> unused entry
            LOG_INFO("Adding key `%s`.", key);
            strncpy(hm_entry_p->key, key, MAX_MAP_KEY_LEN - 1);
            hm_entry_p->key[MAX_MAP_KEY_LEN - 1] = 0; // null terminate
            hm_p->size++;
        }
        else
        {
            // Position used, trying next node
            if (hm_entry_p->next_p == NULL)
            {
                hm_entry_p->next_p             = my_memory_malloc(__FILENAME__, __LINE__, sizeof(HashMapEntry));
                hm_entry_p->next_p->prev_p     = hm_entry_p;
                hm_entry_p->next_p->next_p     = NULL;
                hm_entry_p->next_p->value_cstr = NULL;
                hm_entry_p->next_p->key[0]     = 0; // This will trigger the empty-key case
            }
            hm_entry_p = hm_entry_p->next_p;
        }
    }
    // TODO: check if we are out of available spots and return false
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

#define HashMap_put(hm_p, key, value)            \
    _Generic((value),                            \
        unsigned short     : __HASHMAP_PUT_LLU,  \
        unsigned int       : __HASHMAP_PUT_LLU,  \
        unsigned long      : __HASHMAP_PUT_LLU,  \
        unsigned long long : __HASHMAP_PUT_LLU,  \
        short              : __HASHMAP_PUT_LLD,  \
        int                : __HASHMAP_PUT_LLD,  \
        long               : __HASHMAP_PUT_LLD,  \
        long long          : __HASHMAP_PUT_LLD,  \
        char*              : __HashMap_put_cstr, \
        const char*        : __HashMap_put_cstr  \
    )(hm_p, key, value)
// clang-format on

#define HashMap_get_llu __HASHMAP_GET_LLU
#define HashMap_get_lld __HASHMAP_GET_LLD

#ifdef _TEST
void test_hashmap(void)
{
    PRINT_BANNER();
    PRINT_TEST_TITLE("Hashing");
    __hm_cstr_to_index("TEST", 5);
    PRINT_TEST_TITLE("HasMap LLU create, put, get, remove")
    {
        const size_t capacity              = 4;
        llu_t value_llu                    = 0;
        __hm_autofree__ HashMap* test_hm_p = HashMap_new_with_capacity(HM_TYPE_LLU, capacity);
        ASSERT_EQ(test_hm_p->size, 0, "Initial size is 0");
        ASSERT_EQ(test_hm_p->type, HM_TYPE_LLU, "Type set correctly");
        ASSERT(test_hm_p->capacity > capacity * 1.3, "Initial capacity is sufficiently large");
        ASSERT_EQ(test_hm_p->capacity, 11, "Initial capacity correct");
        ASSERT(HashMap_put(test_hm_p, "test key00", 99U), "Entry put");
        ASSERT_EQ(test_hm_p->size, 1, "Size increased");
        ASSERT(HashMap_put(test_hm_p, "test key01", 1LU), "Entry put");
        ASSERT_EQ(test_hm_p->size, 2, "Size increased");
        ASSERT(HashMap_put(test_hm_p, "test key0F", 5LLU), "Entry put");
        ASSERT_EQ(test_hm_p->size, 3, "Size increased");
        ASSERT(HashMap_put(test_hm_p, "test keyFF", (unsigned short)55), "Entry put");
        ASSERT_EQ(test_hm_p->size, 4, "Size increased");
        ASSERT(HashMap_get_llu(test_hm_p, "test keyFF", &value_llu), "Key found");
        ASSERT(value_llu == 55, "Value correct");
        ASSERT(HashMap_remove(test_hm_p, "test key00"), "Head removed");
        ASSERT(!HashMap_get_llu(test_hm_p, "test key00", &value_llu), "Key not found after removal");
        ASSERT_EQ(test_hm_p->size, 3, "Size decreased");
        ASSERT(HashMap_put(test_hm_p, "test key00", 98U), "Entry put");
        ASSERT_EQ(test_hm_p->size, 4, "Size increased");
        ASSERT(HashMap_get_llu(test_hm_p, "test key00", &value_llu), "Key found after removal");
        ASSERT_EQ(value_llu, 98, "Value updated");
        ASSERT(HashMap_put(test_hm_p, "test key00", 100U), "Existing head entry updated");
        ASSERT_EQ(test_hm_p->size, 4, "Size unchanged");
        ASSERT(HashMap_get_llu(test_hm_p, "test key00", &value_llu), "Key still there");
        ASSERT_EQ(value_llu, 100, "Value updated");
        ASSERT(HashMap_put(test_hm_p, "test key0F", 101U), "Existing node entry updated");
        ASSERT_EQ(test_hm_p->size, 4, "Size unchanged");
        ASSERT(HashMap_get_llu(test_hm_p, "test key0F", &value_llu), "Key still there");
        ASSERT_EQ(value_llu, 101, "Value updated");
        ASSERT(HashMap_put(test_hm_p, "test keyFF", 102U), "Existing leaf entry updated");
        ASSERT_EQ(test_hm_p->size, 4, "Size unchanged");
        ASSERT(HashMap_get_llu(test_hm_p, "test keyFF", &value_llu), "Key still there");
        ASSERT_EQ(value_llu, 102, "Value updated");

        ASSERT(HashMap_remove(test_hm_p, "test keyFF"), "Leaf removed");
        ASSERT_EQ(test_hm_p->size, 3, "Size decreased");
        ASSERT(HashMap_put(test_hm_p, "test keyFF", 3U), "Node re-added");
        ASSERT_EQ(test_hm_p->size, 4, "Size increased");
        ASSERT(HashMap_get_llu(test_hm_p, "test keyFF", &value_llu), "Key found");
        ASSERT_EQ(value_llu, 3U, "Value updated");

        HashMap_print(test_hm_p);
        ASSERT(HashMap_remove(test_hm_p, "test key0F"), "Node removed");
        ASSERT_EQ(test_hm_p->size, 3, "Size decreased");
        ASSERT(HashMap_put(test_hm_p, "test key0F", 2U), "Node re-added (now become a leaf)");
        ASSERT_EQ(test_hm_p->size, 4, "Size increased");
        ASSERT(HashMap_get_llu(test_hm_p, "test key0F", &value_llu), "Key found");
        ASSERT_EQ(value_llu, 2U, "Value updated");
        HashMap_print(test_hm_p);
    }
    PRINT_TEST_TITLE("HasMap CSTR create, put, get, remove")
    {
        const size_t capacity              = 4;
        char* value1_cstr __autofree_ptr__ = NULL;
        char* value2_cstr __autofree_ptr__ = NULL;
        char* value3_cstr __autofree_ptr__ = NULL;
        __hm_autofree__ HashMap* test_hm_p = NULL;
        test_hm_p                          = HashMap_new_with_capacity(HM_TYPE_CSTR, capacity);
        ASSERT_EQ(test_hm_p->size, 0, "Initial size is 0");
        ASSERT_EQ(test_hm_p->type, HM_TYPE_CSTR, "Type set correctly");
        ASSERT(!HashMap_put(test_hm_p, "test key00", 99U), "Forbidden");
        ASSERT(HashMap_put(test_hm_p, "test key00", "test val 00"), "Entry put");
        ASSERT(HashMap_put(test_hm_p, "test key01", "something else"), "Entry put");
        ASSERT_EQ(test_hm_p->size, 2, "Size increased");
        ASSERT(HashMap_get_cstr_malloc(test_hm_p, "test key01", &value1_cstr), "Entry found");
        ASSERT(HashMap_put(test_hm_p, "test key01", "something different"), "Entry put");
        ASSERT(HashMap_get_cstr_malloc(test_hm_p, "test key01", &value2_cstr), "Entry found");
        ASSERT_EQ("something else", value1_cstr, "Returned value unchanged after updating hashmap");
        ASSERT_EQ("something different", value2_cstr, "Value correct");
        ASSERT_EQ(test_hm_p->size, 2, "Size unchanged");
        ASSERT(HashMap_put(test_hm_p, "test key0F", "node 1"), "Entry put");
        ASSERT_EQ(test_hm_p->size, 3, "Size increased");
        ASSERT(HashMap_put(test_hm_p, "test keyFF", "node 2"), "Entry put");
        ASSERT_EQ(test_hm_p->size, 4, "Size increased");
        ASSERT(HashMap_get_cstr_malloc(test_hm_p, "test keyFF", &value3_cstr), "Key found");
        ASSERT_EQ("node 2", value3_cstr, "Value correct");
        ASSERT(HashMap_remove(test_hm_p, "test key00"), "Head removed");
        ASSERT(!HashMap_get_cstr_malloc(test_hm_p, "test key00", &value1_cstr), "Key not found after removal");
        ASSERT_EQ(test_hm_p->size, 3, "Size decreased");
        ASSERT(HashMap_put(test_hm_p, "test key00", "New head"), "Entry put");
        ASSERT_EQ(test_hm_p->size, 4, "Size increased");
        HashMap_print(test_hm_p);
    }
    PRINT_TEST_TITLE("HasMap LLD create, put, get, remove")
    {
        const size_t capacity              = 4;
        lld_t value_lld                    = 0;
        __hm_autofree__ HashMap* test_hm_p = HashMap_new_with_capacity(HM_TYPE_LLD, capacity);
        ASSERT_EQ(test_hm_p->size, 0, "Initial size is 0");
        ASSERT_EQ(test_hm_p->type, HM_TYPE_LLD, "Type set correctly");
        ASSERT(test_hm_p->capacity > capacity * 1.3, "Initial capacity is sufficiently large");
        ASSERT_EQ(test_hm_p->capacity, 11, "Initial capacity correct");
        ASSERT(!HashMap_put(test_hm_p, "test key00", 99U), "Entry not put");
        ASSERT(!HashMap_get_lld(test_hm_p, "test key00", &value_lld), "Entry not gotten");
        ASSERT(HashMap_put(test_hm_p, "test key00", 99), "Entry not put");
        ASSERT(HashMap_get_lld(test_hm_p, "test key00", &value_lld), "Entry not gotten");
    }
}
#endif /* _TEST */
