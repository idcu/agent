#ifndef IDCU_UTILS_MEMORY_POOL_H
#define IDCU_UTILS_MEMORY_POOL_H

#include <stdint.h>
#include "common/lock.h"

#define IDCU_MEM_POOL_MAX_BLOCKS 1024
#define IDCU_MEM_POOL_MAX_SIZE_CLASSES 8

typedef struct {
    void *data;
    uint8_t in_use;
} idcu_PoolBlock;

typedef struct {
    uint32_t block_size;
    uint32_t block_count;
    idcu_PoolBlock *blocks;
    uint32_t free_count;
    uint32_t *free_list;
    uint32_t free_head;
} idcu_SizeClass;

typedef struct {
    idcu_SizeClass size_classes[IDCU_MEM_POOL_MAX_SIZE_CLASSES];
    uint32_t num_size_classes;
    idcu_Mutex lock;
    uint64_t total_allocated;
    uint64_t total_freed;
    uint64_t peak_usage;
} idcu_MemoryPool;

int idcu_mem_pool_init(idcu_MemoryPool *pool);
void idcu_mem_pool_destroy(idcu_MemoryPool *pool);
void* idcu_mem_pool_alloc(idcu_MemoryPool *pool, uint32_t size);
void idcu_mem_pool_free(idcu_MemoryPool *pool, void *ptr);
uint32_t idcu_mem_pool_get_free_count(idcu_MemoryPool *pool, uint32_t size);
uint64_t idcu_mem_pool_get_total_allocated(idcu_MemoryPool *pool);
uint64_t idcu_mem_pool_get_peak_usage(idcu_MemoryPool *pool);

#endif // IDCU_UTILS_MEMORY_POOL_H
