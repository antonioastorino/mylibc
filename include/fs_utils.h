#ifndef FS_UTILS_H
#define FS_UTILS_H

#include "class_string.h"
#include "common.h"
#include <sys/types.h>

// Folders only.
Error fs_utils_mkdir(const char*, mode_t);
Error fs_utils_mkdir_p(const char*, mode_t);
Error fs_utils_rmdir(const char*);
// Files only.
Error fs_utils_rm_from_path_as_char_p(const char*);
Error fs_utils_read_to_string(const char*, String*);
Error fs_utils_append(const char*, const char*);
Error fs_utils_create_with_content(const char*, const char*);
// Files and folders.
bool fs_utils_does_exist(const char*);
Error fs_utils_rm_r(const char*);
bool fs_utils_is_file(char*);
Error fs_utils_get_file_size(char*, off_t*);

// clang-format off
#define fs_utils_rm(file_path_p)                                        \
    _Generic((file_path_p),                                             \
        const char*   : fs_utils_rm_from_path_as_char_p,                \
        char*         : fs_utils_rm_from_path_as_char_p                 \
    )(file_path_p)

#define fs_utils_read_to_string(file_path_p, out_string)                \
    _Generic((file_path_p),                                             \
        const char* : _fs_utils_read_to_string,                         \
        char* : _fs_utils_read_to_string                                \
    )(file_path_p, out_string)
// clang-format on

#if TEST == 1
void test_fs_utils(void);
#endif
#endif
