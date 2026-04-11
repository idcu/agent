#include <idcu/memory/memory.h>
#include <idcu/testframework/testframework.h>
#include <stdio.h>
#include <string.h>

IDCU_TEST_CASE(memory, pool_init_destroy) {
    idcu_MemoryPool pool;
    int ret = idcu_mem_pool_init(&pool);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    idcu_mem_pool_destroy(&pool);
}

IDCU_TEST_CASE(memory, pool_alloc_free) {
    idcu_MemoryPool pool;
    int ret = idcu_mem_pool_init(&pool);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    void* ptr1 = idcu_mem_pool_alloc(&pool, 10);
    IDCU_TEST_ASSERT(ptr1 != NULL);
    
    void* ptr2 = idcu_mem_pool_alloc(&pool, 50);
    IDCU_TEST_ASSERT(ptr2 != NULL);
    
    void* ptr3 = idcu_mem_pool_alloc(&pool, 200);
    IDCU_TEST_ASSERT(ptr3 != NULL);
    
    if (ptr1) {
        strcpy((char*)ptr1, "test123");
        IDCU_TEST_ASSERT_STRING_EQUAL("test123", (char*)ptr1);
    }
    
    idcu_mem_pool_free(&pool, ptr1);
    idcu_mem_pool_free(&pool, ptr2);
    idcu_mem_pool_free(&pool, ptr3);
    
    idcu_mem_pool_destroy(&pool);
}

IDCU_TEST_CASE(memory, pool_statistics) {
    idcu_MemoryPool pool;
    int ret = idcu_mem_pool_init(&pool);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    uint32_t free_count = idcu_mem_pool_get_free_count(&pool, 10);
    IDCU_TEST_ASSERT(free_count > 0);
    
    void* ptr = idcu_mem_pool_alloc(&pool, 10);
    IDCU_TEST_ASSERT(ptr != NULL);
    
    uint64_t total_alloc = idcu_mem_pool_get_total_allocated(&pool);
    IDCU_TEST_ASSERT(total_alloc > 0);
    
    idcu_mem_pool_free(&pool, ptr);
    
    uint64_t peak_usage = idcu_mem_pool_get_peak_usage(&pool);
    IDCU_TEST_ASSERT(peak_usage > 0);
    
    idcu_mem_pool_destroy(&pool);
}

IDCU_TEST_CASE(memory, pool_safety_checks) {
    int ret = idcu_mem_check_null(NULL, "test");
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    int dummy;
    ret = idcu_mem_check_null(&dummy, "test");
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    char dst[10];
    char src[] = "hello";
    ret = idcu_mem_safe_copy(dst, sizeof(dst), src, strlen(src));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_STRING_EQUAL("hello", dst);
}

int main(void) {
    return idcu_test_run_all();
}
