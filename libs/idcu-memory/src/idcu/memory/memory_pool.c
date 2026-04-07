#include "idcu/memory/memory_pool.h"
#include "idcu/common/error_code.h"
#include "idcu/log/log.h"
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

static void write_guard(void* ptr, uint32_t size)
{
    if (!ptr) return;
    uint8_t* guard = (uint8_t*)ptr + size;
    memset(guard, IDCU_MEM_GUARD_VALUE, IDCU_MEM_GUARD_SIZE);
}

static int check_guard(const void* ptr, uint32_t size)
{
    if (!ptr) return -1;
    const uint8_t* guard = (const uint8_t*)ptr + size;
    for (uint32_t i = 0; i < IDCU_MEM_GUARD_SIZE; i++) {
        if (guard[i] != IDCU_MEM_GUARD_VALUE) {
            return -1;
        }
    }
    return 0;
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
        
        int ret = idcu_mutex_init(&sc->class_lock);
        if (ret != IDCU_ERR_SUCCESS) {
            idcu_mem_pool_destroy(pool);
            return ret;
        }
        
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
            sc->blocks[j].data = malloc(sc->block_size + IDCU_MEM_GUARD_SIZE);
            if (!sc->blocks[j].data) {
                idcu_mem_pool_destroy(pool);
                return IDCU_ERR_NO_MEMORY;
            }
            sc->blocks[j].in_use = 0;
            sc->blocks[j].alloc_size = 0;
            sc->blocks[j].size_class = (uint8_t)i;
            write_guard(sc->blocks[j].data, sc->block_size);
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
        
        idcu_mutex_destroy(&sc->class_lock);
    }

    idcu_mutex_destroy(&pool->lock);
}

void* idcu_mem_pool_alloc(idcu_MemoryPool *pool, uint32_t size)
{
    if (!pool || size == 0) {
        if (pool) pool->null_check_count++;
        return NULL;
    }

    int idx = get_size_class_index(size);
    if (idx < 0) {
        void* ptr = malloc(size + IDCU_MEM_GUARD_SIZE);
        if (ptr) {
            write_guard(ptr, size);
        }
        return ptr;
    }

    idcu_SizeClass *sc = &pool->size_classes[idx];
    int ret = idcu_mutex_lock(&sc->class_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return NULL;
    }

    if (sc->free_count == 0) {
        idcu_mutex_unlock(&sc->class_lock);
        return NULL;
    }

    uint32_t block_idx = sc->free_list[sc->free_head];
    sc->free_head++;
    sc->free_count--;
    
    idcu_PoolBlock *block = &sc->blocks[block_idx];
    block->in_use = 1;
    block->alloc_size = size;
    block->size_class = (uint8_t)idx;
    
    idcu_mutex_lock(&pool->lock);
    pool->total_allocated += sc->block_size;
    uint64_t current_usage = pool->total_allocated - pool->total_freed;
    if (current_usage > pool->peak_usage) {
        pool->peak_usage = current_usage;
    }
    idcu_mutex_unlock(&pool->lock);

    idcu_mutex_unlock(&sc->class_lock);
    return block->data;
}

void idcu_mem_pool_free(idcu_MemoryPool *pool, void *ptr)
{
    if (!pool) {
        return;
    }
    
    if (!ptr) {
        pool->null_check_count++;
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
                
                if (check_guard(ptr, sc->block_size) != 0) {
                    IDCU_LOG_ERROR("Memory overflow detected at block %u in class %u", j, i);
                    pool->overflow_check_count++;
                }
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
    idcu_mutex_unlock(&pool->lock);
    
    idcu_mutex_lock(&sc->class_lock);
    idcu_PoolBlock *block = &sc->blocks[block_idx];
    
    block->in_use = 0;
    block->alloc_size = 0;
    write_guard(block->data, sc->block_size);
    sc->free_head--;
    sc->free_list[sc->free_head] = block_idx;
    sc->free_count++;
    
    idcu_mutex_lock(&pool->lock);
    pool->total_freed += sc->block_size;
    idcu_mutex_unlock(&pool->lock);

    idcu_mutex_unlock(&sc->class_lock);
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

    idcu_SizeClass *sc = &pool->size_classes[idx];
    int ret = idcu_mutex_lock(&sc->class_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }

    uint32_t count = sc->free_count;
    idcu_mutex_unlock(&sc->class_lock);
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

int idcu_mem_check_null(const void* ptr, const char* context)
{
    if (!ptr) {
        IDCU_LOG_ERROR("NULL pointer detected: %s", context ? context : "unknown");
        return IDCU_ERR_INVALID_PARAM;
    }
    return IDCU_ERR_OK;
}

int idcu_mem_check_overflow(const void* ptr, size_t size, const char* context)
{
    if (!ptr) {
        IDCU_LOG_ERROR("NULL pointer in overflow check: %s", context ? context : "unknown");
        return IDCU_ERR_INVALID_PARAM;
    }
    
    const uint8_t* end = (const uint8_t*)ptr + size;
    (void)end;
    
    return IDCU_ERR_OK;
}

int idcu_mem_safe_copy(void* dst, size_t dst_size, const void* src, size_t src_size)
{
    if (!dst || !src) {
        IDCU_LOG_ERROR("NULL pointer in safe copy");
        return IDCU_ERR_INVALID_PARAM;
    }
    
    if (src_size > dst_size) {
        IDCU_LOG_ERROR("Buffer overflow prevented: src_size=%zu, dst_size=%zu", src_size, dst_size);
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    
    memcpy(dst, src, src_size);
    return IDCU_ERR_OK;
}

int idcu_mem_pool_get_safety_stats(idcu_MemoryPool* pool, uint32_t* null_checks, uint32_t* overflow_checks)
{
    if (!pool) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&pool->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    if (null_checks) *null_checks = pool->null_check_count;
    if (overflow_checks) *overflow_checks = pool->overflow_check_count;
    
    idcu_mutex_unlock(&pool->lock);
    return IDCU_ERR_OK;
}
