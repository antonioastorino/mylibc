uint32_t __prime_vec[] = {
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

void __next_prime_index(HashMap* map)
{
    if (map->prime_index + 1 < sizeof(__prime_vec) / sizeof(__prime_vec[0]))
    {
        map->prime_index++;
    }
}

size_t __cstr_to_index(char const* cstr, size_t const prime_vec_index)
{
    size_t ret_val = 0;
    while (*cstr)
    {
        ret_val = (ret_val + *cstr) % __prime_vec[prime_vec_index];
        cstr++;
    }
    return ret_val;
}

HashMapEntry hashmap_new_element_llu(char const* key, llu_t value)
{
    HashMapEntry ret_map_element = (HashMapEntry){
        .used      = true,
        .type      = MAP_LLU,
        .value_llu = value,
    };
    strncpy(ret_map_element.key, key, MAX_MAP_KEY_LEN - 1);
    ret_map_element.key[MAX_MAP_KEY_LEN - 1] = 0; // null terminate
    return ret_map_element;
}

HashMapEntry hashmap_new_element_lld(char const* key, lld_t value)
{
    HashMapEntry ret_map_element = (HashMapEntry){
        .used      = true,
        .type      = MAP_LLD,
        .value_lld = value,
    };
    strncpy(ret_map_element.key, key, MAX_MAP_KEY_LEN - 1);
    ret_map_element.key[MAX_MAP_KEY_LEN - 1] = 0; // null terminate
    return ret_map_element;
}

HashMapEntry hashmap_new_element_cstr(char const* key, char const* value)
{
    HashMapEntry ret_map_element = (HashMapEntry){
        .used = true,
        .type = MAP_CSTR,
    };
    strncpy(ret_map_element.key, key, MAX_MAP_KEY_LEN - 1);
    strncpy(ret_map_element.value_cstr, value, MAX_MAP_VAL_LEN - 1);
    ret_map_element.key[MAX_MAP_KEY_LEN - 1]        = 0; // null terminate
    ret_map_element.value_cstr[MAX_MAP_VAL_LEN - 1] = 0; // null terminate
    return ret_map_element;
}

HashMap* HashMap_new_with_capacity(size_t capacity)
{
    HashMap* ret_map_p     = my_memory_malloc(__FILENAME__, __LINE__, sizeof(HashMap));
    ret_map_p->prime_index = 0;
    ret_map_p->size        = 0;
    while (capacity * 1.3 > __prime_vec[ret_map_p->prime_index])
    {
        __next_prime_index(ret_map_p);
    }
    return ret_map_p;
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
    my_memory_free(*map_pp);
    *map_pp = NULL;
}

// clang-format off
#define hashmap_new_element(key, value)                \
    _Generic((value),                                  \
        short              : hashmap_new_element_lld,  \
        int                : hashmap_new_element_lld,  \
        long               : hashmap_new_element_lld,  \
        long long          : hashmap_new_element_lld,  \
        unsigned short     : hashmap_new_element_llu,  \
        unsigned int       : hashmap_new_element_llu,  \
        unsigned long      : hashmap_new_element_llu,  \
        unsigned long long : hashmap_new_element_llu,  \
        char *             : hashmap_new_element_cstr, \
        const char *       : hashmap_new_element_cstr  \
    )(key, value)
// clang-format on

#ifdef _TEST
void test_hashmap(void)
{
    PRINT_BANNER();
    PRINT_TEST_TITLE("doubling prime");
    {
        HashMap map = {0};
        ASSERT_EQ(map.prime_index, 0, "Initial index correct");
        __next_prime_index(&map); // 1
        ASSERT_EQ(map.prime_index, 1, "Next index correct");
        ASSERT_EQ(__prime_vec[map.prime_index], 5, "Next prime correct");
        map.prime_index = 17;
        __next_prime_index(&map); // 18
        ASSERT_EQ(map.prime_index, 18, "Last index correct");
        __next_prime_index(&map); // still 18
        ASSERT_EQ(map.prime_index, 18, "Index doesn't overflow");
        ASSERT_EQ(__prime_vec[map.prime_index], 823117, "Index doesn't overflow");
    }
    PRINT_TEST_TITLE("Hashing");
    __cstr_to_index("TEST", 5);
    PRINT_TEST_TITLE("Create map element");
    {
        HashMapEntry map_element = hashmap_new_element("test key", 32U);
        ASSERT_EQ("test key", map_element.key, "Correct key");
        ASSERT_EQ(MAP_LLU, map_element.type, "Correct type");
        ASSERT_EQ(32, map_element.value_llu, "Correct value");
    }
    PRINT_TEST_TITLE("Create map")
    {
        const size_t capacity           = 5;
        HashMap* map_p __autofree_map__ = HashMap_new_with_capacity(capacity);
        ASSERT_EQ(map_p->size, 0, "Initial size is 0");
        ASSERT(map_p->prime_index > 0, "Initial prime index > 0");
        ASSERT(__prime_vec[map_p->prime_index] > capacity * 1.3, "Initial capacity is 0");
        HashMap_delete(&map_p);
    }
}
#endif /* _TEST */
