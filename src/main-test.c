#include "mylibc.h"
#include "class_json.c"
#include "class_string.c"
#include "class_string_array.c"
#include "common.c"
#include "numparser.c"
#include "fs.c"
#include "my_memory.c"
#include "tcp_utils.c"

#ifdef _TEST
#ifndef _MODULE
int main(void)
{
    logger_init(NULL, NULL);
    test_logger();
    test_class_string();
    test_class_string_array();
    test_class_json();
    test_numparser();
    test_fs();
    test_tcp_utils();
    test_my_memory();
    test_common();
}
#endif /* _MODULE */
#endif /* _TEST */
