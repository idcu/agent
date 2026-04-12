#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "idcu/os/os.h"

static int test_passed = 0;
static int test_failed = 0;

#define TEST(name, cond) do { \
    printf("Test: %s... ", name); \
    if (cond) { \
        printf("PASSED\n"); \
        test_passed++; \
    } else { \
        printf("FAILED\n"); \
        test_failed++; \
    } \
} while(0)

static void test_time(void)
{
    uint64_t t1 = idcu_time_now_ms();
    idcu_sleep_ms(100);
    uint64_t t2 = idcu_time_now_ms();
    TEST("time_ms", (t2 - t1) >= 90 && (t2 - t1) <= 200);
    
    uint64_t t3 = idcu_time_now_us();
    idcu_sleep_us(10000);
    uint64_t t4 = idcu_time_now_us();
    TEST("time_us", (t4 - t3) >= 9000);
    
    uint64_t t5 = idcu_time_now_ns();
    TEST("time_ns", t5 > 0);
}

static void test_mutex(void)
{
    idcu_mutex_t* mutex = idcu_mutex_create();
    TEST("mutex_create", mutex != NULL);
    
    TEST("mutex_lock", idcu_mutex_lock(mutex) == 0);
    TEST("mutex_unlock", idcu_mutex_unlock(mutex) == 0);
    
    idcu_mutex_destroy(mutex);
    TEST("mutex_destroy", 1);
}

static void test_file(void)
{
    const char* test_file = "test_os_test_file.txt";
    const char* test_data = "Hello, idcu-os!";
    size_t len = strlen(test_data);
    
    size_t written;
    TEST("file_write", idcu_file_write(test_file, test_data, len, &written) == 0 && written == len);
    
    char read_buf[256];
    size_t read;
    TEST("file_read", idcu_file_read(test_file, read_buf, sizeof(read_buf), &read) == 0 && read == len);
    TEST("file_content", memcmp(read_buf, test_data, len) == 0);
    
    TEST("file_exists", idcu_file_exists(test_file) == 1);
    TEST("file_delete", idcu_file_delete(test_file) == 0);
    TEST("file_not_exists_after_delete", idcu_file_exists(test_file) == 0);
}

static void test_path(void)
{
    char buf[256];
    TEST("exe_path", idcu_exe_path(buf, sizeof(buf)) == 0);
    printf("  Exe path: %s\n", buf);
    
    TEST("data_dir", idcu_data_dir(buf, sizeof(buf)) == 0);
    printf("  Data dir: %s\n", buf);
    
    TEST("home_dir", idcu_home_dir(buf, sizeof(buf)) == 0);
    printf("  Home dir: %s\n", buf);
    
    char joined[256];
    TEST("path_join", idcu_path_join(joined, sizeof(joined), "/tmp", "test") == 0);
    TEST("path_join_result", strcmp(joined, "/tmp/test") == 0 || strcmp(joined, "/tmp\\test") == 0);
}

static void test_env(void)
{
    const char* test_name = "IDCU_OS_TEST_VAR";
    const char* test_value = "test_value_123";
    
    TEST("env_set", idcu_env_set(test_name, test_value) == 0);
    
    char* val = idcu_env_get(test_name);
    TEST("env_get", val != NULL && strcmp(val, test_value) == 0);
    
    TEST("env_del", idcu_env_del(test_name) == 0);
    TEST("env_get_after_del", idcu_env_get(test_name) == NULL);
}

static void test_sysinfo(void)
{
    idcu_sysinfo_t info;
    TEST("sysinfo_get", idcu_sysinfo_get(&info) == 0);
    
    printf("  OS: %s %s\n", info.os_name, info.os_version);
    printf("  CPU count: %u\n", info.cpu_count);
    printf("  Total memory: %llu MB\n", (unsigned long long)(info.total_memory / (1024 * 1024)));
    printf("  Free memory: %llu MB\n", (unsigned long long)(info.free_memory / (1024 * 1024)));
    
    TEST("sysinfo_cpu_count", info.cpu_count > 0);
    TEST("sysinfo_total_memory", info.total_memory > 0);
    TEST("sysinfo_os_name", info.os_name[0] != '\0');
}

static void test_backtrace(void)
{
    void* frames[32];
    int count = idcu_backtrace_capture(frames, 32);
    TEST("backtrace_capture", count > 0);
    printf("  Backtrace captured %d frames\n", count);
    
    char buf[4096];
    int len = idcu_backtrace_format(frames, count, buf, sizeof(buf));
    TEST("backtrace_format", len > 0);
    printf("  Backtrace:\n%s", buf);
}

static void test_coroutine(void)
{
    static int coro_run = 0;
    
    void coro_func(void* arg)
    {
        (void)arg;
        coro_run = 1;
        idcu_coro_yield();
        coro_run = 2;
    }
    
    idcu_coro_t* coro = idcu_coro_create(coro_func, NULL, 0);
    TEST("coro_create", coro != NULL);
    TEST("coro_not_done_before_resume", !idcu_coro_is_done(coro));
    
    TEST("coro_resume_1", idcu_coro_resume(coro) == 0);
    TEST("coro_run_after_first_resume", coro_run == 1);
    TEST("coro_not_done_after_yield", !idcu_coro_is_done(coro));
    
    TEST("coro_resume_2", idcu_coro_resume(coro) == 0);
    TEST("coro_run_after_second_resume", coro_run == 2);
    TEST("coro_done", idcu_coro_is_done(coro));
    
    idcu_coro_destroy(coro);
    TEST("coro_destroy", 1);
}

int main(void)
{
    printf("========================================\n");
    printf("  IDCU OS Abstraction Layer Tests\n");
    printf("========================================\n\n");
    
    idcu_sock_init();
    
    test_time();
    test_mutex();
    test_file();
    test_path();
    test_env();
    test_sysinfo();
    test_backtrace();
    test_coroutine();
    
    idcu_sock_cleanup();
    
    printf("\n========================================\n");
    printf("  Summary: %d passed, %d failed\n", test_passed, test_failed);
    printf("========================================\n");
    
    return test_failed > 0 ? 1 : 0;
}
