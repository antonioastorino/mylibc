#include "class_json.h"
#include "class_string.h"
#include "class_string_array.h"
#include "common.h"
#include "converter.h"
#include "fs_utils.h"
#include "tcp_utils.h"

#if TEST == 1
int main()
{
    test_class_string();
    test_class_string_array();
    test_class_json();
    test_converter();
    test_fs_utils();
    test_tcp_utils();
}
#else
#error "TEST must be 1"
#endif /* TEST == 0 */
