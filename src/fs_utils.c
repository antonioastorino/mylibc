#include "class_string.h"
#include "common.h"
#include "fs_utils.h"
#include "logger.h"
#include "my_memory.h"
#include <errno.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

Error _fs_utils_recursive_rm_r(FTS*, const char*);

bool _fs_utils_does_exist(const char* p_path)
{
    struct stat st = {0};
    if (stat(p_path, &st) == -1)
    {
        return false;
    }
    else
    {
        return true;
    }
}

Error _fs_utils_create_or_append(
    const char* file_path_char_p,
    const char* new_content_char_p,
    const char* flags_char_p)
{
    FILE* file_handler = fopen(file_path_char_p, flags_char_p);
    if (!file_handler)
    {
        LOG_ERROR("Failed to open file.");
        return ERR_NULL;
    }
    fprintf(file_handler, "%s", new_content_char_p);
    fclose(file_handler);
    return ERR_ALL_GOOD;
}

/* ------------------------------------------ Folders ------------------------------------------- */
Error fs_utils_mkdir(const char* dir_path_char_p, mode_t permission)
{
    Error ret_res   = ERR_ALL_GOOD;
    // Save the current mode mask and reset the mask.
    mode_t old_mask = umask(0);
    if (!_fs_utils_does_exist(dir_path_char_p))
    {
        LOG_TRACE("Trying to create %s", dir_path_char_p);
        if (mkdir(dir_path_char_p, permission) == -1)
        {
            LOG_ERROR("mkdir returned with errno `%d`", errno);
            ret_res = ERR_FS_INTERNAL;
        }
        else
        {
            LOG_TRACE("Folder successfully created.", NULL);
        }
    }
    else
    {
        LOG_ERROR("The folder `%s` already exists.", dir_path_char_p);
        ret_res = ERR_FORBIDDEN;
    }
    // Restore the previous mask.
    umask(old_mask);
    return ret_res;
}

Error fs_utils_mkdir_p(const char* dir_path_char_p, mode_t permission)
{
    size_t path_length = strlen(dir_path_char_p);
    LOG_TRACE("Trying to create `%s`", dir_path_char_p);
    if (_fs_utils_does_exist(dir_path_char_p))
    {
        LOG_ERROR("The folder already exists.");
        return ERR_FORBIDDEN;
    }
    char partial_path[path_length + 1];
    size_t start_index = 0;
    if (dir_path_char_p[0] == '/')
    {
        partial_path[0] = '/';
        // Skip the fist folder found (root folder) in the path - it's an absolute path.
        start_index     = 1;
    }
    for (size_t i = start_index; i < path_length; i++)
    {
        if (dir_path_char_p[i] == '/')
        {
            // Terminate the partial string here.
            partial_path[i] = 0;
            LOG_TRACE("Trying to create `%s`.", partial_path);

            // Check if this path exists or try to create it.
            if (!_fs_utils_does_exist(partial_path))
            {
                return_on_err(fs_utils_mkdir(partial_path, permission));
            }
        }
        // Append path chars to partial_path
        partial_path[i] = dir_path_char_p[i];
    }
    /*
    If the path terminated with a '/', then it was created. Otherwise, we need to make the last
    call.
    */
    if (dir_path_char_p[path_length - 1] != '/')
    {
        return_on_err(fs_utils_mkdir(dir_path_char_p, permission));
    }
    LOG_TRACE("`%s` successfully created.", dir_path_char_p);
    return ERR_ALL_GOOD;
}

Error fs_utils_rmdir(const char* dir_path_char_p)
{
    LOG_INFO("Trying to remove `%s` folder.", dir_path_char_p);
    if (!_fs_utils_does_exist(dir_path_char_p))
    {
        LOG_ERROR("Folder not found.");
        return ERR_INVALID;
    }
    if (rmdir(dir_path_char_p))
    {
        LOG_ERROR("Failed to delete `%s`. errno: `%d`", dir_path_char_p, errno)
        return ERR_FS_INTERNAL;
    }

    LOG_INFO("`%s` folder successfully removed.", dir_path_char_p);
    return ERR_ALL_GOOD;
}

