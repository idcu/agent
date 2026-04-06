#include "idcu/memory/memory_pool.h"
#include "idcu/common/error_code.h"
#include &lt;stdio.h&gt;
#include &lt;string.h&gt;
#include &lt;stdlib.h&gt;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s\n", msg); \
        return 1; \
    } \
} while(0)

static int test_memory_pool_init_destroy(void) {
    idcu_MemoryPool pool;
    int ret = idcu_mem_pool_init(&amp;pool);
    TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "idcu_mem_pool_init should succeed");
    
    idcu_mem_pool_destroy(&amp;pool);
    printf("PASS: test_memory_pool_init_destroy\n");
    return 0;
}

static int test_memory_pool_alloc_free(void) {
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&amp;pool);
    
    void* ptr1 = idcu_mem_pool_alloc(&amp;pool, 256);
    TEST_ASSERT(ptr1 != NULL, "Alloc should succeed");
    
    void* ptr2 = idcu_mem_pool_alloc(&amp;pool, 256);
    TEST_ASSERT(ptr2 != NULL, "Alloc 2 should succeed");
    TEST_ASSERT(ptr1 != ptr2, "Pointers should be different");
    
    idcu_mem_pool_free(&amp;pool, ptr1);
    idcu_mem_pool_free(&amp;pool, ptr2);
    
    idcu_mem_pool_destroy(&amp;pool);
    printf("PASS: test_memory_pool_alloc_free\n");
    return 0;
}

static int test_memory_pool_stats(void) {
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&amp;pool);
    
    uint32_t free_count = idcu_mem_pool_get_free_count(&amp;pool, 256);
    TEST_ASSERT(free_count &gt; 0, "Should have free blocks");
    
    void* ptr = idcu_mem_pool_alloc(&amp;pool, 256);
    TEST_ASSERT(ptr != NULL, "Alloc should succeed");
    
    uint64_t total = idcu_mem_pool_get_total_allocated(&amp;pool);
    TEST_ASSERT(total &gt; 0, "Total allocated should be &gt; 0");
    
    uint64_t peak = idcu_mem_pool_get_peak_usage(&amp;pool);
    TEST_ASSERT(peak &gt; 0, "Peak usage should be &gt; 0");
    
    idcu_mem_pool_free(&amp;pool, ptr);
    idcu_mem_pool_destroy(&amp;pool);
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
