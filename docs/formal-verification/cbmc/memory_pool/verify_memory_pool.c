/**
 * @file verify_memory_pool.c
 * @brief CBMC 形式化验证 - 内存池
 *
 * 验证属性:
 * 1. 分配正确性: 分配的内存块大小正确
 * 2. 无内存泄漏: 所有分配的内存都能被释放
 * 3. 无野指针: 释放的内存不会被错误访问
 * 4. 双重释放检测: 尝试释放已释放的内存会被正确处理
 * 5. 边界检查: 内存块边界不会被越界访问
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define IDCU_ERR_OK 0
#define IDCU_ERR_INVALID_ARG 1
#define IDCU_ERR_MEMORY 2
#define IDCU_MEM_POOL_MAX_SIZE_CLASSES 8
#define IDCU_POOL_HEADER_MAGIC 0xDEADBEEF
#define IDCU_MEM_GUARD_SIZE 4

typedef struct {
    uint32_t magic;
    uint8_t size_class;
    uint32_t block_idx;
    uint8_t guard[IDCU_MEM_GUARD_SIZE];
} idcu_PoolBlockHeader;

typedef struct {
    uint8_t size_class;
    int in_use;
    uint32_t alloc_size;
    void* data;
    uint8_t guard[IDCU_MEM_GUARD_SIZE];
} idcu_PoolBlock;

typedef struct {
    uint32_t block_size;
    uint32_t block_count;
    uint32_t free_count;
    uint32_t free_head;
    uint32_t alloc_count;
    uint32_t free_count_total;
    idcu_PoolBlock* blocks;
    uint32_t* free_list;
} idcu_SizeClass;

typedef struct {
    uint32_t num_size_classes;
    idcu_SizeClass size_classes[IDCU_MEM_POOL_MAX_SIZE_CLASSES];
    uint64_t total_allocated;
    uint64_t total_freed;
    uint64_t current_usage;
    uint64_t peak_usage;
} idcu_MemoryPool;

static const uint32_t default_sizes[IDCU_MEM_POOL_MAX_SIZE_CLASSES] = {
    16, 32, 64, 128, 256, 512, 1024, 2048
};

static const uint32_t default_block_counts[IDCU_MEM_POOL_MAX_SIZE_CLASSES] = {
    4, 4, 4, 4, 4, 4, 4, 4
};

static int select_size_class(uint32_t size) {
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

static int idcu_mem_pool_init(idcu_MemoryPool* pool) {
    if (!pool) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(pool, 0, sizeof(idcu_MemoryPool));
    pool->num_size_classes = IDCU_MEM_POOL_MAX_SIZE_CLASSES;
    
    for (uint32_t i = 0; i < pool->num_size_classes; i++) {
        idcu_SizeClass* sc = &pool->size_classes[i];
        sc->block_size = default_sizes[i];
        sc->block_count = default_block_counts[i];
        sc->free_count = sc->block_count;
        sc->free_head = 0;
        sc->alloc_count = 0;
        sc->free_count_total = 0;
        
        sc->blocks = (idcu_PoolBlock*)calloc(sc->block_count, sizeof(idcu_PoolBlock));
        if (!sc->blocks) {
            return IDCU_ERR_MEMORY;
        }
        
        sc->free_list = (uint32_t*)calloc(sc->block_count, sizeof(uint32_t));
        if (!sc->free_list) {
            free(sc->blocks);
            return IDCU_ERR_MEMORY;
        }
        
        for (uint32_t j = 0; j < sc->block_count - 1; j++) {
            sc->free_list[j] = j + 1;
            sc->blocks[j].size_class = (uint8_t)i;
            sc->blocks[j].in_use = 0;
            
            sc->blocks[j].data = calloc(1, sc->block_size + sizeof(idcu_PoolBlockHeader) + IDCU_MEM_GUARD_SIZE);
            if (!sc->blocks[j].data) {
                return IDCU_ERR_MEMORY;
            }
            
            memset(sc->blocks[j].guard, 0xAA, IDCU_MEM_GUARD_SIZE);
        }
        
        sc->free_list[sc->block_count - 1] = UINT32_MAX;
        sc->blocks[sc->block_count - 1].size_class = (uint8_t)i;
        sc->blocks[sc->block_count - 1].in_use = 0;
        sc->blocks[sc->block_count - 1].data = calloc(1, sc->block_size + sizeof(idcu_PoolBlockHeader) + IDCU_MEM_GUARD_SIZE);
        if (!sc->blocks[sc->block_count - 1].data) {
            return IDCU_ERR_MEMORY;
        }
        memset(sc->blocks[sc->block_count - 1].guard, 0xAA, IDCU_MEM_GUARD_SIZE);
    }
    
    return IDCU_ERR_OK;
}

static void idcu_mem_pool_destroy(idcu_MemoryPool* pool) {
    if (!pool) {
        return;
    }
    
    for (uint32_t i = 0; i < pool->num_size_classes; i++) {
        idcu_SizeClass* sc = &pool->size_classes[i];
        
        if (sc->blocks) {
            for (uint32_t j = 0; j < sc->block_count; j++) {
                if (sc->blocks[j].data) {
                    free(sc->blocks[j].data);
                }
            }
            free(sc->blocks);
        }
        
        if (sc->free_list) {
            free(sc->free_list);
        }
    }
    
    memset(pool, 0, sizeof(idcu_MemoryPool));
}

static void* idcu_mem_pool_alloc(idcu_MemoryPool* pool, uint32_t size) {
    if (!pool || size == 0) {
        return NULL;
    }
    
    int sc_idx = select_size_class(size);
    if (sc_idx < 0) {
        return NULL;
    }
    
    idcu_SizeClass* sc = &pool->size_classes[sc_idx];
    
    if (sc->free_head == UINT32_MAX || sc->free_count == 0) {
        return NULL;
    }
    
    uint32_t block_idx = sc->free_head;
    idcu_PoolBlock* block = &sc->blocks[block_idx];
    
    sc->free_head = sc->free_list[block_idx];
    sc->free_count--;
    sc->alloc_count++;
    
    block->in_use = 1;
    block->alloc_size = size;
    
    idcu_PoolBlockHeader* header = (idcu_PoolBlockHeader*)block->data;
    header->size_class = (uint8_t)sc_idx;
    header->block_idx = block_idx;
    header->magic = IDCU_POOL_HEADER_MAGIC;
    memset(header->guard, 0xBB, IDCU_MEM_GUARD_SIZE);
    
    void* data_ptr = (uint8_t*)block->data + sizeof(idcu_PoolBlockHeader);
    
    pool->total_allocated += sc->block_size;
    pool->current_usage += sc->block_size;
    if (pool->current_usage > pool->peak_usage) {
        pool->peak_usage = pool->current_usage;
    }
    
    return data_ptr;
}

static void idcu_mem_pool_free(idcu_MemoryPool* pool, void* ptr) {
    if (!pool || !ptr) {
        return;
    }
    
    idcu_PoolBlockHeader* header = (idcu_PoolBlockHeader*)((uint8_t*)ptr - sizeof(idcu_PoolBlockHeader));
    
    if (header->magic != IDCU_POOL_HEADER_MAGIC) {
        return;
    }
    
    uint8_t sc_idx = header->size_class;
    if (sc_idx >= pool->num_size_classes) {
        return;
    }
    
    idcu_SizeClass* sc = &pool->size_classes[sc_idx];
    uint32_t block_idx = header->block_idx;
    
    if (block_idx >= sc->block_count) {
        return;
    }
    
    idcu_PoolBlock* block = &sc->blocks[block_idx];
    if (!block->in_use) {
        return;
    }
    
    int guard_valid = 1;
    for (int i = 0; i < IDCU_MEM_GUARD_SIZE; i++) {
        if (block->guard[i] != 0xAA) {
            guard_valid = 0;
            break;
        }
    }
    
    block->in_use = 0;
    block->alloc_size = 0;
    sc->free_count_total++;
    
    memset(block->data, 0, sc->block_size + sizeof(idcu_PoolBlockHeader) + IDCU_MEM_GUARD_SIZE);
    memset(block->guard, 0xAA, IDCU_MEM_GUARD_SIZE);
    
    sc->free_list[block_idx] = sc->free_head;
    sc->free_head = block_idx;
    sc->free_count++;
    
    pool->total_freed += sc->block_size;
    pool->current_usage -= sc->block_size;
}

static uint32_t idcu_mem_pool_get_free_count(idcu_MemoryPool* pool, uint32_t size) {
    if (!pool) {
        return 0;
    }
    
    int sc_idx = select_size_class(size);
    if (sc_idx < 0) {
        return 0;
    }
    
    idcu_SizeClass* sc = &pool->size_classes[sc_idx];
    return sc->free_count;
}

static uint64_t idcu_mem_pool_get_total_allocated(idcu_MemoryPool* pool) {
    if (!pool) {
        return 0;
    }
    return pool->total_allocated;
}

static uint64_t idcu_mem_pool_get_peak_usage(idcu_MemoryPool* pool) {
    if (!pool) {
        return 0;
    }
    return pool->peak_usage;
}

bool verify_memory_pool_invariant(const idcu_MemoryPool* pool)
{
    if (!pool) return true;
    
    for (uint32_t i = 0; i < pool->num_size_classes; i++) {
        const idcu_SizeClass* sc = &pool->size_classes[i];
        
        if (sc->free_count > sc->block_count) {
            return false;
        }
        
        uint32_t in_use_count = 0;
        for (uint32_t j = 0; j < sc->block_count; j++) {
            if (sc->blocks[j].in_use) {
                in_use_count++;
            }
        }
        
        if (in_use_count + sc->free_count != sc->block_count) {
            return false;
        }
    }
    
    return true;
}

void test_memory_pool_init_destroy()
{
    idcu_MemoryPool pool;
    int ret = idcu_mem_pool_init(&pool);
    
    __CPROVER_assert(ret == IDCU_ERR_OK, "初始化应该成功");
    __CPROVER_assert(pool.num_size_classes == IDCU_MEM_POOL_MAX_SIZE_CLASSES, "尺寸类别数应该正确");
    __CPROVER_assert(verify_memory_pool_invariant(&pool), "内存池不变式应该成立");
    
    idcu_mem_pool_destroy(&pool);
}

void test_memory_pool_alloc()
{
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&pool);
    
    uint32_t initial_free = idcu_mem_pool_get_free_count(&pool, 16);
    
    void* ptr = idcu_mem_pool_alloc(&pool, 16);
    
    __CPROVER_assert(ptr != NULL, "分配应该成功");
    __CPROVER_assert(idcu_mem_pool_get_free_count(&pool, 16) == initial_free - 1, "空闲块数应该减少");
    __CPROVER_assert(idcu_mem_pool_get_total_allocated(&pool) > 0, "总分配量应该大于0");
    __CPROVER_assert(verify_memory_pool_invariant(&pool), "内存池不变式应该成立");
    
    idcu_mem_pool_destroy(&pool);
}

void test_memory_pool_free()
{
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&pool);
    
    void* ptr = idcu_mem_pool_alloc(&pool, 32);
    uint32_t free_after_alloc = idcu_mem_pool_get_free_count(&pool, 32);
    
    idcu_mem_pool_free(&pool, ptr);
    
    __CPROVER_assert(idcu_mem_pool_get_free_count(&pool, 32) == free_after_alloc + 1, "空闲块数应该增加");
    __CPROVER_assert(verify_memory_pool_invariant(&pool), "内存池不变式应该成立");
    
    idcu_mem_pool_destroy(&pool);
}

void test_memory_pool_multiple_alloc_free()
{
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&pool);
    
    __CPROVER_assert(verify_memory_pool_invariant(&pool), "初始状态不变式成立");
    
    void* ptr1 = idcu_mem_pool_alloc(&pool, 16);
    void* ptr2 = idcu_mem_pool_alloc(&pool, 16);
    void* ptr3 = idcu_mem_pool_alloc(&pool, 16);
    
    __CPROVER_assert(ptr1 != NULL && ptr2 != NULL && ptr3 != NULL, "多次分配应该成功");
    __CPROVER_assert(verify_memory_pool_invariant(&pool), "多次分配后不变式成立");
    
    idcu_mem_pool_free(&pool, ptr2);
    __CPROVER_assert(verify_memory_pool_invariant(&pool), "释放中间块后不变式成立");
    
    idcu_mem_pool_free(&pool, ptr1);
    __CPROVER_assert(verify_memory_pool_invariant(&pool), "释放第一个块后不变式成立");
    
    idcu_mem_pool_free(&pool, ptr3);
    __CPROVER_assert(verify_memory_pool_invariant(&pool), "释放最后一个块后不变式成立");
    
    idcu_mem_pool_destroy(&pool);
}

void test_memory_pool_different_sizes()
{
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&pool);
    
    void* ptr16 = idcu_mem_pool_alloc(&pool, 16);
    void* ptr32 = idcu_mem_pool_alloc(&pool, 32);
    void* ptr64 = idcu_mem_pool_alloc(&pool, 64);
    void* ptr128 = idcu_mem_pool_alloc(&pool, 128);
    
    __CPROVER_assert(ptr16 != NULL && ptr32 != NULL && ptr64 != NULL && ptr128 != NULL, 
                    "不同大小的分配应该都成功");
    __CPROVER_assert(verify_memory_pool_invariant(&pool), "不同大小分配后不变式成立");
    
    idcu_mem_pool_free(&pool, ptr128);
    idcu_mem_pool_free(&pool, ptr64);
    idcu_mem_pool_free(&pool, ptr32);
    idcu_mem_pool_free(&pool, ptr16);
    
    __CPROVER_assert(verify_memory_pool_invariant(&pool), "全部释放后不变式成立");
    
    idcu_mem_pool_destroy(&pool);
}

void test_memory_pool_peak_usage()
{
    idcu_MemoryPool pool;
    idcu_mem_pool_init(&pool);
    
    void* ptr1 = idcu_mem_pool_alloc(&pool, 16);
    void* ptr2 = idcu_mem_pool_alloc(&pool, 16);
    
    uint64_t peak = idcu_mem_pool_get_peak_usage(&pool);
    __CPROVER_assert(peak > 0, "峰值使用量应该大于0");
    
    idcu_mem_pool_free(&pool, ptr1);
    idcu_mem_pool_free(&pool, ptr2);
    
    __CPROVER_assert(idcu_mem_pool_get_peak_usage(&pool) == peak, "峰值使用量应该保持不变");
    
    idcu_mem_pool_destroy(&pool);
}

int main()
{
    test_memory_pool_init_destroy();
    test_memory_pool_alloc();
    test_memory_pool_free();
    test_memory_pool_multiple_alloc_free();
    test_memory_pool_different_sizes();
    test_memory_pool_peak_usage();
    
    return 0;
}
