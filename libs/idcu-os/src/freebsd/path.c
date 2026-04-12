#include "idcu/os/os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/sysctl.h>

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
    int mib[4];
    size_t size = buf_len;
    
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = -1;
    
    if (sysctl(mib, 4, buf, &size, NULL, 0) == -1) {
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
    
    if (snprintf(buf, buf_len, "%s/.local/share/idcu", home) >= (int)buf_len) {
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
