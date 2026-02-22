#define __hm_autofree__ __attribute__((cleanup(HashMap_delete)))
#define array_sizeof(__arr__) sizeof(__arr__) / sizeof(__arr__[0])
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
           && ((prime_index + 1) < array_sizeof(__prime_vec)))
    {
        prime_index++;
        printf("%zu, %llu\n", prime_index, __prime_vec[prime_index]);
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
            hm_entry_p->value_cstr = my_memory_malloc(__FILENAME__, __LINE__, 0);
            break;
        }
    }
    return ret_hm_p;
}

void HashMapEntry_delete(HashMapEntry* hm_entry_p, HashMapType type)
{
    if (hm_entry_p->next_p)
    {
        HashMapEntry_delete(hm_entry_p->next_p, type);
    }
    printf("Delete key %s\n", hm_entry_p->key);
    if (type == HM_TYPE_CSTR)
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
        return;
    }
    if (*map_pp == NULL)
    {
        return;
    }
    for (size_t entry_index = 0; entry_index < (*map_pp)->capacity; entry_index++)
    {
        HashMapEntry_delete(&(*map_pp)->entries[entry_index], (*map_pp)->type);
    }

    my_memory_free((*map_pp)->entries);
    my_memory_free(*map_pp);
    *map_pp = NULL;
}

bool HashMap_get_llu(HashMap* hm_p, const char* key, llu_t* out_value_p)
{
    size_t index = __hm_cstr_to_index(key, hm_p->capacity);
    if (strlen(hm_p->entries[index].key))
    {
        *out_value_p = hm_p->entries[index].value_llu;
        return true;
    }
    return false;
}

#define is_entry_used(entry_p) (strlen(hm_entry_p->key))
bool HashMap_put_llu(HashMap* hm_p, const char* key, llu_t value)
{
    size_t index             = __hm_cstr_to_index(key, hm_p->capacity);
    HashMapEntry* hm_entry_p = &hm_p->entries[index];
    while (true)
    {
        if (my_strncmp(hm_entry_p->key, key))
        {
            // Key match
            LOG_INFO("Putting `%s:%llu` at index `%zu`.", key, value, index)
            hm_entry_p->value_llu = value;
            break;
        }
        else if (strlen(hm_entry_p->key) == 0)
        {
            // Empty key - unused entry
            LOG_INFO("Adding key `%s`.", key);
            strncpy(hm_entry_p->key, key, MAX_MAP_KEY_LEN - 1);
            hm_entry_p->key[MAX_MAP_KEY_LEN - 1] = 0; // null terminate
        }
        else
        {
            // Position used, trying next node
            if (hm_entry_p->next_p == NULL)
            {
                hm_entry_p->next_p         = my_memory_malloc(__FILENAME__, __LINE__, sizeof(HashMapEntry));
                hm_entry_p->next_p->prev_p = hm_entry_p;
                hm_entry_p->next_p->next_p = NULL;
                hm_entry_p->next_p->key[0] = 0; // This will trigger the empty-key case
            }
            hm_entry_p = hm_entry_p->next_p;
        }
    }
    // TODO: check if we are out of available spots and return false
    return true;
}

void HashMap_print_llu(HashMap* hm_p)
{
    if (!hm_p)
    {
        return;
    }
    for (size_t index = 0; index < hm_p->capacity; index++)
    {
        HashMapEntry* hm_entry_p = &hm_p->entries[index];
        printf("`%6zu - %s:%llu`\n", //
               index,
               hm_entry_p->key,
               hm_entry_p->value_llu);
        while (hm_entry_p->next_p)
        {
            hm_entry_p = hm_entry_p->next_p;
            printf("`     ----> %s:%llu`\n", //
                   hm_entry_p->key,
                   hm_entry_p->value_llu);
        }
    }
}

// clang-format off
//#define hm_new_element(key, value)                \
//    _Generic((value),                             \
//        short              : hm_new_element_lld,  \
//        int                : hm_new_element_lld,  \
//        long               : hm_new_element_lld,  \
//        long long          : hm_new_element_lld,  \
//        unsigned short     : hm_new_element_llu,  \
//        unsigned int       : hm_new_element_llu,  \
//        unsigned long      : hm_new_element_llu,  \
//        unsigned long long : hm_new_element_llu,  \
//        char *             : hm_new_element_cstr, \
//        const char *       : hm_new_element_cstr  \
//    )(key, value)
// clang-format on

#ifdef _TEST
void test_hashmap(void)
{
    PRINT_BANNER();
    PRINT_TEST_TITLE("Hashing");
    __hm_cstr_to_index("TEST", 5);
    PRINT_TEST_TITLE("Create map")
    {
        const size_t capacity              = 3;
        __hm_autofree__ HashMap* test_hm_p = HashMap_new_with_capacity(HM_TYPE_LLU, capacity);
        ASSERT_EQ(test_hm_p->size, 0, "Initial size is 0");
        ASSERT_EQ(test_hm_p->type, HM_TYPE_LLU, "Initial prime index > 0");
        ASSERT(test_hm_p->capacity > capacity * 1.3, "Initial capacity is sufficiently large");
        ASSERT_EQ(test_hm_p->capacity, 5, "Initial capacity is sufficiently large");
        ASSERT(HashMap_put_llu(test_hm_p, "test key0", 0), "Entry put");
        ASSERT(HashMap_put_llu(test_hm_p, "test key1", 1), "Entry put");
        ASSERT(HashMap_put_llu(test_hm_p, "test key5", 5), "Entry put");
        HashMap_print_llu(test_hm_p);
    }
}
#endif /* _TEST */
