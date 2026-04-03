#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler/msg_bus.h"
#include "scheduler/coroutine.h"
#include "utils/memory_pool.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

static idcu_MessageBus g_bus;
static idcu_CoroScheduler g_sched;
static idcu_MemoryPool g_pool;

static uint64_t get_timestamp_us(void)
{
#ifdef _WIN32
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (uint64_t)(count.QuadPart * 1000000 / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + (uint64_t)ts.tv_nsec / 1000;
#endif
}

static void bench_msg_send(uint64_t iterations) {
    idcu_StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    
    for (uint64_t i = 0; i < iterations; i++) {
        idcu_msg_send(&g_bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
    }
}

static void bench_msg_recv(uint64_t iterations) {
    idcu_Message msg;
    
    for (uint64_t i = 0; i < iterations; i++) {
        idcu_msg_recv(&g_bus, 2, &msg);
    }
}

static void bench_msg_batch_send(uint64_t iterations) {
    idcu_MessageBatch batch;
    idcu_StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    
    for (uint64_t i = 0; i < iterations; i += IDCU_MSG_BATCH_MAX) {
        batch.count = 0;
        for (uint32_t j = 0; j < IDCU_MSG_BATCH_MAX && (i + j) < iterations; j++) {
            batch.msgs[j].source_mod_id = 1;
            batch.msgs[j].target_mod_id = 2;
            batch.msgs[j].priority = IDCU_MSG_PRIO_NORMAL;
            batch.msgs[j].data = ctx;
            batch.msgs[j].payload = NULL;
            batch.count++;
        }
        idcu_msg_send_batch(&g_bus, &batch);
    }
}

static void bench_memory_pool_alloc(uint64_t iterations) {
    void* ptrs[1024];
    uint32_t ptr_count = 0;
    
    for (uint64_t i = 0; i < iterations; i++) {
        if (ptr_count < 1024) {
            ptrs[ptr_count++] = idcu_mem_pool_alloc(&g_pool, 64);
        } else {
            idcu_mem_pool_free(&g_pool, ptrs[0]);
            for (uint32_t j = 1; j < 1024; j++) {
                ptrs[j - 1] = ptrs[j];
            }
            ptrs[1023] = idcu_mem_pool_alloc(&g_pool, 64);
        }
    }
    
    for (uint32_t i = 0; i < ptr_count; i++) {
        idcu_mem_pool_free(&g_pool, ptrs[i]);
    }
}

static idcu_CoroState test_coro_func(idcu_Coroutine* coro) {
    (void)coro;
    return IDCU_CORO_FINISHED;
}

static void bench_coro_create(uint64_t iterations) {
    for (uint64_t i = 0; i < iterations; i++) {
        int id = idcu_coro_create(&g_sched, test_coro_func, 32, 10, NULL);
        if (id > 0) {
            idcu_coro_destroy(&g_sched, id);
        }
    }
}

typedef void (*BenchmarkFunc)(uint64_t);

typedef struct {
    const char* name;
    BenchmarkFunc func;
    uint64_t iterations;
    uint64_t duration_us;
} Benchmark;

#define MAX_BENCHMARKS 16

typedef struct {
    Benchmark benchmarks[MAX_BENCHMARKS];
    int count;
    const char* name;
} BenchmarkSuite;

static void bench_suite_init(BenchmarkSuite* suite, const char* name) {
    suite->name = name;
    suite->count = 0;
}

static void bench_suite_add_benchmark(BenchmarkSuite* suite, const char* name, BenchmarkFunc func, uint64_t iterations) {
    if (suite->count >= MAX_BENCHMARKS) return;
    suite->benchmarks[suite->count].name = name;
    suite->benchmarks[suite->count].func = func;
    suite->benchmarks[suite->count].iterations = iterations;
    suite->benchmarks[suite->count].duration_us = 0;
    suite->count++;
}

static void bench_suite_run(BenchmarkSuite* suite) {
    for (int i = 0; i < suite->count; i++) {
        Benchmark* b = &suite->benchmarks[i];
        uint64_t start = get_timestamp_us();
        b->func(b->iterations);
        uint64_t end = get_timestamp_us();
        b->duration_us = end - start;
    }
}

static void bench_suite_print_summary(BenchmarkSuite* suite) {
    printf("\n%s Benchmark Results:\n", suite->name);
    printf("--------------------------------------------------------\n");
    printf("%-20s %12s %12s %12s\n", "Benchmark", "Iterations", "Time(us)", "Ops/sec");
    printf("--------------------------------------------------------\n");
    
    for (int i = 0; i < suite->count; i++) {
        Benchmark* b = &suite->benchmarks[i];
        double ops_per_sec = (b->duration_us > 0) ? 
            (double)b->iterations * 1000000.0 / b->duration_us : 0.0;
        printf("%-20s %12llu %12llu %12.0f\n", 
               b->name, 
               (unsigned long long)b->iterations, 
               (unsigned long long)b->duration_us,
               ops_per_sec);
    }
    printf("--------------------------------------------------------\n");
}

int main(void) {
    printf("IDCU Agent - Performance Benchmarks\n");
    printf("===================================\n");
    
    idcu_msg_bus_init(&g_bus);
    idcu_coro_sched_init(&g_sched);
    idcu_mem_pool_init(&g_pool);
    
    BenchmarkSuite suite;
    bench_suite_init(&suite, "Core Performance");
    
    bench_suite_add_benchmark(&suite, "msg_send", bench_msg_send, 100000);
    bench_suite_add_benchmark(&suite, "msg_recv", bench_msg_recv, 100000);
    bench_suite_add_benchmark(&suite, "msg_batch_send", bench_msg_batch_send, 100000);
    bench_suite_add_benchmark(&suite, "memory_pool_alloc", bench_memory_pool_alloc, 100000);
    bench_suite_add_benchmark(&suite, "coro_create", bench_coro_create, 10000);
    
    bench_suite_run(&suite);
    bench_suite_print_summary(&suite);
    
    idcu_mem_pool_destroy(&g_pool);
    idcu_coro_sched_destroy(&g_sched);
    idcu_msg_bus_destroy(&g_bus);
    
    printf("\nBenchmark completed!\n");
    return 0;
}
