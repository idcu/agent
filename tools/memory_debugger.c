#include "memory_debugger.h"
#include <idcu/common/vector.h>
#include <idcu/common/lock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <execinfo.h>
#endif

#define IDCU_MEM_MAX_SNAPSHOTS 10

static idcu_Vector allocations;
static idcu_Mutex debugger_mutex;
static int initialized = 0;
static size_t total_allocations = 0;
static size_t total_frees = 0;
static size_t current_bytes = 0;
static size_t peak_bytes = 0;
static size_t size_class_counts[IDCU_MEM_POOL_MAX_SIZE_CLASSES];
static size_t size_class_bytes[IDCU_MEM_POOL_MAX_SIZE_CLASSES];

typedef struct {
    uint64_t timestamp;
    size_t current_bytes;
    size_t peak_bytes;
    size_t allocation_count;
} idcu_MemorySnapshot;

static idcu_MemorySnapshot snapshots[IDCU_MEM_MAX_SNAPSHOTS];
static size_t snapshot_count = 0;
static size_t snapshot_index = 0;

static const uint32_t default_sizes[IDCU_MEM_POOL_MAX_SIZE_CLASSES] = {
    16, 32, 64, 128, 256, 512, 1024, 2048
};

static int select_size_class(size_t size) {
    if (size == 0) return -1;
    if (size <= 16) return 0;
    if (size <= 32) return 1;
    if (size <= 64) return 2;
    if (size <= 128) return 3;
    if (size <= 256) return 4;
    if (size <= 512) return 5;
    if (size <= 1024) return 6;
    if (size <= 2048) return 7;
    return -1;
}

static int capture_stack_trace(void** frames, int max_frames) {
#ifdef _WIN32
    (void)frames;
    (void)max_frames;
    return 0;
#else
    return backtrace(frames, max_frames);
#endif
}

static uint64_t get_timestamp(void) {
    return (uint64_t)time(NULL);
}

int idcu_memory_debugger_init(void) {
    if (initialized) {
        return 0;
    }
    
    idcu_vector_init(&allocations, sizeof(idcu_MemoryAllocation), 64);
    idcu_mutex_init(&debugger_mutex);
    initialized = 1;
    total_allocations = 0;
    total_frees = 0;
    current_bytes = 0;
    peak_bytes = 0;
    snapshot_count = 0;
    snapshot_index = 0;
    memset(size_class_counts, 0, sizeof(size_class_counts));
    memset(size_class_bytes, 0, sizeof(size_class_bytes));
    memset(snapshots, 0, sizeof(snapshots));
    return 0;
}

void idcu_memory_debugger_destroy(void) {
    if (!initialized) {
        return;
    }
    
    idcu_mutex_lock(&debugger_mutex);
    
    idcu_memory_debugger_print_leaks();
    
    for (size_t i = 0; i < allocations.size; i++) {
        idcu_MemoryAllocation* alloc = (idcu_MemoryAllocation*)idcu_vector_get(&allocations, i);
        if (alloc->in_use) {
            free(alloc->address);
        }
    }
    
    idcu_vector_destroy(&allocations);
    idcu_mutex_unlock(&debugger_mutex);
    idcu_mutex_destroy(&debugger_mutex);
    initialized = 0;
}

void* idcu_debug_malloc(size_t size, const char* file, int line) {
    if (!initialized) {
        idcu_memory_debugger_init();
    }
    
    idcu_mutex_lock(&debugger_mutex);
    
    void* ptr = malloc(size);
    if (!ptr) {
        idcu_mutex_unlock(&debugger_mutex);
        return NULL;
    }
    
    idcu_MemoryAllocation alloc;
    alloc.address = ptr;
    alloc.size = size;
    alloc.file = file;
    alloc.line = line;
    alloc.timestamp = get_timestamp();
    alloc.in_use = 1;
    alloc.stack_depth = capture_stack_trace(alloc.stack_frames, IDCU_MEM_MAX_STACK_FRAMES);
    
    idcu_vector_push(&allocations, &alloc);
    total_allocations++;
    current_bytes += size;
    if (current_bytes > peak_bytes) {
        peak_bytes = current_bytes;
    }
    
    int sc_idx = select_size_class(size);
    if (sc_idx >= 0) {
        size_class_counts[sc_idx]++;
        size_class_bytes[sc_idx] += size;
    }
    
    idcu_mutex_unlock(&debugger_mutex);
    
    return ptr;
}

