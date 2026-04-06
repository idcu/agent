#include "test/test_framework.h"
#include "memory_pool.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_memory_pool_init_destroy(void) {
    idcu_MemoryPool pool;
    int ret = idcu_mem_pool_init(&pool);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_mem_pool_init should succeed");
    
    idcu_mem_pool_destroy(&pool);
    IDCU_TEST_PASS();
}

static void test_memory_pool_alloc_free(void) {
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&pool);
    
    void* ptr1 = idcu_mem_pool_alloc(&pool, 256);
    IDCU_TEST_ASSERT(ptr1 != NULL, "Alloc should succeed");
    
    void* ptr2 = idcu_mem_pool_alloc(&pool, 256);
    IDCU_TEST_ASSERT(ptr2 != NULL, "Alloc 2 should succeed");
    
    idcu_mem_pool_free(&pool, ptr1);
    
    idcu_mem_pool_free(&pool, ptr2);
    
    idcu_mem_pool_destroy(&pool);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Memory Pool Tests");
    
    idcu_test_suite_add_test(&g_suite, "memory_pool_init_destroy", test_memory_pool_init_destroy);
    idcu_test_suite_add_test(&g_suite, "memory_pool_alloc_free", test_memory_pool_alloc_free);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
