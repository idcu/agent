#include "idcu/memory/memory_pool.h"
#include &lt;stdio.h&gt;
#include &lt;string.h&gt;

int main(void) {
    idcu_MemoryPool pool;
    int ret = idcu_mem_pool_init(&amp;pool);
    if (ret != IDCU_ERR_SUCCESS) {
        fprintf(stderr, "Failed to initialize memory pool\n");
        return 1;
    }
    
    printf("Memory pool initialized successfully\n");
    
    // Allocate some memory
    void* ptr1 = idcu_mem_pool_alloc(&amp;pool, 64);
    if (ptr1) {
        strcpy((char*)ptr1, "Hello, memory pool!");
        printf("Allocated 64 bytes: %s\n", (char*)ptr1);
    }
    
    void* ptr2 = idcu_mem_pool_alloc(&amp;pool, 128);
    if (ptr2) {
        strcpy((char*)ptr2, "Another block");
        printf("Allocated 128 bytes: %s\n", (char*)ptr2);
    }
    
    // Free the memory
    if (ptr1) {
        idcu_mem_pool_free(&amp;pool, ptr1);
        printf("Freed 64 bytes\n");
    }
    
    if (ptr2) {
        idcu_mem_pool_free(&amp;pool, ptr2);
        printf("Freed 128 bytes\n");
    }
    
    // Cleanup
    idcu_mem_pool_destroy(&amp;pool);
    printf("Memory pool destroyed\n");
    
    return 0;
}
