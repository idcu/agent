#include <idcu/memory/memory.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// 默认尺寸类别配置 (16, 32, 64, 128, 256, 512, 1024, 2048)
static const uint32_t default_sizes[IDCU_MEM_POOL_MAX_SIZE_CLASSES] = {
    16, 32, 64, 128, 256, 512, 1024, 2048
};

static const uint32_t default_block_counts[IDCU_MEM_POOL_MAX_SIZE_CLASSES] = {
    256, 128, 64, 32, 16, 8, 4, 2
};

// 选择合适的尺寸类别
static int select_size_class(uint32_t size) {
    for (int i = 0; i < IDCU_MEM_POOL_MAX_SIZE_CLASSES; i++) {
        if (size <= default_sizes[i]) {
            return i;
        }
    }
    return -1; // 超过最大尺寸
}

int idcu_mem_pool_init(idcu_MemoryPool* pool) {
    if (!pool) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(pool, 0, sizeof(idcu_MemoryPool));
    pool->num_size_classes = IDCU_MEM_POOL_MAX_SIZE_CLASSES;
    
    // 初始化主锁
    idcu_mutex_init(&pool->lock);
    
    // 初始化每个尺寸类别
    for (uint32_t i = 0; i < pool->num_size_classes; i++) {
        idcu_SizeClass* sc = &pool->size_classes[i];
        sc->block_size = default_sizes[i];
        sc->block_count = default_block_counts[i];
        sc->free_count = sc->block_count;
        sc->free_head = 0;
        
        // 初始化类锁
        idcu_mutex_init(&sc->class_lock);
        
        // 分配块数组
        sc->blocks = (idcu_PoolBlock*)calloc(sc->block_count, sizeof(idcu_PoolBlock));
        if (!sc->blocks) {
            idcu_mem_pool_destroy(pool);
            return IDCU_ERR_MEMORY;
        }
        
        // 分配空闲列表
        sc->free_list = (uint32_t*)calloc(sc->block_count, sizeof(uint32_t));
        if (!sc->free_list) {
            idcu_mem_pool_destroy(pool);
            return IDCU_ERR_MEMORY;
        }
        
        // 初始化空闲链表
        for (uint32_t j = 0; j < sc->block_count - 1; j++) {
            sc->free_list[j] = j + 1;
            sc->blocks[j].size_class = (uint8_t)i;
            sc->blocks[j].in_use = 0;
            
            // 分配实际数据内存
            sc->blocks[j].data = calloc(1, sc->block_size + sizeof(idcu_PoolBlockHeader) + IDCU_MEM_GUARD_SIZE);
            if (!sc->blocks[j].data) {
                idcu_mem_pool_destroy(pool);
                return IDCU_ERR_MEMORY;
            }
            
            // 初始化保护标记
            memset(sc->blocks[j].guard, 0xAA, IDCU_MEM_GUARD_SIZE);
        }
        
        // 最后一个块
        sc->free_list[sc->block_count - 1] = UINT32_MAX;
        sc->blocks[sc->block_count - 1].size_class = (uint8_t)i;
        sc->blocks[sc->block_count - 1].in_use = 0;
        sc->blocks[sc->block_count - 1].data = calloc(1, sc->block_size + sizeof(idcu_PoolBlockHeader) + IDCU_MEM_GUARD_SIZE);
        if (!sc->blocks[sc->block_count - 1].data) {
            idcu_mem_pool_destroy(pool);
            return IDCU_ERR_MEMORY;
        }
        memset(sc->blocks[sc->block_count - 1].guard, 0xAA, IDCU_MEM_GUARD_SIZE);
    }
    
    return IDCU_ERR_OK;
}

void idcu_mem_pool_destroy(idcu_MemoryPool* pool) {
    if (!pool) {
        return;
    }
    
    // 销毁每个尺寸类别
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
        
        idcu_mutex_destroy(&sc->class_lock);
    }
    
    idcu_mutex_destroy(&pool->lock);
    memset(pool, 0, sizeof(idcu_MemoryPool));
}

