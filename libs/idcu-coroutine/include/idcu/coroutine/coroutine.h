/**
 * @file coroutine.h
 * @brief 协程调度模块
 *
 * 提供用户态协程实现，支持多优先级调度、时间片轮转，
 * 适用于高并发 IO 密集型场景。
 */

#ifndef IDCU_COROUTINE_COROUTINE_H
#define IDCU_COROUTINE_COROUTINE_H

#include "context.h"
#include "idcu/common/config.h"
#include "idcu/common/lock.h"

/** 最大协程优先级数 */
#define IDCU_CORO_MAX_PRIO 64
/** 最大协程数量 */
#define IDCU_CORO_MAX_COUNT 256
/** 默认时间片（毫秒） */
#define IDCU_CORO_DEFAULT_TIMESLICE 10

/**
 * @brief 协程状态枚举
 */
typedef enum
{
    IDCU_CORO_IDLE,      /**< 空闲状态，未使用 */
    IDCU_CORO_READY,     /**< 就绪状态，等待调度 */
    IDCU_CORO_RUNNING,   /**< 运行状态，正在执行 */
    IDCU_CORO_SUSPENDED, /**< 挂起状态，主动让出 */
    IDCU_CORO_FINISHED   /**< 结束状态，执行完成 */
} idcu_CoroState;

typedef struct idcu_Coroutine idcu_Coroutine;

/**
 * @brief 协程统计信息
 */
typedef struct
{
    uint64_t total_runtime_us; /**< 总运行时间（微秒） */
    uint64_t switch_count;     /**< 切换次数 */
    uint64_t last_switch_ts;   /**< 上次切换时间戳 */
    uint32_t timeslice_used;   /**< 当前时间片已使用 */
} idcu_CoroStats;

/**
 * @brief 协程结构体
 *
 * 包含协程的标识、优先级、状态、上下文和统计信息。
 */
struct idcu_Coroutine
{
    uint32_t          id;                    /**< 协程唯一标识符 */
    uint32_t          prio;                  /**< 优先级，数值越小优先级越高 */
    idcu_CoroState    state;                 /**< 当前状态 */
    idcu_StackContext ctx;                   /**< 栈上下文 */
    idcu_CoroState (*func)(idcu_Coroutine*); /**< 协程入口函数 */
    uint32_t       timeslice;                /**< 时间片（毫秒） */
    idcu_CoroStats stats;                    /**< 统计信息 */
    void*          user_data;                /**< 用户自定义数据 */
};

/**
 * @brief 环形队列结构体
 *
 * 用于实现就绪队列。
 */
typedef struct
{
    uint32_t items[IDCU_CORO_MAX_COUNT]; /**< 队列元素数组 */
    uint32_t head;                       /**< 队头索引 */
    uint32_t tail;                       /**< 队尾索引 */
    uint32_t count;                      /**< 元素数量 */
} idcu_CircularQueue;

/**
 * @brief 协程调度器结构体
 *
 * 管理所有协程、就绪队列和调度状态。
 */
typedef struct
{
    idcu_Coroutine     coros[IDCU_CORO_MAX_COUNT];           /**< 协程数组 */
    idcu_CircularQueue ready_queues[IDCU_CORO_MAX_PRIO];     /**< 各优先级就绪队列 */
    uint32_t           id_to_index[IDCU_CORO_MAX_COUNT + 1]; /**< ID 到索引的映射 */
    uint32_t           count;                                /**< 活跃协程数 */
    uint32_t           current;                              /**< 当前运行的协程索引 */
    uint32_t           current_prio;                         /**< 当前优先级 */
    uint32_t           next_id;                              /**< 下一个协程 ID */
    idcu_Mutex         lock;                                 /**< 线程安全锁 */
    uint64_t           last_ts;                              /**< 上次调度时间戳 */
} idcu_CoroScheduler;

/**
 * @brief 初始化协程调度器
 *
 * @param sched 调度器指针
 */
void idcu_coro_sched_init(idcu_CoroScheduler* sched);

/**
 * @brief 销毁协程调度器
 *
 * @param sched 调度器指针
 */
void idcu_coro_sched_destroy(idcu_CoroScheduler* sched);

/**
 * @brief 运行协程调度器
 *
 * 开始协程调度循环，直到所有协程结束。
 *
 * @param sched 调度器指针
 * @return idcu_CoroState 最终状态
 */
idcu_CoroState idcu_coro_sched_run(idcu_CoroScheduler* sched);

/**
 * @brief 创建新协程
 *
 * @param sched 调度器指针
 * @param func 协程入口函数
 * @param prio 优先级
 * @param timeslice 时间片（毫秒）
 * @param user_data 用户自定义数据
 * @return int 成功返回协程 ID（>0），失败返回 -1
 */
int idcu_coro_create(idcu_CoroScheduler* sched, idcu_CoroState (*func)(idcu_Coroutine*),
                     uint32_t prio, uint32_t timeslice, void* user_data);

/**
 * @brief 销毁协程
 *
 * @param sched 调度器指针
 * @param id 协程 ID
 * @return int 成功返回 0，失败返回 -1
 */
int idcu_coro_destroy(idcu_CoroScheduler* sched, uint32_t id);

/**
 * @brief 挂起协程
 *
 * 主动让出 CPU，进入挂起状态。
 *
 * @param sched 调度器指针
 * @param id 协程 ID
 * @return int 成功返回 0，失败返回 -1
 */
int idcu_coro_suspend(idcu_CoroScheduler* sched, uint32_t id);

/**
 * @brief 恢复协程
 *
 * 将挂起的协程重新放入就绪队列。
 *
 * @param sched 调度器指针
 * @param id 协程 ID
 * @return int 成功返回 0，失败返回 -1
 */
int idcu_coro_resume(idcu_CoroScheduler* sched, uint32_t id);

/**
 * @brief 获取协程指针
 *
 * @param sched 调度器指针
 * @param id 协程 ID
 * @return idcu_Coroutine* 成功返回指针，失败返回 NULL
 */
idcu_Coroutine* idcu_coro_get(idcu_CoroScheduler* sched, uint32_t id);

/**
 * @brief 获取就绪队列中的协程总数
 *
 * @param sched 调度器指针
 * @return uint32_t 就绪协程数量
 */
uint32_t idcu_coro_get_ready_count(idcu_CoroScheduler* sched);

#endif  // IDCU_COROUTINE_COROUTINE_H
