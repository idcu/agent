#include "utils/memory_pool.h"
#include "common/error_code.h"
#include <stdlib.h>
#include <string.h>

static const uint32_t size_class_sizes[] = {
    16, 32, 64, 128, 256, 512, 1024, 2048
};

static int get_size_class_index(uint32_t size)
{
    for (uint32_t i = 0; i < IDCU_MEM_POOL_MAX_SIZE_CLASSES; i++) {
        if (size <= size_class_sizes[i]) {
            return i;
        }
    }
    return -1;
}

int idcu_mem_pool_init(idcu_MemoryPool *pool)
{
    if (!pool) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(pool, 0, sizeof(idcu_MemoryPool));
    idcu_mutex_init(&pool->lock);
    pool->num_size_classes = IDCU_MEM_POOL_MAX_SIZE_CLASSES;

    for (uint32_t i = 0; i < pool->num_size_classes; i++) {
        idcu_SizeClass *sc = &pool->size_classes[i];
        sc->block_size = size_class_sizes[i];
        sc->block_count = IDCU_MEM_POOL_MAX_BLOCKS / pool->num_size_classes;
        
        sc->blocks = (idcu_PoolBlock*)malloc(sc->block_count * sizeof(idcu_PoolBlock));
        if (!sc->blocks) {
            idcu_mem_pool_destroy(pool);
            return IDCU_ERR_NO_MEMORY;
        }
        
        sc->free_list = (uint32_t*)malloc(sc->block_count * sizeof(uint32_t));
        if (!sc->free_list) {
            idcu_mem_pool_destroy(pool);
            return IDCU_ERR_NO_MEMORY;
        }
        
        for (uint32_t j = 0; j < sc->block_count; j++) {
            sc->blocks[j].data = malloc(sc->block_size);
            if (!sc->blocks[j].data) {
                idcu_mem_pool_destroy(pool);
                return IDCU_ERR_NO_MEMORY;
            }
            sc->blocks[j].in_use = 0;
            sc->free_list[j] = j;
        }
        
        sc->free_count = sc->block_count;
        sc->free_head = 0;
    }

    return IDCU_ERR_SUCCESS;
}

void idcu_mem_pool_destroy(idcu_MemoryPool *pool)
{
    if (!pool) {
        return;
    }

    for (uint32_t i = 0; i < pool->num_size_classes; i++) {
        idcu_SizeClass *sc = &pool->size_classes[i];
        
        if (sc->blocks) {
            for (uint32_t j = 0; j < sc->block_count; j++) {
                if (sc->blocks[j].data) {
                    free(sc->blocks[j].data);
                }
            }
            free(sc->blocks);
            sc->blocks = NULL;
        }
        
        if (sc->free_list) {
            free(sc->free_list);
            sc->free_list = NULL;
        }
    }

    idcu_mutex_destroy(&pool->lock);
}

void* idcu_mem_pool_alloc(idcu_MemoryPool *pool, uint32_t size)
{
    if (!pool || size == 0) {
        return NULL;
    }

    int idx = get_size_class_index(size);
    if (idx < 0) {
        return malloc(size);
    }

    int ret = idcu_mutex_lock(&pool->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return NULL;
    }

    idcu_SizeClass *sc = &pool->size_classes[idx];
    if (sc->free_count == 0) {
        idcu_mutex_unlock(&pool->lock);
        return NULL;
    }

    uint32_t block_idx = sc->free_list[sc->free_head];
    sc->free_head++;
    sc->free_count--;
    
    idcu_PoolBlock *block = &sc->blocks[block_idx];
    block->in_use = 1;
    
    pool->total_allocated += sc->block_size;
    uint64_t current_usage = pool->total_allocated - pool->total_freed;
    if (current_usage > pool->peak_usage) {
        pool->peak_usage = current_usage;
    }

    idcu_mutex_unlock(&pool->lock);
    return block->data;
}

void idcu_mem_pool_free(idcu_MemoryPool *pool, void *ptr)
{
    if (!pool || !ptr) {
        return;
    }

    int found = 0;
    int class_idx = -1;
    uint32_t block_idx = 0;

    int ret = idcu_mutex_lock(&pool->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        free(ptr);
        return;
    }

    for (uint32_t i = 0; i < pool->num_size_classes; i++) {
        idcu_SizeClass *sc = &pool->size_classes[i];
        for (uint32_t j = 0; j < sc->block_count; j++) {
            if (sc->blocks[j].data == ptr) {
                class_idx = i;
                block_idx = j;
                found = 1;
                break;
            }
        }
        if (found) {
            break;
        }
    }

    if (!found) {
        idcu_mutex_unlock(&pool->lock);
        free(ptr);
        return;
    }

    idcu_SizeClass *sc = &pool->size_classes[class_idx];
    idcu_PoolBlock *block = &sc->blocks[block_idx];
    
    block->in_use = 0;
    sc->free_head--;
    sc->free_list[sc->free_head] = block_idx;
    sc->free_count++;
    
    pool->total_freed += sc->block_size;

    idcu_mutex_unlock(&pool->lock);
}

uint32_t idcu_mem_pool_get_free_count(idcu_MemoryPool *pool, uint32_t size)
{
    if (!pool) {
        return 0;
    }

    int idx = get_size_class_index(size);
    if (idx < 0) {
        return 0;
    }

    int ret = idcu_mutex_lock(&pool->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }

    uint32_t count = pool->size_classes[idx].free_count;
    idcu_mutex_unlock(&pool->lock);
    return count;
}

uint64_t idcu_mem_pool_get_total_allocated(idcu_MemoryPool *pool)
{
    if (!pool) {
        return 0;
    }

    int ret = idcu_mutex_lock(&pool->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }

    uint64_t total = pool->total_allocated;
    idcu_mutex_unlock(&pool->lock);
    return total;
}

uint64_t idcu_mem_pool_get_peak_usage(idcu_MemoryPool *pool)
{
    if (!pool) {
        return 0;
    }

    int ret = idcu_mutex_lock(&pool->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }

    uint64_t peak = pool->peak_usage;
    idcu_mutex_unlock(&pool->lock);
    return peak;
}
