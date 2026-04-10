#ifndef IDCU_MEMORY_MEMORY_H
#define IDCU_MEMORY_MEMORY_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/memory/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化内存池
 * @param pool 内存池指针
 * @return IDCU_ERR_OK 成功，其他值表示错误
 */
int idcu_mem_pool_init(idcu_MemoryPool* pool);

/**
 * @brief 销毁内存池
 * @param pool 内存池指针
 */
void idcu_mem_pool_destroy(idcu_MemoryPool* pool);

/**
 * @brief 从内存池分配内存
 * @param pool 内存池指针
 * @param size 分配大小（字节）
 * @return 分配的内存指针，失败返回NULL
 */
void* idcu_mem_pool_alloc(idcu_MemoryPool* pool, uint32_t size);

/**
 * @brief 释放内存回内存池
 * @param pool 内存池指针
 * @param ptr 要释放的内存指针
 */
void idcu_mem_pool_free(idcu_MemoryPool* pool, void* ptr);

/**
 * @brief 获取指定尺寸的空闲块数
 * @param pool 内存池指针
 * @param size 请求大小
 * @return 空闲块数
 */
uint32_t idcu_mem_pool_get_free_count(idcu_MemoryPool* pool, uint32_t size);

/**
 * @brief 获取累计分配字节数
 * @param pool 内存池指针
 * @return 累计分配字节数
 */
uint64_t idcu_mem_pool_get_total_allocated(idcu_MemoryPool* pool);

/**
 * @brief 获取峰值使用字节数
 * @param pool 内存池指针
 * @return 峰值使用字节数
 */
uint64_t idcu_mem_pool_get_peak_usage(idcu_MemoryPool* pool);

/**
 * @brief 检查空指针
 * @param ptr 要检查的指针
 * @param context 上下文描述
 * @return IDCU_ERR_OK 成功，IDCU_ERR_INVALID_ARG 空指针
 */
int idcu_mem_check_null(const void* ptr, const char* context);

/**
 * @brief 检查缓冲区溢出
 * @param ptr 缓冲区指针
 * @param size 缓冲区大小
 * @param context 上下文描述
 * @return IDCU_ERR_OK 成功，IDCU_ERR_OUT_OF_RANGE 溢出
 */
int idcu_mem_check_overflow(const void* ptr, size_t size, const char* context);

/**
 * @brief 安全内存拷贝
 * @param dst 目标缓冲区
 * @param dst_size 目标缓冲区大小
 * @param src 源缓冲区
 * @param src_size 源大小
 * @return IDCU_ERR_OK 成功，其他值表示错误
 */
int idcu_mem_safe_copy(void* dst, size_t dst_size, const void* src, size_t src_size);

/**
 * @brief 获取安全统计信息
 * @param pool 内存池指针
 * @param null_checks 输出：空指针检查次数
 * @param overflow_checks 输出：溢出检查次数
 * @return IDCU_ERR_OK 成功
 */
int idcu_mem_pool_get_safety_stats(idcu_MemoryPool* pool, uint32_t* null_checks,
                                    uint32_t* overflow_checks);

#ifdef __cplusplus
}
#endif

#endif
