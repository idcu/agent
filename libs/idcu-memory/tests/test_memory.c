#include <idcu/memory/memory.h>
#include <idcu/testframework/testframework.h>
#include <stdio.h>
#include <string.h>

static idcu_MemoryPool pool;

TEST_CASE(memory_pool_init_destroy)
{
    int ret = idcu_mem_pool_init(&pool);
    TEST_ASSERT_EQUAL(ret, IDCU_ERR_OK);
    idcu_mem_pool_destroy(&pool);
    TEST_PASS();
}

TEST_CASE(memory_pool_alloc_free)
{
    int ret = idcu_mem_pool_init(&pool);
    TEST_ASSERT_EQUAL(ret, IDCU_ERR_OK);
    
    // 测试分配不同尺寸
    void* ptr1 = idcu_mem_pool_alloc(&pool, 10);
    TEST_ASSERT_NOT_NULL(ptr1);
    
    void* ptr2 = idcu_mem_pool_alloc(&pool, 50);
    TEST_ASSERT_NOT_NULL(ptr2);
    
    void* ptr3 = idcu_mem_pool_alloc(&pool, 200);
    TEST_ASSERT_NOT_NULL(ptr3);
    
    // 测试写入和读取
    if (ptr1) {
        strcpy((char*)ptr1, "test123");
        TEST_ASSERT_EQUAL_STRING((char*)ptr1, "test123");
    }
    
    // 释放
    idcu_mem_pool_free(&pool, ptr1);
    idcu_mem_pool_free(&pool, ptr2);
    idcu_mem_pool_free(&pool, ptr3);
    
    idcu_mem_pool_destroy(&pool);
    TEST_PASS();
}

TEST_CASE(memory_pool_statistics)
{
    int ret = idcu_mem_pool_init(&pool);
    TEST_ASSERT_EQUAL(ret, IDCU_ERR_OK);
    
    uint32_t free_count = idcu_mem_pool_get_free_count(&pool, 10);
    TEST_ASSERT_GREATER_THAN(free_count, 0);
    
    void* ptr = idcu_mem_pool_alloc(&pool, 10);
    TEST_ASSERT_NOT_NULL(ptr);
    
    uint64_t total_alloc = idcu_mem_pool_get_total_allocated(&pool);
    TEST_ASSERT_GREATER_THAN(total_alloc, 0);
    
    idcu_mem_pool_free(&pool, ptr);
    
    uint64_t peak_usage = idcu_mem_pool_get_peak_usage(&pool);
    TEST_ASSERT_GREATER_THAN(peak_usage, 0);
    
    idcu_mem_pool_destroy(&pool);
    TEST_PASS();
}

TEST_CASE(memory_pool_safety_checks)
{
    int ret = idcu_mem_check_null(NULL, "test");
    TEST_ASSERT_EQUAL(ret, IDCU_ERR_INVALID_ARG);
    
    int dummy;
    ret = idcu_mem_check_null(&dummy, "test");
    TEST_ASSERT_EQUAL(ret, IDCU_ERR_OK);
    
    char dst[10];
    char src[] = "hello";
    ret = idcu_mem_safe_copy(dst, sizeof(dst), src, strlen(src));
    TEST_ASSERT_EQUAL(ret, IDCU_ERR_OK);
    TEST_ASSERT_EQUAL_STRING(dst, "hello");
    
    TEST_PASS();
}

TEST_SUITE_BEGIN(memory_pool_tests)
    TEST_SUITE_ADD_TEST(memory_pool_init_destroy)
    TEST_SUITE_ADD_TEST(memory_pool_alloc_free)
    TEST_SUITE_ADD_TEST(memory_pool_statistics)
    TEST_SUITE_ADD_TEST(memory_pool_safety_checks)
TEST_SUITE_END()

int main(void)
{
    return RUN_TEST_SUITE(memory_pool_tests);
}
