#include "hash_table.h"
#include <stdlib.h>

static const uint32_t __prime_vec[] = {
    2,    5,    11,    23,    47,    97,     197,    397,    797,    1597,
    3203, 6421, 12853, 25717, 51437, 102877, 205759, 411527, 823117,
};

static uint8_t __current_prime_index = 0;

static void __next_prime_index(void)
{
    if (__current_prime_index + 1 < sizeof(__prime_vec) / sizeof(__prime_vec[0]))
    {
        __current_prime_index++;
    }
}

#if TEST == 1
void test_hash_table(void)
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
}
#endif /* TEST == 1 */
