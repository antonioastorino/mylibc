#include "class_json.h"
#include "class_string.h"
#include "class_string_array.h"
#include "common.h"
#include "converter.h"
#include "fs_utils.h"
#include "my_memory.h"
#include "tcp_utils.h"
#include "hash_table.h"

int main(void)
{
#if TEST == 1
    logger_init(NULL, NULL);
    test_logger();
    test_class_string();
    test_class_string_array();
    test_class_json();
    test_converter();
    test_fs_utils();
    test_tcp_utils();
    test_my_memory();
    test_hash_table();
#endif /* TEST == 1 */
}
