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
#define IDCU_MEM_MAX_STACK_FRAMES       32
#define IDCU_MEM_MAX_SNAPSHOTS          16

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
    const char* file;
    int line;
    uint64_t timestamp;
    int stack_depth;
    void* stack_frames[IDCU_MEM_MAX_STACK_FRAMES];
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
    uint64_t        alloc_count;
    uint64_t        free_count_total;
} idcu_SizeClass;

// 内存统计信息
typedef struct {
    uint64_t total_allocated;
    uint64_t total_freed;
    uint64_t current_usage;
    uint64_t peak_usage;
    uint64_t active_allocations;
    uint64_t size_class_stats[IDCU_MEM_POOL_MAX_SIZE_CLASSES];
    uint64_t leak_count;
} idcu_MemoryStats;

// 内存统计快照
typedef struct {
    uint64_t timestamp;
    uint64_t current_usage;
    uint64_t peak_usage;
    uint64_t alloc_count;
} idcu_MemoryStatsSnapshot;

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
    int            debug_enabled;
    idcu_MemoryStatsSnapshot snapshots[IDCU_MEM_MAX_SNAPSHOTS];
    uint32_t       snapshot_index;
    uint32_t       snapshot_count;
} idcu_MemoryPool;

#ifdef __cplusplus
}
#endif

#endif
