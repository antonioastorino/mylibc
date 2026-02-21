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

uint8_t __current_prime_index = 0;

void __next_prime_index(void)
{
    if (__current_prime_index + 1 < sizeof(__prime_vec) / sizeof(__prime_vec[0]))
    {
        __current_prime_index++;
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

MapElement hashmap_new_element_llu(char const* key, llu_t value)
{
    MapElement ret_map_element = (MapElement){
        .used      = true,
        .type      = MAP_LLU,
        .value_llu = value,
    };
    strncpy(ret_map_element.key, key, MAX_MAP_KEY_LEN - 1);
    ret_map_element.key[MAX_MAP_KEY_LEN - 1] = 0; // null terminate
    return ret_map_element;
}

MapElement hashmap_new_element_lld(char const* key, lld_t value)
{
    MapElement ret_map_element = (MapElement){
        .used      = true,
        .type      = MAP_LLD,
        .value_lld = value,
    };
    strncpy(ret_map_element.key, key, MAX_MAP_KEY_LEN - 1);
    ret_map_element.key[MAX_MAP_KEY_LEN - 1] = 0; // null terminate
    return ret_map_element;
}

MapElement hashmap_new_element_cstr(char const* key, char const* value)
{
    MapElement ret_map_element = (MapElement){
        .used = true,
        .type = MAP_CSTR,
    };
    strncpy(ret_map_element.key, key, MAX_MAP_KEY_LEN - 1);
    strncpy(ret_map_element.value_cstr, value, MAX_MAP_VAL_LEN - 1);
    ret_map_element.key[MAX_MAP_KEY_LEN - 1]        = 0; // null terminate
    ret_map_element.value_cstr[MAX_MAP_VAL_LEN - 1] = 0; // null terminate
    return ret_map_element;
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
    ASSERT_EQ(__current_prime_index, 0, "Initial index correct");
    __next_prime_index(); // 1
    ASSERT_EQ(__current_prime_index, 1, "Next index correct");
    ASSERT_EQ(__prime_vec[__current_prime_index], 5, "Next prime correct");
    __current_prime_index = 17;
    __next_prime_index(); // 18
    ASSERT_EQ(__current_prime_index, 18, "Last index correct");
    __next_prime_index(); // still 18
    ASSERT_EQ(__current_prime_index, 18, "Index doesn't overflow");
    ASSERT_EQ(__prime_vec[__current_prime_index], 823117, "Index doesn't overflow");
    PRINT_TEST_TITLE("Hashing");
    __cstr_to_index("TEST", 5);
    PRINT_TEST_TITLE("Create map element");
    MapElement map_element = hashmap_new_element("test key", 32U);
    ASSERT_EQ("test key", map_element.key, "Correct key");
    ASSERT_EQ(32, map_element.value_llu, "Correct value");
}
#endif /* _TEST */
