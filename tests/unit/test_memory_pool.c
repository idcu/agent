#include "test/test_framework.h"
#include "utils/memory_pool.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static void test_memory_pool_init_destroy(void) {
    MemoryPool pool;
    mp_init(&pool, 256, 10);
    TEST_ASSERT(pool.block_size == 256, "Block size should be 256");
    TEST_ASSERT(pool.total_blocks == 10, "Total blocks should be 10");
    TEST_ASSERT(pool.used_blocks == 0, "Used blocks should be 0");
    
    mp_destroy(&pool);
    TEST_PASS();
}

static void test_memory_pool_alloc_free(void) {
    MemoryPool pool;
    mp_init(&pool, 256, 5);
    
    void* ptr1 = mp_alloc(&pool);
    TEST_ASSERT(ptr1 != NULL, "Alloc should succeed");
    TEST_ASSERT(pool.used_blocks == 1, "Used blocks should be 1");
    
    void* ptr2 = mp_alloc(&pool);
    TEST_ASSERT(ptr2 != NULL, "Alloc 2 should succeed");
    TEST_ASSERT(pool.used_blocks == 2, "Used blocks should be 2");
    
    mp_free(&pool, ptr1);
    TEST_ASSERT(pool.used_blocks == 1, "Used blocks should be 1");
    
    mp_free(&pool, ptr2);
    TEST_ASSERT(pool.used_blocks == 0, "Used blocks should be 0");
    
    mp_destroy(&pool);
    TEST_PASS();
}

static void test_memory_pool_stats(void) {
    MemoryPool pool;
    mp_init(&pool, 256, 5);
    
    void* ptr1 = mp_alloc(&pool);
    void* ptr2 = mp_alloc(&pool);
    
    size_t used = mp_get_used(&pool);
    TEST_ASSERT(used == 2, "Used should be 2");
    
    size_t peak = mp_get_peak(&pool);
    TEST_ASSERT(peak == 2, "Peak should be 2");
    
    mp_free(&pool, ptr1);
    used = mp_get_used(&pool);
    TEST_ASSERT(used == 1, "Used should be 1");
    peak = mp_get_peak(&pool);
    TEST_ASSERT(peak == 2, "Peak should remain 2");
    
    mp_free(&pool, ptr2);
    mp_destroy(&pool);
    TEST_PASS();
}

static void test_global_memory_stats(void) {
    global_memory_stats_init();
    
    global_memory_alloc_track(100);
    global_memory_alloc_track(200);
    
    GlobalMemoryStats stats;
    global_memory_get_stats(&stats);
    TEST_ASSERT(stats.total_allocated == 300, "Total allocated should be 300");
    TEST_ASSERT(stats.current_usage == 300, "Current usage should be 300");
    
    global_memory_free_track(150);
    global_memory_get_stats(&stats);
    TEST_ASSERT(stats.total_freed == 150, "Total freed should be 150");
    TEST_ASSERT(stats.current_usage == 150, "Current usage should be 150");
    
    global_memory_stats_destroy();
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Memory Pool Tests");
    
    test_suite_add_test(&g_suite, "memory_pool_init_destroy", test_memory_pool_init_destroy);
    test_suite_add_test(&g_suite, "memory_pool_alloc_free", test_memory_pool_alloc_free);
    test_suite_add_test(&g_suite, "memory_pool_stats", test_memory_pool_stats);
    test_suite_add_test(&g_suite, "global_memory_stats", test_global_memory_stats);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