void* idcu_debug_calloc(size_t num, size_t size, const char* file, int line) {
    if (!initialized) {
        idcu_memory_debugger_init();
    }
    
    idcu_mutex_lock(&debugger_mutex);
    
    size_t total_size = num * size;
    void* ptr = calloc(num, size);
    if (!ptr) {
        idcu_mutex_unlock(&debugger_mutex);
        return NULL;
    }
    
    idcu_MemoryAllocation alloc;
    alloc.address = ptr;
    alloc.size = total_size;
    alloc.file = file;
    alloc.line = line;
    alloc.timestamp = get_timestamp();
    alloc.in_use = 1;
    alloc.stack_depth = capture_stack_trace(alloc.stack_frames, IDCU_MEM_MAX_STACK_FRAMES);
    
    idcu_vector_push(&allocations, &alloc);
    total_allocations++;
    current_bytes += total_size;
    if (current_bytes > peak_bytes) {
        peak_bytes = current_bytes;
    }
    
    int sc_idx = select_size_class(total_size);
    if (sc_idx >= 0) {
        size_class_counts[sc_idx]++;
        size_class_bytes[sc_idx] += total_size;
    }
    
    idcu_mutex_unlock(&debugger_mutex);
    
    return ptr;
}

void* idcu_debug_realloc(void* ptr, size_t size, const char* file, int line) {
    if (!initialized) {
        idcu_memory_debugger_init();
    }
    
    idcu_mutex_lock(&debugger_mutex);
    
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        idcu_mutex_unlock(&debugger_mutex);
        return NULL;
    }
    
    if (ptr) {
        for (size_t i = 0; i < allocations.size; i++) {
            idcu_MemoryAllocation* alloc = (idcu_MemoryAllocation*)idcu_vector_get(&allocations, i);
            if (alloc->address == ptr && alloc->in_use) {
                int old_sc_idx = select_size_class(alloc->size);
                if (old_sc_idx >= 0) {
                    size_class_counts[old_sc_idx]--;
                    size_class_bytes[old_sc_idx] -= alloc->size;
                }
                
                current_bytes -= alloc->size;
                alloc->address = new_ptr;
                alloc->size = size;
                alloc->file = file;
                alloc->line = line;
                alloc->timestamp = get_timestamp();
                alloc->stack_depth = capture_stack_trace(alloc->stack_frames, IDCU_MEM_MAX_STACK_FRAMES);
                current_bytes += size;
                
                if (current_bytes > peak_bytes) {
                    peak_bytes = current_bytes;
                }
                
                int new_sc_idx = select_size_class(size);
                if (new_sc_idx >= 0) {
                    size_class_counts[new_sc_idx]++;
                    size_class_bytes[new_sc_idx] += size;
                }
                
                idcu_mutex_unlock(&debugger_mutex);
                return new_ptr;
            }
        }
    }
    
    idcu_MemoryAllocation alloc;
    alloc.address = new_ptr;
    alloc.size = size;
    alloc.file = file;
    alloc.line = line;
    alloc.timestamp = get_timestamp();
    alloc.in_use = 1;
    alloc.stack_depth = capture_stack_trace(alloc.stack_frames, IDCU_MEM_MAX_STACK_FRAMES);
    
    idcu_vector_push(&allocations, &alloc);
    total_allocations++;
    current_bytes += size;
    if (current_bytes > peak_bytes) {
        peak_bytes = current_bytes;
    }
    
    int sc_idx = select_size_class(size);
    if (sc_idx >= 0) {
        size_class_counts[sc_idx]++;
        size_class_bytes[sc_idx] += size;
    }
    
    idcu_mutex_unlock(&debugger_mutex);
    
    return new_ptr;
}

