#include "idcu/os/os.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int idcu_file_read(const char* path, void* buf, size_t len, size_t* bytes_read)
{
    FILE* f = fopen(path, "rb");
    if (!f) {
        return -1;
    }
    size_t read = fread(buf, 1, len, f);
    if (bytes_read) {
        *bytes_read = read;
    }
    fclose(f);
    return 0;
}

int idcu_file_write(const char* path, const void* buf, size_t len, size_t* bytes_written)
{
    FILE* f = fopen(path, "wb");
    if (!f) {
        return -1;
    }
    size_t written = fwrite(buf, 1, len, f);
    if (bytes_written) {
        *bytes_written = written;
    }
    fclose(f);
    return 0;
}

int idcu_file_delete(const char* path)
{
    return DeleteFileA(path) ? 0 : -1;
}

int idcu_file_exists(const char* path)
{
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) ? 1 : 0;
}

int idcu_mkdir(const char* path)
{
    return CreateDirectoryA(path, NULL) ? 0 : -1;
}

int idcu_mkdir_p(const char* path)
{
    char* tmp = _strdup(path);
    if (!tmp) {
        return -1;
    }
    
    char* p = tmp;
    while (*p) {
        if (*p == '\\' && p != tmp && *(p - 1) != ':') {
            *p = '\0';
            CreateDirectoryA(tmp, NULL);
            *p = '\\';
        } else if (*p == '/' && p != tmp) {
            *p = '\0';
            CreateDirectoryA(tmp, NULL);
            *p = '/';
        }
        p++;
    }
    
    int result = CreateDirectoryA(tmp, NULL) ? 0 : -1;
    free(tmp);
    return result;
}