/* ------------------------------------------- Files -------------------------------------------- */
Error fs_utils_rm_from_path_as_char_p(const char* file_path_char_p)
{
    Error ret_res  = ERR_ALL_GOOD;
    char* paths[2] = {(char*)file_path_char_p, NULL};

    // Create the received path handle.
    FTS* fts_p     = fts_open(paths, FTS_PHYSICAL | FTS_NOCHDIR, NULL);
    if (fts_p == NULL)
    {
        LOG_ERROR("Failed to initialize fts. errno: `%d`", errno);
        return ERR_FS_INTERNAL;
    }
    /*
      Get next entry (could be file or directory). No need to check for ENOENT because we just
      found it.
      */
    FTSENT* dir_entry_p = fts_read(fts_p);
    if (dir_entry_p->fts_info == FTS_F)
    {
        // It is a file.
        if (unlink(file_path_char_p))
        {
            LOG_TRACE("Removal failed with errno: %d.", errno);
            ret_res = ERR_FS_INTERNAL;
        }
    }
    else
    {
        // It is a folder.
        ret_res = fs_utils_rmdir(file_path_char_p);
    }
    fts_close(fts_p);
    return_on_err(ret_res);
    LOG_TRACE("`%s` successfully deleted.", file_path_char_p);
    return ret_res;
}

Error fs_utils_append(const char* file_path_char_p, const char* new_content_char_p)
{
    return _fs_utils_create_or_append(file_path_char_p, new_content_char_p, "a");
}

Error fs_utils_create_with_content(const char* file_path_char_p, const char* new_content_char_p)
{
    return _fs_utils_create_or_append(file_path_char_p, new_content_char_p, "w");
}

Error _fs_utils_read_to_string(const char* file_path_char_p, String* out_string_obj_p)
{
    FILE* file_handler = fopen(file_path_char_p, "r");
    if (!file_handler)
    {
        LOG_ERROR("Failed to open `%s`", file_path_char_p);
        return ERR_FS_INTERNAL;
    }
    int c;
    size_t chars_read = 0;
    size_t size       = 4096;
    char* buf         = MALLOC(size);
    if (buf == NULL)
    {
        LOG_ERROR("FATAL: out of memory");
        exit(ERR_FATAL);
    }

    while ((c = getc(file_handler)) != EOF)
    {
        if (chars_read >= size - 1)
        {
            /* time to make it bigger */
            size = (size_t)(size * 1.5);
            buf  = REALLOC(buf, size);
            if (buf == NULL)
            {
                LOG_ERROR("FATAL: out of memory");
                exit(ERR_FATAL);
            }
        }
        buf[chars_read++] = c;
    }
    buf[chars_read++] = '\0';
    fclose(file_handler);
    (*out_string_obj_p) = String_new(buf);
    FREE(buf);
    buf = NULL;
    return ERR_ALL_GOOD;
}

/* ------------------------------------- Files and folders -------------------------------------- */
Error _fs_utils_recursive_rm_r(FTS* fts_p, const char* dir_path_char_p)
{
    Error ret_res = ERR_ALL_GOOD;
    /*
    Get next entry (could be file or directory). No need to check for ENOENT because we just found
    it.
    */
    LOG_TRACE("Reached %s", dir_path_char_p);
    FTSENT* dir_entry_p = fts_read(fts_p);
    LOG_TRACE("Info code %d", dir_entry_p->fts_info);
    if (dir_entry_p->fts_info == FTS_D)
    {
        // We are inside a directory - recurse;
        FTSENT* children = fts_children(fts_p, 0);
        LOG_TRACE("Folder `%s` found", dir_entry_p->fts_path);
        FTSENT* link = children;
        while (link != NULL)
        {
            LOG_TRACE("Found child: %s.", link->fts_name);
            String child_path_string = String_new("%s/%s", dir_path_char_p, link->fts_name);

            LOG_TRACE("Trying: %s.", child_path_string.str);
            // Do your recursion thing.
            ret_res = _fs_utils_recursive_rm_r(fts_p, child_path_string.str);
            String_destroy(&child_path_string);
            return_on_err(ret_res);
            // Go to the next entry.
            link = link->fts_link;
        }
    }
    // The recursion is over. Delete what you found.
    struct stat st;
    if (lstat(dir_path_char_p, &st) == -1)
    {
        LOG_ERROR("%s", strerror(errno));
        return ERR_FS_INTERNAL;
    }
    switch (st.st_mode & S_IFMT)
    {
    case S_IFDIR:
        LOG_TRACE("Trying to delete folder `%s`", dir_path_char_p);
        ret_res = fs_utils_rmdir(dir_path_char_p);
        return_on_err(ret_res);
        break;
    case S_IFREG:
        LOG_TRACE("Trying to delete file `%s`", dir_path_char_p);
        if (unlink(dir_path_char_p))
        {
            LOG_ERROR("Removal failed with errno: %d.", errno);
            ret_res = ERR_FS_INTERNAL;
        }
        break;
    default:
        LOG_ERROR("Unsupported or forbidden removal of file type `%d`", st.st_mode & S_IFMT);
        ret_res = ERR_INVALID;
        break;
    }

    return ret_res;
}

