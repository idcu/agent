#ifndef IDCU_UTILS_MEMORY_POOL_H
#define IDCU_UTILS_MEMORY_POOL_H

#include <stdint.h>
#include "common/lock.h"

#define MEM_POOL_MAX_BLOCKS 1024
#define MEM_POOL_MAX_SIZE_CLASSES 8

typedef struct {
    void *data;
    uint8_t in_use;
} PoolBlock;

typedef struct {
    uint32_t block_size;
    uint32_t block_count;
    PoolBlock *blocks;
    uint32_t free_count;
    uint32_t *free_list;
    uint32_t free_head;
} SizeClass;

typedef struct {
    SizeClass size_classes[MEM_POOL_MAX_SIZE_CLASSES];
    uint32_t num_size_classes;
    Mutex lock;
    uint64_t total_allocated;
    uint64_t total_freed;
    uint64_t peak_usage;
} MemoryPool;

int mem_pool_init(MemoryPool *pool);
void mem_pool_destroy(MemoryPool *pool);
void* mem_pool_alloc(MemoryPool *pool, uint32_t size);
void mem_pool_free(MemoryPool *pool, void *ptr);
uint32_t mem_pool_get_free_count(MemoryPool *pool, uint32_t size);
uint64_t mem_pool_get_total_allocated(MemoryPool *pool);
uint64_t mem_pool_get_peak_usage(MemoryPool *pool);

#endif // IDCU_UTILS_MEMORY_POOL_H
