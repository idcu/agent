#include <idcu/common/vector.h>
#include <idcu/common/hash_map.h>
#include <idcu/common/error_code.h>
#include <idcu/msgbus/msgbus.h>
#include <idcu/memory/memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#define BENCHMARK_ITERATIONS 100000
#define BENCHMARK_MSG_ITERATIONS 10000
#define BENCHMARK_MEM_ITERATIONS 100000

static uint64_t get_time_ms(void) {
    return (uint64_t)clock() * 1000 / CLOCKS_PER_SEC;
}

static void benchmark_vector_push(void) {
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int));
    
    uint64_t start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        idcu_vector_push(&vec, &i);
    }
    uint64_t elapsed = get_time_ms() - start;
    
    printf("Vector push: %d iterations in %" PRIu64 " ms (%.2f ops/ms)\n",
           BENCHMARK_ITERATIONS, elapsed, (double)BENCHMARK_ITERATIONS / elapsed);
    
    idcu_vector_destroy(&vec);
}

static void benchmark_vector_get(void) {
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int));
    
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        idcu_vector_push(&vec, &i);
    }
    
    uint64_t start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        volatile int* val = (int*)idcu_vector_get(&vec, i);
        (void)val;
    }
    uint64_t elapsed = get_time_ms() - start;
    
    printf("Vector get: %d iterations in %" PRIu64 " ms (%.2f ops/ms)\n",
           BENCHMARK_ITERATIONS, elapsed, (double)BENCHMARK_ITERATIONS / elapsed);
    
    idcu_vector_destroy(&vec);
}

static void benchmark_hash_map_set(void) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, 0, sizeof(int));
    
    char key[32];
    int value = 42;
    
    uint64_t start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        idcu_hash_map_put(&map, key, &value);
    }
    uint64_t elapsed = get_time_ms() - start;
    
    printf("Hash map set: %d iterations in %" PRIu64 " ms (%.2f ops/ms)\n",
           BENCHMARK_ITERATIONS, elapsed, (double)BENCHMARK_ITERATIONS / elapsed);
    
    idcu_hash_map_destroy(&map);
}

static void benchmark_hash_map_get(void) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, 0, sizeof(int));
    
    char key[32];
    int value = 42;
    
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        idcu_hash_map_put(&map, key, &value);
    }
    
    uint64_t start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        volatile int val;
        idcu_hash_map_get(&map, key, &val);
        (void)val;
    }
    uint64_t elapsed = get_time_ms() - start;
    
    printf("Hash map get: %d iterations in %" PRIu64 " ms (%.2f ops/ms)\n",
           BENCHMARK_ITERATIONS, elapsed, (double)BENCHMARK_ITERATIONS / elapsed);
    
    idcu_hash_map_destroy(&map);
}

static int g_msg_received_count = 0;

static void test_msg_handler(idcu_MsgTopic topic, const void* data, size_t data_size, void* user_data) {
    (void)topic;
    (void)data;
    (void)data_size;
    (void)user_data;
    g_msg_received_count++;
}

static void benchmark_msgbus_publish(void) {
    idcu_MsgBus* bus = NULL;
    idcu_msgbus_init(&bus);
    
    idcu_MsgSubscriber* subscriber = NULL;
    idcu_msgbus_subscribe(bus, 100, test_msg_handler, NULL, &subscriber);
    
    int test_data = 42;
    g_msg_received_count = 0;
    
    uint64_t start = get_time_ms();
    for (int i = 0; i < BENCHMARK_MSG_ITERATIONS; i++) {
        idcu_msgbus_publish(bus, 100, &test_data, sizeof(test_data), IDCU_MSG_PRIORITY_NORMAL);
    }
    uint64_t elapsed = get_time_ms() - start;
    
    printf("Message bus publish: %d iterations in %" PRIu64 " ms (%.2f ops/ms)\n",
           BENCHMARK_MSG_ITERATIONS, elapsed, (double)BENCHMARK_MSG_ITERATIONS / elapsed);
    
    idcu_msgbus_unsubscribe(bus, subscriber);
    idcu_msgbus_destroy(bus);
}

