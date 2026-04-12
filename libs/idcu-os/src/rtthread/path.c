#include "idcu/os/os.h"
#include <rtthread.h>
#include <string.h>
#include <stdio.h>

int idcu_exe_path(char* buf, size_t buf_len)
{
    if (!buf || buf_len == 0) {
        return -1;
    }
    strncpy(buf, "/", buf_len - 1);
    buf[buf_len - 1] = '\0';
    return 0;
}

int idcu_home_dir(char* buf, size_t buf_len)
{
    if (!buf || buf_len == 0) {
        return -1;
    }
    strncpy(buf, "/root", buf_len - 1);
    buf[buf_len - 1] = '\0';
    return 0;
}

int idcu_data_dir(char* buf, size_t buf_len)
{
    if (!buf || buf_len == 0) {
        return -1;
    }
    strncpy(buf, "/data", buf_len - 1);
    buf[buf_len - 1] = '\0';
    return 0;
}

int idcu_path_join(char* buf, size_t buf_len, const char* a, const char* b)
{
    if (!buf || buf_len == 0 || !a || !b) {
        return -1;
    }
    size_t a_len = strlen(a);
    size_t b_len = strlen(b);
    size_t total_len = a_len + 1 + b_len + 1;
    
    if (total_len > buf_len) {
        return -1;
    }
    
    strcpy(buf, a);
    if (a_len > 0 && buf[a_len - 1] != '/') {
        strcat(buf, "/");
    }
    if (b[0] == '/') {
        strcat(buf, b + 1);
    } else {
        strcat(buf, b);
    }
    
    return 0;
}
