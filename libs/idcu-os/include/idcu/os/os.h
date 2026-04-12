#ifndef IDCU_OS_H
#define IDCU_OS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_OS_WINDOWS    defined(_WIN32)
#define IDCU_OS_LINUX      (defined(__linux__) && !defined(__ANDROID__))
#define IDCU_OS_MACOS      defined(__APPLE__)
#define IDCU_OS_FREEBSD    defined(__FreeBSD__)
#define IDCU_OS_ANDROID    defined(__ANDROID__)
#define IDCU_OS_HARMONY    (defined(__OHOS__) || defined(__HARMONYOS__))
#define IDCU_OS_VECTOR     defined(__VECTOROS__)
#define IDCU_OS_RTTHREAD   defined(__RTTHREAD__)

#define IDCU_OS_POSIX      (IDCU_OS_LINUX || IDCU_OS_MACOS || IDCU_OS_FREEBSD || \
                            IDCU_OS_ANDROID || IDCU_OS_HARMONY)
#define IDCU_OS_EMBEDDED   (IDCU_OS_VECTOR || IDCU_OS_RTTHREAD)

typedef struct idcu_mutex_t idcu_mutex_t;
typedef struct idcu_cond_t idcu_cond_t;
typedef struct idcu_rwlock_t idcu_rwlock_t;
typedef struct idcu_coro_t idcu_coro_t;

idcu_mutex_t* idcu_mutex_create(void);
void idcu_mutex_destroy(idcu_mutex_t* mutex);
int idcu_mutex_lock(idcu_mutex_t* mutex);
int idcu_mutex_unlock(idcu_mutex_t* mutex);
int idcu_mutex_trylock(idcu_mutex_t* mutex);

idcu_cond_t* idcu_cond_create(void);
void idcu_cond_destroy(idcu_cond_t* cond);
int idcu_cond_wait(idcu_cond_t* cond, idcu_mutex_t* mutex);
int idcu_cond_timedwait(idcu_cond_t* cond, idcu_mutex_t* mutex, uint32_t timeout_ms);
int idcu_cond_signal(idcu_cond_t* cond);
int idcu_cond_broadcast(idcu_cond_t* cond);

idcu_rwlock_t* idcu_rwlock_create(void);
void idcu_rwlock_destroy(idcu_rwlock_t* rwlock);
int idcu_rwlock_rdlock(idcu_rwlock_t* rwlock);
int idcu_rwlock_wrlock(idcu_rwlock_t* rwlock);
int idcu_rwlock_unlock(idcu_rwlock_t* rwlock);

typedef void (*idcu_coro_func_t)(void* arg);
idcu_coro_t* idcu_coro_create(idcu_coro_func_t func, void* arg, size_t stack_size);
void idcu_coro_destroy(idcu_coro_t* coro);
int idcu_coro_yield(void);
int idcu_coro_resume(idcu_coro_t* coro);
bool idcu_coro_is_done(idcu_coro_t* coro);

uint64_t idcu_time_now_ms(void);
uint64_t idcu_time_now_us(void);
uint64_t idcu_time_now_ns(void);
void idcu_sleep_ms(uint32_t ms);
void idcu_sleep_us(uint32_t us);

int idcu_sock_init(void);
void idcu_sock_cleanup(void);
int idcu_sock_close(int sock);
int idcu_sock_bind(int sock, const char* addr, uint16_t port);
int idcu_sock_listen(int sock, int backlog);
int idcu_sock_accept(int sock, char* client_addr, size_t addr_len, uint16_t* client_port);
int idcu_sock_connect(int sock, const char* addr, uint16_t port);
int idcu_sock_send(int sock, const void* data, size_t len);
int idcu_sock_recv(int sock, void* data, size_t len);
int idcu_sock_set_nonblocking(int sock, bool nonblocking);

typedef struct idcu_pollfd_t {
    int fd;
    short events;
    short revents;
} idcu_pollfd_t;

int idcu_poll(idcu_pollfd_t* fds, size_t nfds, int timeout_ms);

int idcu_file_read(const char* path, void* buf, size_t len, size_t* bytes_read);
int idcu_file_write(const char* path, const void* buf, size_t len, size_t* bytes_written);
int idcu_file_delete(const char* path);
int idcu_file_exists(const char* path);
int idcu_mkdir(const char* path);
int idcu_mkdir_p(const char* path);

int idcu_path_join(char* buf, size_t buf_len, const char* path1, const char* path2);
int idcu_exe_path(char* buf, size_t buf_len);
int idcu_data_dir(char* buf, size_t buf_len);
int idcu_home_dir(char* buf, size_t buf_len);

char* idcu_env_get(const char* name);
int idcu_env_set(const char* name, const char* value);
int idcu_env_del(const char* name);

typedef void (*idcu_signal_handler_t)(int signum);
int idcu_signal_register(int signum, idcu_signal_handler_t handler);

int idcu_backtrace_capture(void** frames, size_t max_frames);
int idcu_backtrace_format(void** frames, size_t num_frames, char* buf, size_t buf_len);

void* idcu_dlopen(const char* path);
void* idcu_dlsym(void* handle, const char* symbol);
int idcu_dlclose(void* handle);
const char* idcu_dlerror(void);

typedef struct idcu_sysinfo_t {
    uint64_t total_memory;
    uint64_t free_memory;
    uint64_t total_swap;
    uint64_t free_swap;
    uint32_t cpu_count;
    char os_name[64];
    char os_version[64];
} idcu_sysinfo_t;

typedef struct idcu_statvfs_t {
    uint64_t block_size;
    uint64_t total_blocks;
    uint64_t free_blocks;
    uint64_t available_blocks;
    uint64_t total_files;
    uint64_t free_files;
} idcu_statvfs_t;

int idcu_sysinfo_get(idcu_sysinfo_t* info);
int idcu_statvfs_get(const char* path, idcu_statvfs_t* statvfs);

#ifdef __cplusplus
}
#endif

#endif