Error fs_utils_rm_r(const char* dir_path_char_p)
{
    LOG_INFO("Trying to remove `%s` recursively.", dir_path_char_p);
    if (!_fs_utils_does_exist(dir_path_char_p))
    {
        LOG_ERROR("Folder `%s` not found.", dir_path_char_p);
        return ERR_INVALID;
    }
    char* paths[] = {(char*)dir_path_char_p, NULL};
    // Create the received path handle.
    FTS* fts_p    = fts_open(paths, FTS_PHYSICAL | FTS_NOCHDIR, NULL);
    if (fts_p == NULL)
    {
        LOG_ERROR("Failed to initialize fts. errno `%d`.", errno);
        fts_close(fts_p);
        return ERR_FS_INTERNAL;
    }
    // Start deleting recursively.
    Error ret_res = _fs_utils_recursive_rm_r(fts_p, dir_path_char_p);
    fts_close(fts_p);
    return_on_err(ret_res);
    LOG_INFO("`%s` successfully deleted.", dir_path_char_p);
    return ret_res;
}

bool fs_utils_is_file(char* path_to_file_char_p)
{
    struct stat st = {0};
    if (stat(path_to_file_char_p, &st) == -1)
    {
        return false;
    }
    if ((st.st_mode & S_IFMT) == S_IFREG)
    {
        return true;
    }
    return false;
}

Error fs_utils_get_file_size(char* path_to_file_char_p, off_t* out_file_size)
{
    struct stat st = {0};
    if (stat(path_to_file_char_p, &st) == -1)
    {
        LOG_PERROR("Failed to stat `%s`", path_to_file_char_p);
        return ERR_FS_INTERNAL;
    }
    if ((st.st_mode & S_IFMT) == S_IFREG)
    {
        *out_file_size = st.st_size;
        return ERR_ALL_GOOD;
    }
    LOG_ERROR("`%s` is not a file", path_to_file_char_p);
    return ERR_INVALID;
}

