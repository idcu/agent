#include "idcu/os/os.h"
#include <rtthread.h>
#include <dfs_posix.h>
#include <string.h>

int idcu_file_read(const char* path, void* buf, size_t len, size_t* bytes_read)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    ssize_t read_bytes = read(fd, buf, len);
    if (read_bytes < 0) {
        close(fd);
        return -1;
    }
    if (bytes_read) {
        *bytes_read = (size_t)read_bytes;
    }
    close(fd);
    return 0;
}

int idcu_file_write(const char* path, const void* buf, size_t len, size_t* bytes_written)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return -1;
    }
    ssize_t written_bytes = write(fd, buf, len);
    if (written_bytes < 0) {
        close(fd);
        return -1;
    }
    if (bytes_written) {
        *bytes_written = (size_t)written_bytes;
    }
    close(fd);
    return 0;
}

int idcu_file_delete(const char* path)
{
    return unlink(path);
}

int idcu_file_exists(const char* path)
{
    return access(path, F_OK) == 0 ? 1 : 0;
}

int idcu_mkdir(const char* path)
{
    return mkdir(path, 0755);
}

int idcu_mkdir_p(const char* path)
{
    char* tmp = strdup(path);
    if (!tmp) {
        return -1;
    }
    
    char* p = tmp;
    while (*p) {
        if (*p == '/' && p != tmp) {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
        p++;
    }
    
    int result = mkdir(tmp, 0755);
    free(tmp);
    return result;
}