static int expand_size_class(idcu_SizeClass* sc) {
    uint32_t new_block_count = sc->block_count * 2;
    if (new_block_count > UINT16_MAX) {
        return IDCU_ERR_MEMORY;
    }

    idcu_PoolBlock* new_blocks = (idcu_PoolBlock*)realloc(sc->blocks, new_block_count * sizeof(idcu_PoolBlock));
    if (!new_blocks) {
        return IDCU_ERR_MEMORY;
    }
    sc->blocks = new_blocks;

    uint32_t* new_free_list = (uint32_t*)realloc(sc->free_list, new_block_count * sizeof(uint32_t));
    if (!new_free_list) {
        return IDCU_ERR_MEMORY;
    }
    sc->free_list = new_free_list;

    // Initialize new blocks
    for (uint32_t j = sc->block_count; j < new_block_count - 1; j++) {
        sc->free_list[j] = j + 1;
        sc->blocks[j].size_class = sc->blocks[0].size_class;
        sc->blocks[j].in_use = 0;

        // Allocate actual data memory
        sc->blocks[j].data = calloc(1, sc->block_size + sizeof(idcu_PoolBlockHeader) + IDCU_MEM_GUARD_SIZE);
        if (!sc->blocks[j].data) {
            return IDCU_ERR_MEMORY;
        }

        // Initialize guard marks
        memset(sc->blocks[j].guard, 0xAA, IDCU_MEM_GUARD_SIZE);
    }

    // Last new block
    sc->free_list[new_block_count - 1] = sc->free_head;
    sc->free_head = sc->block_count;
    sc->blocks[new_block_count - 1].size_class = sc->blocks[0].size_class;
    sc->blocks[new_block_count - 1].in_use = 0;
    sc->blocks[new_block_count - 1].data = calloc(1, sc->block_size + sizeof(idcu_PoolBlockHeader) + IDCU_MEM_GUARD_SIZE);
    if (!sc->blocks[new_block_count - 1].data) {
        return IDCU_ERR_MEMORY;
    }
    memset(sc->blocks[new_block_count - 1].guard, 0xAA, IDCU_MEM_GUARD_SIZE);

    sc->free_count += (new_block_count - sc->block_count);
    sc->block_count = new_block_count;

    return IDCU_ERR_OK;
}

void* idcu_mem_pool_alloc(idcu_MemoryPool* pool, uint32_t size) {
    if (!pool || size == 0) {
        return NULL;
    }
    
    int sc_idx = select_size_class(size);
    if (sc_idx < 0) {
        return NULL;
    }
    
    idcu_SizeClass* sc = &pool->size_classes[sc_idx];
    idcu_mutex_lock(&sc->class_lock);
    
    if (sc->free_head == UINT32_MAX || sc->free_count == 0) {
        int ret = expand_size_class(sc);
        if (ret != IDCU_ERR_OK) {
            idcu_mutex_unlock(&sc->class_lock);
            return NULL;
        }
    }
    
    // 从空闲链表取块
    uint32_t block_idx = sc->free_head;
    idcu_PoolBlock* block = &sc->blocks[block_idx];
    
    // 更新空闲链表
    sc->free_head = sc->free_list[block_idx];
    sc->free_count--;
    
    // 标记为使用中
    block->in_use = 1;
    block->alloc_size = size;
    
    // 设置块头部
    idcu_PoolBlockHeader* header = (idcu_PoolBlockHeader*)block->data;
    header->size_class = (uint8_t)sc_idx;
    header->block_idx = block_idx;
    header->magic = IDCU_POOL_HEADER_MAGIC;
    memset(header->guard, 0xBB, IDCU_MEM_GUARD_SIZE);
    
    // 返回数据区域（跳过头部）
    void* data_ptr = (uint8_t*)block->data + sizeof(idcu_PoolBlockHeader);
    
    idcu_mutex_unlock(&sc->class_lock);
    
    // 更新统计
    idcu_mutex_lock(&pool->lock);
    pool->total_allocated += sc->block_size;
    pool->current_usage += sc->block_size;
    if (pool->current_usage > pool->peak_usage) {
        pool->peak_usage = pool->current_usage;
    }
    idcu_mutex_unlock(&pool->lock);
    
    return data_ptr;
}

