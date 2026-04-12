#include "idcu/os.h"
#include <windows.h>
#include <shlwapi.h>
#include <string.h>
#include <stdlib.h>

#pragma comment(lib, "shlwapi.lib")

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
    if (len1 > 0 && buf[len1 - 1] != '\\' && buf[len1 - 1] != '/') {
        buf[len1] = '\\';
        buf[len1 + 1] = '\0';
    }
    strcat(buf, path2);
    return 0;
}

int idcu_exe_path(char* buf, size_t buf_len)
{
    if (GetModuleFileNameA(NULL, buf, (DWORD)buf_len) == 0) {
        return -1;
    }
    return 0;
}

int idcu_data_dir(char* buf, size_t buf_len)
{
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, buf) != S_OK) {
        return -1;
    }
    size_t len = strlen(buf);
    const char* suffix = "\\idcu";
    size_t suffix_len = strlen(suffix);
    if (len + suffix_len + 1 > buf_len) {
        return -1;
    }
    strcat(buf, suffix);
    return 0;
}

int idcu_home_dir(char* buf, size_t buf_len)
{
    if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, buf) != S_OK) {
        return -1;
    }
    return 0;
}
