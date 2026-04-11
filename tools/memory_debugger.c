#include "memory_debugger.h"
#include <idcu/common/vector.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static idcu_Vector allocations;
static int initialized = 0;
static size_t total_allocated = 0;

int idcu_memory_debugger_init(void) {
    if (initialized) {
        return 0;
    }
    
    idcu_vector_init(&allocations, sizeof(idcu_MemoryAllocation));
    initialized = 1;
    total_allocated = 0;
    return 0;
}

void idcu_memory_debugger_destroy(void) {
    if (!initialized) {
        return;
    }
    
    idcu_memory_debugger_print_leaks();
    
    for (size_t i = 0; i < allocations.size; i++) {
        idcu_MemoryAllocation* alloc = (idcu_MemoryAllocation*)idcu_vector_get(&allocations, i);
        if (alloc->in_use) {
            free(alloc->address);
        }
    }
    
    idcu_vector_destroy(&allocations);
    initialized = 0;
}

void* idcu_debug_malloc(size_t size, const char* file, int line) {
    if (!initialized) {
        idcu_memory_debugger_init();
    }
    
    void* ptr = malloc(size);
    if (!ptr) {
        return NULL;
    }
    
    idcu_MemoryAllocation alloc;
    alloc.address = ptr;
    alloc.size = size;
    alloc.file = file;
    alloc.line = line;
    alloc.timestamp = (uint64_t)time(NULL);
    alloc.in_use = 1;
    
    idcu_vector_push(&allocations, &alloc);
    total_allocated += size;
    
    return ptr;
}

void* idcu_debug_calloc(size_t num, size_t size, const char* file, int line) {
    if (!initialized) {
        idcu_memory_debugger_init();
    }
    
    void* ptr = calloc(num, size);
    if (!ptr) {
        return NULL;
    }
    
    idcu_MemoryAllocation alloc;
    alloc.address = ptr;
    alloc.size = num * size;
    alloc.file = file;
    alloc.line = line;
    alloc.timestamp = (uint64_t)time(NULL);
    alloc.in_use = 1;
    
    idcu_vector_push(&allocations, &alloc);
    total_allocated += num * size;
    
    return ptr;
}

void* idcu_debug_realloc(void* ptr, size_t size, const char* file, int line) {
    if (!initialized) {
        idcu_memory_debugger_init();
    }
    
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        return NULL;
    }
    
    if (ptr) {
        for (size_t i = 0; i < allocations.size; i++) {
            idcu_MemoryAllocation* alloc = (idcu_MemoryAllocation*)idcu_vector_get(&allocations, i);
            if (alloc->address == ptr && alloc->in_use) {
                total_allocated -= alloc->size;
                alloc->address = new_ptr;
                alloc->size = size;
                alloc->file = file;
                alloc->line = line;
                total_allocated += size;
                return new_ptr;
            }
        }
    }
    
    idcu_MemoryAllocation alloc;
    alloc.address = new_ptr;
    alloc.size = size;
    alloc.file = file;
    alloc.line = line;
    alloc.timestamp = (uint64_t)time(NULL);
    alloc.in_use = 1;
    
    idcu_vector_push(&allocations, &alloc);
    total_allocated += size;
    
    return new_ptr;
}

void idcu_debug_free(void* ptr, const char* file, int line) {
    (void)file;
    (void)line;
    
    if (!ptr || !initialized) {
        free(ptr);
        return;
    }
    
    for (size_t i = 0; i < allocations.size; i++) {
        idcu_MemoryAllocation* alloc = (idcu_MemoryAllocation*)idcu_vector_get(&allocations, i);
        if (alloc->address == ptr && alloc->in_use) {
            alloc->in_use = 0;
            total_allocated -= alloc->size;
            free(ptr);
            return;
        }
    }
    
    free(ptr);
}

size_t idcu_memory_debugger_get_allocated_count(void) {
    if (!initialized) {
        return 0;
    }
    
    size_t count = 0;
    for (size_t i = 0; i < allocations.size; i++) {
        idcu_MemoryAllocation* alloc = (idcu_MemoryAllocation*)idcu_vector_get(&allocations, i);
        if (alloc->in_use) {
            count++;
        }
    }
    return count;
}

size_t idcu_memory_debugger_get_total_allocated(void) {
    return total_allocated;
}

void idcu_memory_debugger_print_leaks(void) {
    if (!initialized) {
        return;
    }
    
    size_t leak_count = 0;
    size_t leak_size = 0;
    
    printf("=== Memory Leak Report ===\n");
    
    for (size_t i = 0; i < allocations.size; i++) {
        idcu_MemoryAllocation* alloc = (idcu_MemoryAllocation*)idcu_vector_get(&allocations, i);
        if (alloc->in_use) {
            leak_count++;
            leak_size += alloc->size;
            printf("  Leak #%zu: %p (%zu bytes) at %s:%d\n",
                   leak_count, alloc->address, alloc->size, alloc->file, alloc->line);
        }
    }
    
    if (leak_count == 0) {
        printf("  No memory leaks detected!\n");
    } else {
        printf("  Total: %zu leaks, %zu bytes\n", leak_count, leak_size);
    }
    
    printf("==========================\n");
}

void idcu_memory_debugger_reset(void) {
    if (!initialized) {
        return;
    }
    
    for (size_t i = 0; i < allocations.size; i++) {
        idcu_MemoryAllocation* alloc = (idcu_MemoryAllocation*)idcu_vector_get(&allocations, i);
        if (alloc->in_use) {
            free(alloc->address);
        }
    }
    
    idcu_vector_clear(&allocations);
    total_allocated = 0;
}