#if TEST == 1
void test_fs_utils()
{
    PRINT_BANNER();
    Error ret_res;
    char* path_string;
    char* content_char_p;
    String content_string;

    PRINT_TEST_TITLE("mkdir - pass")
    path_string = "test/artifacts/test_folder_0";
    ret_res     = fs_utils_mkdir(path_string, 0666);
    ASSERT(ret_res == ERR_ALL_GOOD, "`fs_utils_mkdir` works fine.");

    PRINT_TEST_TITLE("mkdir - should fail")
    ret_res = fs_utils_mkdir(path_string, 0666);
    ASSERT(ret_res == ERR_FORBIDDEN, "`fs_utils_mkdir` should fail if the folder exists.");
    PRINT_TEST_TITLE("mkdir -p - should fail")
    ret_res = fs_utils_mkdir_p(path_string, 0666);
    ASSERT(ret_res == ERR_FORBIDDEN, "`fs_utils_mkdir_p` should fail if the folder exists.");

    PRINT_TEST_TITLE("mkdir -p - pass")
    path_string = "test/artifacts/test_folder_1/new_inner_folder/";
    ret_res     = fs_utils_mkdir_p(path_string, 0777);
    ASSERT(ret_res == ERR_ALL_GOOD, "`fs_utils_mkdir_p` works fine when the path ends with '/'.");

    PRINT_TEST_TITLE("mkdir -p - pass")
    path_string = "test/artifacts/test_folder_2/new_inner_folder";
    ret_res     = fs_utils_mkdir_p(path_string, 0777);
    ASSERT(
        ret_res == ERR_ALL_GOOD,
        "`fs_utils_mkdir_p` works fine when the path does not end with '/'.");

    PRINT_TEST_TITLE("rmdir - pass")
    path_string = "test/artifacts/empty-0";
    ret_res     = fs_utils_rmdir(path_string);
    ASSERT(ret_res == ERR_ALL_GOOD, "`fs_utils_rmdir` works fine when the folder is empty.");

    PRINT_TEST_TITLE("rmdir - fail")
    path_string = "test/artifacts/non-empty-0";
    ret_res     = fs_utils_rmdir(path_string);
    ASSERT(ret_res == ERR_FS_INTERNAL, "`fs_utils_rmdir` should fail if the folder is not empty.");

    PRINT_TEST_TITLE("rm -r - pass")
    path_string = "test/artifacts/empty";
    ret_res     = fs_utils_rm_r(path_string);
    ASSERT(ret_res == ERR_ALL_GOOD, "`fs_utils_rm_r` works fine when the folder is empty.");

    PRINT_TEST_TITLE("rm -r - pass")
    path_string = "test/artifacts/non-empty";
    ret_res     = fs_utils_rm_r(path_string);
    ASSERT(ret_res == ERR_ALL_GOOD, "`fs_utils_rm_r` works fine when the folder is NOT empty.");

    PRINT_TEST_TITLE("rm -r - fail")
    path_string = "test/artifacts/missing";
    ret_res     = fs_utils_rm_r(path_string);
    ASSERT(ret_res == ERR_INVALID, "`fs_utils_rm_r` should fail if the folder is missing.");

    PRINT_TEST_TITLE("rm -r - pass")
    path_string = "test/artifacts/delete_me.txt";
    ret_res     = fs_utils_rm_r(path_string);
    ASSERT(ret_res == ERR_ALL_GOOD, "`fs_utils_rm_r` should NOT fail on a file.");

    PRINT_TEST_TITLE("read to string - short text, use const String*for file path");
    path_string = "test/assets/readme.txt";
    fs_utils_read_to_string(path_string, &content_string);
    ASSERT_EQ(content_string.str, "This is a very good string!", "File read correctly.");
    String_destroy(&content_string);

    PRINT_TEST_TITLE("read to string - short text, use const char * for file path");
    fs_utils_read_to_string("test/assets/readme.txt", &content_string);
    ASSERT_EQ(content_string.str, "This is a very good string!", "File read correctly.");
    String_destroy(&content_string);

    PRINT_TEST_TITLE("read to string - long text");
    path_string = "test/assets/readme-long.txt";
    fs_utils_read_to_string(path_string, &content_string);
    ASSERT_EQ(content_string.length, 16599, "Read string size matches.");
    String_destroy(&content_string);

    PRINT_TEST_TITLE("append to initially missing file");
    path_string    = "test/artifacts/new-file.txt";
    content_char_p = "this is new\n";
    fs_utils_append(path_string, content_char_p);
    fs_utils_append(path_string, content_char_p);
    fs_utils_read_to_string(path_string, &content_string);
    ASSERT_EQ(
        content_string.str, "this is new\nthis is new\n", "File created and modified correctly");
    String_destroy(&content_string);

    PRINT_TEST_TITLE("create with content string");
    {
        path_string    = "test/artifacts/new-file-2.txt";
        content_char_p = "this is new\n";
        // First time - create.
        fs_utils_create_with_content(path_string, content_char_p);
        // Second time - overwrite.
        fs_utils_create_with_content(path_string, content_char_p);
        ASSERT(fs_utils_is_file(path_string), "Created file found.");
        fs_utils_read_to_string(path_string, &content_string);
        ASSERT_EQ(content_string.str, "this is new\n", "File created and modified correctly");
        String_destroy(&content_string);
    }

    PRINT_TEST_TITLE("Get file size");
    {
        path_string = "test/assets/readme.txt";
        off_t file_size;
        ASSERT(fs_utils_get_file_size(path_string, &file_size) == ERR_ALL_GOOD, "File found.");
        ASSERT(file_size == 27, "File size correct.");
    }
    /**/
}
#endif
