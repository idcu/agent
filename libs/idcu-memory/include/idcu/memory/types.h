#ifndef IDCU_MEMORY_TYPES_H
#define IDCU_MEMORY_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 关键常量
#define IDCU_MEM_POOL_MAX_BLOCKS        1024
#define IDCU_MEM_POOL_MAX_SIZE_CLASSES 8
#define IDCU_MEM_GUARD_SIZE             16
#define IDCU_POOL_HEADER_MAGIC          0x49444355  // "IDCU"

// 内存块头部
typedef struct {
    uint8_t  size_class;
    uint32_t block_idx;
    uint32_t magic;
    uint8_t  guard[IDCU_MEM_GUARD_SIZE];
} idcu_PoolBlockHeader;

// 内存块
typedef struct {
    void*    data;
    uint8_t  in_use;
    uint32_t alloc_size;
    uint8_t  size_class;
    uint8_t  guard[IDCU_MEM_GUARD_SIZE];
} idcu_PoolBlock;

// 尺寸类别
typedef struct {
    uint32_t        block_size;
    uint32_t        block_count;
    idcu_PoolBlock* blocks;
    uint32_t        free_count;
    uint32_t*       free_list;
    uint32_t        free_head;
    idcu_Mutex      class_lock;
} idcu_SizeClass;

// 内存池主结构体
typedef struct {
    idcu_SizeClass size_classes[IDCU_MEM_POOL_MAX_SIZE_CLASSES];
    uint32_t       num_size_classes;
    idcu_Mutex     lock;
    uint64_t       total_allocated;
    uint64_t       total_freed;
    uint64_t       peak_usage;
    uint64_t       current_usage;
    uint32_t       null_check_count;
    uint32_t       overflow_check_count;
} idcu_MemoryPool;

#ifdef __cplusplus
}
#endif

#endif