static void benchmark_msgbus_process(void) {
    idcu_MsgBus* bus = NULL;
    idcu_msgbus_init(&bus);
    
    idcu_MsgSubscriber* subscriber = NULL;
    idcu_msgbus_subscribe(bus, 200, test_msg_handler, NULL, &subscriber);
    
    int test_data = 42;
    for (int i = 0; i < BENCHMARK_MSG_ITERATIONS; i++) {
        idcu_msgbus_publish(bus, 200, &test_data, sizeof(test_data), IDCU_MSG_PRIORITY_NORMAL);
    }
    
    g_msg_received_count = 0;
    uint64_t start = get_time_ms();
    idcu_msgbus_process(bus);
    uint64_t elapsed = get_time_ms() - start;
    
    printf("Message bus process: %d messages in %" PRIu64 " ms (%.2f msgs/ms, received: %d)\n",
           BENCHMARK_MSG_ITERATIONS, elapsed, (double)BENCHMARK_MSG_ITERATIONS / elapsed, g_msg_received_count);
    
    idcu_msgbus_unsubscribe(bus, subscriber);
    idcu_msgbus_destroy(bus);
}

static void benchmark_memory_pool_alloc(void) {
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&pool);
    
    uint64_t start = get_time_ms();
    void* ptrs[BENCHMARK_MEM_ITERATIONS];
    for (int i = 0; i < BENCHMARK_MEM_ITERATIONS; i++) {
        ptrs[i] = idcu_mem_pool_alloc(&pool, 64);
    }
    uint64_t elapsed = get_time_ms() - start;
    
    printf("Memory pool alloc: %d iterations in %" PRIu64 " ms (%.2f ops/ms)\n",
           BENCHMARK_MEM_ITERATIONS, elapsed, (double)BENCHMARK_MEM_ITERATIONS / elapsed);
    
    for (int i = 0; i < BENCHMARK_MEM_ITERATIONS; i++) {
        idcu_mem_pool_free(&pool, ptrs[i]);
    }
    
    idcu_mem_pool_destroy(&pool);
}

static void benchmark_memory_pool_free(void) {
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&pool);
    
    void* ptrs[BENCHMARK_MEM_ITERATIONS];
    for (int i = 0; i < BENCHMARK_MEM_ITERATIONS; i++) {
        ptrs[i] = idcu_mem_pool_alloc(&pool, 64);
    }
    
    uint64_t start = get_time_ms();
    for (int i = 0; i < BENCHMARK_MEM_ITERATIONS; i++) {
        idcu_mem_pool_free(&pool, ptrs[i]);
    }
    uint64_t elapsed = get_time_ms() - start;
    
    printf("Memory pool free: %d iterations in %" PRIu64 " ms (%.2f ops/ms)\n",
           BENCHMARK_MEM_ITERATIONS, elapsed, (double)BENCHMARK_MEM_ITERATIONS / elapsed);
    
    idcu_mem_pool_destroy(&pool);
}

int main(void) {
    printf("=== IDCU Common Library Benchmarks ===\n");
    printf("Iterations: %d\n\n", BENCHMARK_ITERATIONS);
    
    benchmark_vector_push();
    benchmark_vector_get();
    benchmark_hash_map_set();
    benchmark_hash_map_get();
    
    printf("\n=== Message Bus Benchmarks ===\n");
    printf("Iterations: %d\n\n", BENCHMARK_MSG_ITERATIONS);
    
    benchmark_msgbus_publish();
    benchmark_msgbus_process();
    
    printf("\n=== Memory Pool Benchmarks ===\n");
    printf("Iterations: %d\n\n", BENCHMARK_MEM_ITERATIONS);
    
    benchmark_memory_pool_alloc();
    benchmark_memory_pool_free();
    
    printf("\n=== Benchmarks Complete ===\n");
    return 0;
}