void idcu_debug_free(void* ptr, const char* file, int line) {
    (void)file;
    (void)line;
    
    if (!ptr || !initialized) {
        free(ptr);
        return;
    }
    
    idcu_mutex_lock(&debugger_mutex);
    
    for (size_t i = 0; i < allocations.size; i++) {
        idcu_MemoryAllocation* alloc = (idcu_MemoryAllocation*)idcu_vector_get(&allocations, i);
        if (alloc->address == ptr && alloc->in_use) {
            int sc_idx = select_size_class(alloc->size);
            if (sc_idx >= 0) {
                size_class_counts[sc_idx]--;
                size_class_bytes[sc_idx] -= alloc->size;
            }
            
            alloc->in_use = 0;
            total_frees++;
            current_bytes -= alloc->size;
            free(ptr);
            idcu_mutex_unlock(&debugger_mutex);
            return;
        }
    }
    
    free(ptr);
    idcu_mutex_unlock(&debugger_mutex);
}

size_t idcu_memory_debugger_get_allocated_count(void) {
    if (!initialized) {
        return 0;
    }
    
    idcu_mutex_lock(&debugger_mutex);
    
    size_t count = 0;
    for (size_t i = 0; i < allocations.size; i++) {
        idcu_MemoryAllocation* alloc = (idcu_MemoryAllocation*)idcu_vector_get(&allocations, i);
        if (alloc->in_use) {
            count++;
        }
    }
    
    idcu_mutex_unlock(&debugger_mutex);
    return count;
}

size_t idcu_memory_debugger_get_total_allocated(void) {
    return total_allocations;
}

size_t idcu_memory_debugger_get_current_bytes(void) {
    if (!initialized) {
        return 0;
    }
    return current_bytes;
}

size_t idcu_memory_debugger_get_peak_bytes(void) {
    if (!initialized) {
        return 0;
    }
    return peak_bytes;
}

void idcu_memory_debugger_get_stats(idcu_MemoryDebuggerStats* stats) {
    if (!initialized || !stats) {
        return;
    }
    
    idcu_mutex_lock(&debugger_mutex);
    
    memset(stats, 0, sizeof(idcu_MemoryDebuggerStats));
    stats->total_allocations = total_allocations;
    stats->total_frees = total_frees;
    stats->current_allocations = idcu_memory_debugger_get_allocated_count();
    stats->current_bytes = current_bytes;
    stats->peak_bytes = peak_bytes;
    
    for (int i = 0; i < IDCU_MEM_POOL_MAX_SIZE_CLASSES; i++) {
        stats->size_class_stats[i].size_class = i;
        stats->size_class_stats[i].allocation_count = size_class_counts[i];
        stats->size_class_stats[i].total_bytes = size_class_bytes[i];
    }
    
    idcu_mutex_unlock(&debugger_mutex);
}

void idcu_memory_debugger_print_leaks(void) {
    if (!initialized) {
        return;
    }
    
    idcu_mutex_lock(&debugger_mutex);
    
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
            
            if (alloc->stack_depth > 0) {
                printf("    Stack trace:\n");
                for (int j = 0; j < alloc->stack_depth; j++) {
                    printf("      #%d: %p\n", j, alloc->stack_frames[j]);
                }
            }
        }
    }
    
    if (leak_count == 0) {
        printf("  No memory leaks detected!\n");
    } else {
        printf("  Total: %zu leaks, %zu bytes\n", leak_count, leak_size);
    }
    
    printf("==========================\n");
    
    idcu_mutex_unlock(&debugger_mutex);
}

