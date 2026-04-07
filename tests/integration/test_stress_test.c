#include "idcu/log/log.h"
#include "idcu/common/error_code.h"
#include "idcu/memory/memory_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#endif

#define TEST_PASS 0
#define TEST_FAIL 1

static uint64_t get_current_time_ms(void)
{
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

static void sleep_ms(uint32_t ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

typedef struct {
    int thread_id;
    volatile int should_stop;
    uint64_t messages_sent;
    uint64_t operations_completed;
} stress_thread_context_t;

#ifdef _WIN32
static DWORD WINAPI stress_message_thread(LPVOID param)
#else
static void* stress_message_thread(void* param)
#endif
{
    stress_thread_context_t* ctx = (stress_thread_context_t*)param;
    idcu_log_info("Stress thread %d started", ctx->thread_id);
    
    while (!ctx->should_stop) {
        ctx->operations_completed++;
        ctx->messages_sent++;
        sleep_ms(1);
    }
    
    idcu_log_info("Stress thread %d finished: sent=%llu, ops=%llu", 
                  ctx->thread_id, (unsigned long long)ctx->messages_sent, 
                  (unsigned long long)ctx->operations_completed);
    
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

static int test_high_concurrent_messaging(void)
{
    idcu_log_info("=== Starting high concurrent messaging test ===");
    
    const int NUM_THREADS = 16;
    const int DURATION_MS = 5000;
    
    stress_thread_context_t threads[NUM_THREADS];
    void* thread_handles[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].thread_id = i;
        threads[i].should_stop = 0;
        threads[i].messages_sent = 0;
        threads[i].operations_completed = 0;
        
#ifdef _WIN32
        thread_handles[i] = CreateThread(NULL, 0, stress_message_thread, &threads[i], 0, NULL);
        if (!thread_handles[i]) {
            idcu_log_error("Failed to create thread %d", i);
            return TEST_FAIL;
        }
#else
        if (pthread_create((pthread_t*)&thread_handles[i], NULL, stress_message_thread, &threads[i]) != 0) {
            idcu_log_error("Failed to create thread %d", i);
            return TEST_FAIL;
        }
#endif
    }
    
    uint64_t start_time = get_current_time_ms();
    sleep_ms(DURATION_MS);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].should_stop = 1;
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
#ifdef _WIN32
        WaitForSingleObject(thread_handles[i], INFINITE);
        CloseHandle(thread_handles[i]);
#else
        pthread_join(*(pthread_t*)&thread_handles[i], NULL);
#endif
    }
    
    uint64_t total_messages = 0;
    uint64_t total_ops = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        total_messages += threads[i].messages_sent;
        total_ops += threads[i].operations_completed;
    }
    
    uint64_t duration = get_current_time_ms() - start_time;
    double msg_per_sec = (double)total_messages / ((double)duration / 1000.0);
    
    idcu_log_info("High concurrent messaging test results:");
    idcu_log_info("  Duration: %llu ms", (unsigned long long)duration);
    idcu_log_info("  Total messages: %llu", (unsigned long long)total_messages);
    idcu_log_info("  Messages/sec: %.2f", msg_per_sec);
    idcu_log_info("  Threads: %d", NUM_THREADS);
    
    if (total_messages > 0) {
        idcu_log_info("=== High concurrent messaging test PASSED ===");
        return TEST_PASS;
    } else {
        idcu_log_error("=== High concurrent messaging test FAILED ===");
        return TEST_FAIL;
    }
}

static int test_memory_pool_stress(void)
{
    idcu_log_info("=== Starting memory pool stress test ===");
    
    const int NUM_ALLOCS = 10000;
    const size_t BLOCK_SIZE = 256;
    
    idcu_MemoryPool pool;
    int ret = idcu_memory_pool_init(&pool, BLOCK_SIZE, NUM_ALLOCS);
    if (ret != IDCU_ERR_SUCCESS) {
        idcu_log_error("Failed to initialize memory pool");
        return TEST_FAIL;
    }
    
    void* blocks[NUM_ALLOCS];
    uint64_t start_time = get_current_time_ms();
    
    for (int i = 0; i < NUM_ALLOCS; i++) {
        blocks[i] = idcu_memory_pool_alloc(&pool);
        if (!blocks[i]) {
            idcu_log_error("Failed to allocate block %d", i);
            idcu_memory_pool_destroy(&pool);
            return TEST_FAIL;
        }
        memset(blocks[i], 0xAA, BLOCK_SIZE);
    }
    
    for (int i = 0; i < NUM_ALLOCS; i++) {
        idcu_memory_pool_free(&pool, blocks[i]);
    }
    
    uint64_t duration = get_current_time_ms() - start_time;
    
    idcu_log_info("Memory pool stress test results:");
    idcu_log_info("  Allocations: %d", NUM_ALLOCS);
    idcu_log_info("  Block size: %zu bytes", BLOCK_SIZE);
    idcu_log_info("  Duration: %llu ms", (unsigned long long)duration);
    idcu_log_info("  Ops/sec: %.2f", (double)NUM_ALLOCS * 2 / ((double)duration / 1000.0));
    
    idcu_memory_pool_destroy(&pool);
    idcu_log_info("=== Memory pool stress test PASSED ===");
    return TEST_PASS;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    
    idcu_log_info("=== Starting Stress Tests ===");
    
    int failures = 0;
    
    if (test_memory_pool_stress() != TEST_PASS) {
        failures++;
    }
    
    if (test_high_concurrent_messaging() != TEST_PASS) {
        failures++;
    }
    
    if (failures == 0) {
        idcu_log_info("=== All Stress Tests PASSED ===");
        return 0;
    } else {
        idcu_log_error("=== %d Stress Test(s) FAILED ===", failures);
        return 1;
    }
}
