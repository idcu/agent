#include "idcu/os/os.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

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
    return 0;
}
