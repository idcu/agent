#include "idcu/os.h"
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

int idcu_path_join(char* buf, size_t buf_len, const char* path1, const char* path2)
{
    if (!buf || buf_len == 0 || !path1 || !path2) {
        return -1;
    }
    size_t len1 = strlen(path1);
    size_t len2 = strlen(path2);
    if (len1 + len2 + 2 > buf_len) {
        return -1;
    }
    strcpy(buf, path1);
    if (len1 > 0 && buf[len1 - 1] != '/') {
        buf[len1] = '/';
        buf[len1 + 1] = '\0';
    }
    strcat(buf, path2);
    return 0;
}

int idcu_exe_path(char* buf, size_t buf_len)
{
    ssize_t len = readlink("/proc/self/exe", buf, buf_len - 1);
    if (len == -1) {
        return -1;
    }
    buf[len] = '\0';
    return 0;
}

int idcu_data_dir(char* buf, size_t buf_len)
{
    const char* home = getenv("HOME");
    if (!home) {
        return -1;
    }
    size_t home_len = strlen(home);
    const char* suffix = "/.idcu";
    size_t suffix_len = strlen(suffix);
    if (home_len + suffix_len + 1 > buf_len) {
        return -1;
    }
    strcpy(buf, home);
    strcat(buf, suffix);
    return 0;
}

int idcu_home_dir(char* buf, size_t buf_len)
{
    const char* home = getenv("HOME");
    if (!home) {
        return -1;
    }
    strncpy(buf, home, buf_len - 1);
    buf[buf_len - 1] = '\0';
    return 0;
}