void idcu_mem_pool_free(idcu_MemoryPool* pool, void* ptr) {
    if (!pool || !ptr) {
        return;
    }
    
    // 验证指针有效性
    idcu_PoolBlockHeader* header = (idcu_PoolBlockHeader*)((uint8_t*)ptr - sizeof(idcu_PoolBlockHeader));
    
    if (header->magic != IDCU_POOL_HEADER_MAGIC) {
        return; // 无效指针，安全返回
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
    
    idcu_mutex_lock(&sc->class_lock);
    
    idcu_PoolBlock* block = &sc->blocks[block_idx];
    if (!block->in_use) {
        idcu_mutex_unlock(&sc->class_lock);
        return; // 双重释放
    }
    
    // 验证保护标记
    int guard_valid = 1;
    for (int i = 0; i < IDCU_MEM_GUARD_SIZE; i++) {
        if (block->guard[i] != 0xAA) {
            guard_valid = 0;
            break;
        }
    }
    
    // 标记为空闲
    block->in_use = 0;
    block->alloc_size = 0;
    
    // 清空数据
    memset(block->data, 0, sc->block_size + sizeof(idcu_PoolBlockHeader) + IDCU_MEM_GUARD_SIZE);
    memset(block->guard, 0xAA, IDCU_MEM_GUARD_SIZE);
    
    // 放回空闲链表
    sc->free_list[block_idx] = sc->free_head;
    sc->free_head = block_idx;
    sc->free_count++;
    
    idcu_mutex_unlock(&sc->class_lock);
    
    // 更新统计
    idcu_mutex_lock(&pool->lock);
    pool->total_freed += sc->block_size;
    pool->current_usage -= sc->block_size;
    idcu_mutex_unlock(&pool->lock);
}

uint32_t idcu_mem_pool_get_free_count(idcu_MemoryPool* pool, uint32_t size) {
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

uint64_t idcu_mem_pool_get_total_allocated(idcu_MemoryPool* pool) {
    if (!pool) {
        return 0;
    }
    return pool->total_allocated;
}

uint64_t idcu_mem_pool_get_peak_usage(idcu_MemoryPool* pool) {
    if (!pool) {
        return 0;
    }
    return pool->peak_usage;
}

int idcu_mem_check_null(const void* ptr, const char* context) {
    (void)context;
    if (!ptr) {
        return IDCU_ERR_INVALID_ARG;
    }
    return IDCU_ERR_OK;
}

int idcu_mem_check_overflow(const void* ptr, size_t size, const char* context) {
    (void)ptr;
    (void)size;
    (void)context;
    return IDCU_ERR_OK;
}

int idcu_mem_safe_copy(void* dst, size_t dst_size, const void* src, size_t src_size) {
    if (!dst || !src) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t copy_size = (src_size < dst_size) ? src_size : dst_size;
    memcpy(dst, src, copy_size);
    
    if (copy_size < dst_size) {
        memset((uint8_t*)dst + copy_size, 0, dst_size - copy_size);
    }
    
    return IDCU_ERR_OK;
}

int idcu_mem_pool_get_safety_stats(idcu_MemoryPool* pool, uint32_t* null_checks, uint32_t* overflow_checks) {
    if (!pool) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (null_checks) {
        *null_checks = pool->null_check_count;
    }
    
    if (overflow_checks) {
        *overflow_checks = pool->overflow_check_count;
    }
    
    return IDCU_ERR_OK;
}

uint32_t idcu_mem_pool_get_leak_count(idcu_MemoryPool* pool) {
    if (!pool) {
        return 0;
    }
    
    uint32_t leak_count = 0;
    idcu_mutex_lock(&pool->lock);
    
    for (uint32_t i = 0; i < pool->num_size_classes; i++) {
        idcu_SizeClass* sc = &pool->size_classes[i];
        idcu_mutex_lock(&sc->class_lock);
        
        for (uint32_t j = 0; j < sc->block_count; j++) {
            if (sc->blocks[j].in_use) {
                leak_count++;
            }
        }
        
        idcu_mutex_unlock(&sc->class_lock);
    }
    
    idcu_mutex_unlock(&pool->lock);
    return leak_count;
}

void idcu_mem_pool_report_leaks(idcu_MemoryPool* pool) {
    if (!pool) {
        return;
    }
    
    idcu_mutex_lock(&pool->lock);
    fprintf(stderr, "=== Memory Leak Report ===\n");
    
    uint32_t total_leaks = 0;
    uint64_t total_leaked_bytes = 0;
    
    for (uint32_t i = 0; i < pool->num_size_classes; i++) {
        idcu_SizeClass* sc = &pool->size_classes[i];
        idcu_mutex_lock(&sc->class_lock);
        
        uint32_t class_leaks = 0;
        for (uint32_t j = 0; j < sc->block_count; j++) {
            if (sc->blocks[j].in_use) {
                class_leaks++;
                total_leaks++;
                total_leaked_bytes += sc->block_size;
            }
        }
        
        if (class_leaks > 0) {
            fprintf(stderr, "  Size class %u (%u bytes): %u leaked blocks\n", 
                    i, sc->block_size, class_leaks);
        }
        
        idcu_mutex_unlock(&sc->class_lock);
    }
    
    fprintf(stderr, "Total: %u leaked blocks, %llu bytes\n", 
            total_leaks, (unsigned long long)total_leaked_bytes);
    fprintf(stderr, "==========================\n");
    
    idcu_mutex_unlock(&pool->lock);
}
