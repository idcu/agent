#include "idcu/os/os.h"
#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

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
    
    if (len1 > 0 && len2 > 0 && buf[len1 - 1] != '\\' && buf[len1 - 1] != '/') {
        strcat(buf, "\\");
    }
    
    if (path2) {
        strcat(buf, path2);
    }
    
    return 0;
}

int idcu_exe_path(char* buf, size_t buf_len)
{
    DWORD len = GetModuleFileNameA(NULL, buf, (DWORD)buf_len);
    return len > 0 && len < buf_len ? 0 : -1;
}

int idcu_data_dir(char* buf, size_t buf_len)
{
    HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, buf);
    if (FAILED(hr)) {
        return -1;
    }
    
    size_t len = strlen(buf);
    if (len + 12 > buf_len) {
        return -1;
    }
    strcat(buf, "\\idcu");
    return 0;
}

int idcu_home_dir(char* buf, size_t buf_len)
{
    HRESULT hr = SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, buf);
    return SUCCEEDED(hr) ? 0 : -1;
}
