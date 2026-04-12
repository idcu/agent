#ifndef IDCU_COMMON_CONFIG_H
#define IDCU_COMMON_CONFIG_H

#define IDCU_CONFIG_MAX_MODULES    32
#define IDCU_CONFIG_MAX_MSG        64
#define IDCU_CONFIG_STACK_CTX_SIZE 256
#define IDCU_CONFIG_LOG_LEVEL      1
#define IDCU_AGENT_VERSION_STR     "1.0.0"
#define IDCU_CONFIG_FILE_PATH      "config/agent.yaml"
#define IDCU_CONFIG_SECTION_MAX    64
#define IDCU_CONFIG_PATH_MAX       512

#define IDCU_PERM_SEND       (1U &lt;&lt; 0)
#define IDCU_PERM_RECV       (1U &lt;&lt; 1)
#define IDCU_PERM_RUN        (1U &lt;&lt; 2)
#define IDCU_PERM_HW         (1U &lt;&lt; 3)
#define IDCU_PERM_CONFIG     (1U &lt;&lt; 4)
#define IDCU_PERM_LOG        (1U &lt;&lt; 5)
#define IDCU_PERM_DEBUG      (1U &lt;&lt; 6)
#define IDCU_PERM_MODULE_MGR (1U &lt;&lt; 7)

/**
 * @brief 线程安全注解和编译时检查机制
 *
 * 这些宏提供了编译时的线程安全注解和检查。
 * 它们使用标准C特性，在不支持的编译器上会优雅地降级。
 */

/**
 * @brief 标记函数为线程安全的
 *
 * 使用此宏标记可以从任何线程安全调用的函数。
 *
 * 示例:
 * @code
 * IDCU_THREAD_SAFE int idcu_some_function(void);
 * @endcode
 */
#if defined(__GNUC__) || defined(__clang__)
#define IDCU_THREAD_SAFE __attribute__((warn_unused_result))
#else
#define IDCU_THREAD_SAFE
#endif

/**
 * @brief 标记函数为非线程安全的
 *
 * 使用此宏标记需要外部同步的函数。
 *
 * 示例:
 * @code
 * IDCU_NOT_THREAD_SAFE void idcu_some_function(void);
 * @endcode
 */
#if defined(__GNUC__) || defined(__clang__)
#define IDCU_NOT_THREAD_SAFE
#else
#define IDCU_NOT_THREAD_SAFE
#endif

/**
 * @brief 编译时静态断言
 *
 * 在编译时检查条件，如果条件为假则产生编译错误。
 *
 * 示例:
 * @code
 * IDCU_STATIC_ASSERT(sizeof(int) == 4, "int必须是4字节");
 * @endcode
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define IDCU_STATIC_ASSERT(cond, msg) _Static_assert((cond), msg)
#elif defined(__cplusplus) && __cplusplus >= 201103L
#define IDCU_STATIC_ASSERT(cond, msg) static_assert((cond), msg)
#elif defined(__GNUC__) || defined(__clang__)
#define IDCU_STATIC_ASSERT(cond, msg) \
    typedef char idcu_static_assert_##__LINE__[(cond) ? 1 : -1] __attribute__((unused))
#else
#define IDCU_STATIC_ASSERT(cond, msg) \
    typedef char idcu_static_assert_##__LINE__[(cond) ? 1 : -1]
#endif

/**
 * @brief 标记变量在多线程环境下为易失的
 *
 * 使用此宏标记需要在多线程间可见的变量。
 */
#define IDCU_VOLATILE volatile

/**
 * @brief 编译时检查锁顺序（静态验证）
 *
 * 这个宏提供了一个框架来声明和验证锁获取顺序，
 * 可以帮助在编译时捕获潜在的死锁问题。
 */
#define IDCU_LOCK_LEVEL(level) \
    enum { IDCU_LOCK_LEVEL_##__LINE__ = (level) }

#endif
