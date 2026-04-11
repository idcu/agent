#include <idcu/common/vector.h>
#include <idcu/common/hash_map.h>
#include <idcu/common/error_code.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BENCHMARK_ITERATIONS 100000

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
    idcu_hash_map_init(&map, sizeof(int));
    
    char key[32];
    int value = 42;
    
    uint64_t start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        idcu_hash_map_set(&map, key, &value);
    }
    uint64_t elapsed = get_time_ms() - start;
    
    printf("Hash map set: %d iterations in %" PRIu64 " ms (%.2f ops/ms)\n",
           BENCHMARK_ITERATIONS, elapsed, (double)BENCHMARK_ITERATIONS / elapsed);
    
    idcu_hash_map_destroy(&map);
}

static void benchmark_hash_map_get(void) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, sizeof(int));
    
    char key[32];
    int value = 42;
    
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        idcu_hash_map_set(&map, key, &value);
    }
    
    uint64_t start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        volatile int* val = (int*)idcu_hash_map_get(&map, key);
        (void)val;
    }
    uint64_t elapsed = get_time_ms() - start;
    
    printf("Hash map get: %d iterations in %" PRIu64 " ms (%.2f ops/ms)\n",
           BENCHMARK_ITERATIONS, elapsed, (double)BENCHMARK_ITERATIONS / elapsed);
    
    idcu_hash_map_destroy(&map);
}

int main(void) {
    printf("=== IDCU Common Library Benchmarks ===\n");
    printf("Iterations: %d\n\n", BENCHMARK_ITERATIONS);
    
    benchmark_vector_push();
    benchmark_vector_get();
    benchmark_hash_map_set();
    benchmark_hash_map_get();
    
    printf("\n=== Benchmarks Complete ===\n");
    return 0;
}
