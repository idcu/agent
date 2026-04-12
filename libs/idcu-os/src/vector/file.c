#include "idcu/os.h"

int idcu_file_read(const char* path, void* buf, size_t len, size_t* bytes_read) { (void)path; (void)buf; (void)len; (void)bytes_read; return -1; }
int idcu_file_write(const char* path, const void* buf, size_t len, size_t* bytes_written) { (void)path; (void)buf; (void)len; (void)bytes_written; return -1; }
int idcu_file_delete(const char* path) { (void)path; return -1; }
int idcu_file_exists(const char* path) { (void)path; return 0; }
int idcu_mkdir(const char* path) { (void)path; return -1; }
int idcu_mkdir_p(const char* path) { (void)path; return -1; }
