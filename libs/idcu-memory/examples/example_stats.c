#include "idcu/memory/memory_pool.h"
#include &lt;stdio.h&gt;

int main(void) {
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&amp;pool);
    
    printf("=== Memory Pool Statistics ===\n");
    
    // Check initial stats
    uint32_t free_64 = idcu_mem_pool_get_free_count(&amp;pool, 64);
    printf("Initial free blocks (64 bytes): %u\n", free_64);
    
    // Allocate some blocks
    void* ptrs[10];
    for (int i = 0; i &lt; 10; i++) {
        ptrs[i] = idcu_mem_pool_alloc(&amp;pool, 64);
    }
    
    // Check stats after allocation
    free_64 = idcu_mem_pool_get_free_count(&amp;pool, 64);
    printf("Free blocks after allocating 10: %u\n", free_64);
    
    uint64_t total = idcu_mem_pool_get_total_allocated(&amp;pool);
    printf("Total allocated: %llu bytes\n", (unsigned long long)total);
    
    uint64_t peak = idcu_mem_pool_get_peak_usage(&amp;pool);
    printf("Peak usage: %llu bytes\n", (unsigned long long)peak);
    
    // Free half of them
    for (int i = 0; i &lt; 5; i++) {
        idcu_mem_pool_free(&amp;pool, ptrs[i]);
    }
    
    // Check stats after partial free
    free_64 = idcu_mem_pool_get_free_count(&amp;pool, 64);
    printf("Free blocks after freeing 5: %u\n", free_64);
    
    // Free the rest
    for (int i = 5; i &lt; 10; i++) {
        idcu_mem_pool_free(&amp;pool, ptrs[i]);
    }
    
    idcu_mem_pool_destroy(&amp;pool);
    printf("Done!\n");
    
    return 0;
}
