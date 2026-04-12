#include "idcu/os/os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <mach-o/dyld.h>

int idcu_path_join(char* buf, size_t buf_len, const char* path1, const char* path2)
{
    if (!buf || buf_len == 0) {
        return -1;
    }
    
    size_t len1 = path1 ? strlen(path1) : 0;
    size_t len2 = path2 ? strlen(path2) : 0;
    
    if (len1 + len2 + 2 > buf_len) {
        return -1;
    }
    
    if (path1) {
        strcpy(buf, path1);
    } else {
        buf[0] = '\0';
    }
    
    if (len1 > 0 && len2 > 0 && buf[len1 - 1] != '/') {
        strcat(buf, "/");
    }
    
    if (path2) {
        strcat(buf, path2);
    }
    
    return 0;
}

int idcu_exe_path(char* buf, size_t buf_len)
{
    uint32_t size = (uint32_t)buf_len;
    if (_NSGetExecutablePath(buf, &size) != 0) {
        return -1;
    }
    return 0;
}

int idcu_data_dir(char* buf, size_t buf_len)
{
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (!pw) {
            return -1;
        }
        home = pw->pw_dir;
    }
    
    if (snprintf(buf, buf_len, "%s/Library/Application Support/idcu", home) >= (int)buf_len) {
        return -1;
    }
    return 0;
}

int idcu_home_dir(char* buf, size_t buf_len)
{
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (!pw) {
            return -1;
        }
        home = pw->pw_dir;
    }
    
    if (strlen(home) >= buf_len) {
        return -1;
    }
    strcpy(buf, home);
    return 0;
}
