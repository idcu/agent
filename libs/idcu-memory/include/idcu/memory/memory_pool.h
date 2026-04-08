/**
 * @file memory_pool.h
 * @brief 内存池管理模块
 *
 * 提供高效的内存池实现，支持多尺寸类别的内存块管理，
 * 减少内存碎片，提高分配/释放性能。适用于频繁小内存分配场景。
 */

#ifndef IDCU_MEMORY_MEMORY_POOL_H
#define IDCU_MEMORY_MEMORY_POOL_H

#include "idcu/common/error_code.h"
#include "idcu/common/lock.h"

#include <stddef.h>
#include <stdint.h>

/** 内存池最大块数量 */
#define IDCU_MEM_POOL_MAX_BLOCKS 1024
/** 内存池最大尺寸类别数 */
#define IDCU_MEM_POOL_MAX_SIZE_CLASSES 8
/** 内存保护标记大小 */
#define IDCU_MEM_GUARD_SIZE 16
/** 内存保护标记值 */
#define IDCU_MEM_GUARD_VALUE 0xDE

/**
 * @brief 内存块头部信息
 *
 * 存储在分配内存的头部，用于快速查找块信息
 */
typedef struct
{
    uint8_t  size_class; /**< 尺寸类别索引 */
    uint32_t block_idx;  /**< 块索引 */
    uint32_t magic;      /**< 魔法数，用于验证头部有效性 */
} idcu_PoolBlockHeader;

/** 内存块头部魔法数 */
#define IDCU_POOL_HEADER_MAGIC 0x49444355 /* "IDCU" */

/**
 * @brief 内存池块结构体
 *
 * 表示单个内存块的状态和数据指针。
 */
typedef struct
{
    void*    data;       /**< 内存块数据指针（包含头部） */
    uint8_t  in_use;     /**< 是否正在使用标志，1=使用中，0=空闲 */
    uint32_t alloc_size; /**< 实际分配大小 */
    uint8_t  size_class; /**< 所属尺寸类别索引 */
} idcu_PoolBlock;

/**
 * @brief 尺寸类别结构体
 *
 * 管理相同尺寸的内存块集合，包含空闲链表。
 */
typedef struct
{
    uint32_t        block_size;  /**< 该类别的块大小（字节） */
    uint32_t        block_count; /**< 该类别的总块数 */
    idcu_PoolBlock* blocks;      /**< 块数组指针 */
    uint32_t        free_count;  /**< 空闲块数量 */
    uint32_t*       free_list;   /**< 空闲块索引链表 */
    uint32_t        free_head;   /**< 空闲链表头索引 */
    idcu_Mutex      class_lock;  /**< 每个尺寸类别的独立锁 */
} idcu_SizeClass;

/**
 * @brief 内存池主结构体
 *
 * 包含所有尺寸类别、锁和统计信息。
 */
typedef struct
{
    idcu_SizeClass size_classes[IDCU_MEM_POOL_MAX_SIZE_CLASSES]; /**< 尺寸类别数组 */
    uint32_t       num_size_classes;                             /**< 实际使用的尺寸类别数 */
    idcu_Mutex     lock;                                         /**< 线程安全锁 */
    uint64_t       total_allocated;                              /**< 累计分配字节数 */
    uint64_t       total_freed;                                  /**< 累计释放字节数 */
    uint64_t       peak_usage;                                   /**< 峰值使用字节数 */
    uint32_t       null_check_count;                             /**< 空指针检查计数 */
    uint32_t       overflow_check_count;                         /**< 溢出检查计数 */
} idcu_MemoryPool;

/**
 * @brief 初始化内存池
 *
 * @param pool 内存池指针
 * @return int 成功返回 0，失败返回非零错误码
 */
int idcu_mem_pool_init(idcu_MemoryPool* pool);

/**
 * @brief 销毁内存池，释放所有资源
 *
 * @param pool 内存池指针
 */
void idcu_mem_pool_destroy(idcu_MemoryPool* pool);

/**
 * @brief 从内存池分配内存（带安全检查）
 *
 * 根据请求大小自动选择合适的尺寸类别进行分配。
 *
 * @param pool 内存池指针
 * @param size 要分配的大小（字节）
 * @return void* 成功返回指针，失败返回 NULL
 */
void* idcu_mem_pool_alloc(idcu_MemoryPool* pool, uint32_t size);

/**
 * @brief 释放内存回内存池（带安全检查）
 *
 * @param pool 内存池指针
 * @param ptr 要释放的内存指针
 */
void idcu_mem_pool_free(idcu_MemoryPool* pool, void* ptr);

/**
 * @brief 获取指定大小的空闲块数量
 *
 * @param pool 内存池指针
 * @param size 内存大小（字节）
 * @return uint32_t 空闲块数量
 */
uint32_t idcu_mem_pool_get_free_count(idcu_MemoryPool* pool, uint32_t size);

/**
 * @brief 获取累计分配字节数
 *
 * @param pool 内存池指针
 * @return uint64_t 累计分配字节数
 */
uint64_t idcu_mem_pool_get_total_allocated(idcu_MemoryPool* pool);

/**
 * @brief 获取峰值使用字节数
 *
 * @param pool 内存池指针
 * @return uint64_t 峰值使用字节数
 */
uint64_t idcu_mem_pool_get_peak_usage(idcu_MemoryPool* pool);

/**
 * @brief 检查空指针
 *
 * @param ptr 要检查的指针
 * @param context 上下文描述
 * @return int 0=安全，非0=错误
 */
int idcu_mem_check_null(const void* ptr, const char* context);

/**
 * @brief 检查缓冲区溢出
 *
 * @param ptr 缓冲区指针
 * @param size 缓冲区大小
 * @param context 上下文描述
 * @return int 0=安全，非0=错误
 */
int idcu_mem_check_overflow(const void* ptr, size_t size, const char* context);

/**
 * @brief 安全的内存复制
 *
 * @param dst 目标缓冲区
 * @param dst_size 目标缓冲区大小
 * @param src 源缓冲区
 * @param src_size 要复制的大小
 * @return int 0=成功，非0=错误
 */
int idcu_mem_safe_copy(void* dst, size_t dst_size, const void* src, size_t src_size);

/**
 * @brief 获取安全检查统计
 *
 * @param pool 内存池指针
 * @param null_checks 空指针检查计数输出
 * @param overflow_checks 溢出检查计数输出
 * @return int 0=成功，非0=错误
 */
int idcu_mem_pool_get_safety_stats(idcu_MemoryPool* pool, uint32_t* null_checks,
                                   uint32_t* overflow_checks);

#endif  // IDCU_MEMORY_MEMORY_POOL_H