void idcu_memory_debugger_print_stats(void) {
    if (!initialized) {
        return;
    }
    
    idcu_mutex_lock(&debugger_mutex);
    
    printf("=== Memory Debugger Statistics ===\n");
    printf("  Total allocations: %zu\n", total_allocations);
    printf("  Total frees: %zu\n", total_frees);
    printf("  Current allocations: %zu\n", idcu_memory_debugger_get_allocated_count());
    printf("  Current bytes: %zu\n", current_bytes);
    printf("  Peak bytes: %zu\n", peak_bytes);
    printf("\n");
    printf("  Size class statistics:\n");
    
    for (int i = 0; i < IDCU_MEM_POOL_MAX_SIZE_CLASSES; i++) {
        printf("    Size %u: %zu allocations, %zu bytes\n",
               default_sizes[i], size_class_counts[i], size_class_bytes[i]);
    }
    
    printf("==================================\n");
    
    idcu_mutex_unlock(&debugger_mutex);
}

void idcu_memory_debugger_reset(void) {
    if (!initialized) {
        return;
    }
    
    idcu_mutex_lock(&debugger_mutex);
    
    for (size_t i = 0; i < allocations.size; i++) {
        idcu_MemoryAllocation* alloc = (idcu_MemoryAllocation*)idcu_vector_get(&allocations, i);
        if (alloc->in_use) {
            free(alloc->address);
        }
    }
    
    idcu_vector_clear(&allocations);
    total_allocations = 0;
    total_frees = 0;
    current_bytes = 0;
    peak_bytes = 0;
    snapshot_count = 0;
    snapshot_index = 0;
    memset(size_class_counts, 0, sizeof(size_class_counts));
    memset(size_class_bytes, 0, sizeof(size_class_bytes));
    memset(snapshots, 0, sizeof(snapshots));
    
    idcu_mutex_unlock(&debugger_mutex);
}

void idcu_memory_debugger_take_snapshot(void) {
    if (!initialized) {
        return;
    }
    
    idcu_mutex_lock(&debugger_mutex);
    
    idcu_MemorySnapshot* snapshot = &snapshots[snapshot_index];
    snapshot->timestamp = get_timestamp();
    snapshot->current_bytes = current_bytes;
    snapshot->peak_bytes = peak_bytes;
    snapshot->allocation_count = idcu_memory_debugger_get_allocated_count();
    
    snapshot_index = (snapshot_index + 1) % IDCU_MEM_MAX_SNAPSHOTS;
    if (snapshot_count < IDCU_MEM_MAX_SNAPSHOTS) {
        snapshot_count++;
    }
    
    idcu_mutex_unlock(&debugger_mutex);
}

void idcu_memory_debugger_print_trend(void) {
    if (!initialized || snapshot_count == 0) {
        printf("No memory snapshots available.\n");
        return;
    }
    
    idcu_mutex_lock(&debugger_mutex);
    
    printf("=== Memory Usage Trend ===\n");
    printf("  Snapshots: %zu\n", snapshot_count);
    printf("\n");
    
    for (size_t i = 0; i < snapshot_count; i++) {
        size_t actual_index;
        if (snapshot_count < IDCU_MEM_MAX_SNAPSHOTS) {
            actual_index = i;
        } else {
            actual_index = (snapshot_index + i) % IDCU_MEM_MAX_SNAPSHOTS;
        }
        
        idcu_MemorySnapshot* snapshot = &snapshots[actual_index];
        printf("  #%zu: time=%llu, current=%zu bytes, peak=%zu bytes, allocations=%zu\n",
               i + 1, (unsigned long long)snapshot->timestamp,
               snapshot->current_bytes, snapshot->peak_bytes,
               snapshot->allocation_count);
    }
    
    printf("==========================\n");
    
    idcu_mutex_unlock(&debugger_mutex);
}
