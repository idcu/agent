#include "idcu/memory/memory_pool.h"
#include "idcu/common/error_code.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s\n", msg); \
        return 1; \
    } \
} while(0)

static int test_memory_pool_init_destroy(void) {
    idcu_MemoryPool pool;
    int ret = idcu_mem_pool_init(&pool);
    TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "idcu_mem_pool_init should succeed");
    
    idcu_mem_pool_destroy(&pool);
    printf("PASS: test_memory_pool_init_destroy\n");
    return 0;
}

static int test_memory_pool_alloc_free(void) {
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&pool);
    
    void* ptr1 = idcu_mem_pool_alloc(&pool, 256);
    TEST_ASSERT(ptr1 != NULL, "Alloc should succeed");
    
    void* ptr2 = idcu_mem_pool_alloc(&pool, 256);
    TEST_ASSERT(ptr2 != NULL, "Alloc 2 should succeed");
    TEST_ASSERT(ptr1 != ptr2, "Pointers should be different");
    
    idcu_mem_pool_free(&pool, ptr1);
    idcu_mem_pool_free(&pool, ptr2);
    
    idcu_mem_pool_destroy(&pool);
    printf("PASS: test_memory_pool_alloc_free\n");
    return 0;
}

static int test_memory_pool_stats(void) {
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&pool);
    
    uint32_t free_count = idcu_mem_pool_get_free_count(&pool, 256);
    TEST_ASSERT(free_count > 0, "Should have free blocks");
    
    void* ptr = idcu_mem_pool_alloc(&pool, 256);
    TEST_ASSERT(ptr != NULL, "Alloc should succeed");
    
    uint64_t total = idcu_mem_pool_get_total_allocated(&pool);
    TEST_ASSERT(total > 0, "Total allocated should be > 0");
    
    uint64_t peak = idcu_mem_pool_get_peak_usage(&pool);
    TEST_ASSERT(peak > 0, "Peak usage should be > 0");
    
    idcu_mem_pool_free(&pool, ptr);
    idcu_mem_pool_destroy(&pool);
    printf("PASS: test_memory_pool_stats\n");
    return 0;
}

int main(void) {
    int failures = 0;
    
    if (test_memory_pool_init_destroy() != 0) failures++;
    if (test_memory_pool_alloc_free() != 0) failures++;
    if (test_memory_pool_stats() != 0) failures++;
    
    if (failures == 0) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("%d tests failed!\n", failures);
        return 1;
    }
}
