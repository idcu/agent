#ifndef IDCU_COMMON_ERROR_CODE_H
#define IDCU_COMMON_ERROR_CODE_H

/**
 * @file error_code.h
 * @brief 统一的错误码定义
 * 
 * 本文件定义了 IDCU Agent 系统的统一错误码，
 * 所有模块应使用此处定义的错误码进行错误处理。
 */

typedef enum {
    // ========================================
    // 成功状态码 (0)
    // ========================================
    IDCU_ERR_OK             = 0,    /**< 成功（别名） */
    IDCU_ERR_SUCCESS        = 0,    /**< 成功 */

    // ========================================
    // 通用错误类 (-1 ~ -99)
    // ========================================
    IDCU_ERR_GENERAL        = -1,   /**< 通用错误 */
    IDCU_ERR_INVALID_PARAM  = -2,   /**< 参数无效 */
    IDCU_ERR_INVALID_ARG    = -2,   /**< 参数无效（别名） */
    IDCU_ERR_NO_MEMORY      = -3,   /**< 内存不足（主要名称） */
    IDCU_ERR_OUT_OF_MEM     = -3,   /**< 内存不足（别名） */
    IDCU_ERR_PERM_DENIED    = -4,   /**< 权限拒绝 */
    IDCU_ERR_QUEUE_FULL     = -5,   /**< 队列满 */
    IDCU_ERR_QUEUE_EMPTY    = -6,   /**< 队列空 */
    IDCU_ERR_MODULE_NOT_FOUND = -7, /**< 模块未找到 */
    IDCU_ERR_MODULE_INIT    = -8,   /**< 模块初始化失败 */
    IDCU_ERR_CONFIG_LOAD    = -9,   /**< 配置加载失败 */
    IDCU_ERR_MSG_SEND       = -10,  /**< 消息发送失败 */
    IDCU_ERR_MSG_RECV       = -11,  /**< 消息接收失败 */
    IDCU_ERR_CORO_CREATE    = -12,  /**< 协程创建失败 */
    IDCU_ERR_SANDBOX_INIT   = -13,  /**< 沙箱初始化失败 */
    IDCU_ERR_MODULE_LOAD    = -14,  /**< 模块加载失败 */
    IDCU_ERR_MODULE_INVALID = -15,  /**< 模块无效 */
    IDCU_ERR_NOT_FOUND      = -16,  /**< 未找到 */
    IDCU_ERR_ALREADY_EXISTS = -17,  /**< 已存在 */
    IDCU_ERR_BUSY           = -18,  /**< 忙 */
    IDCU_ERR_MODULE_RUN     = -19,  /**< 模块运行失败 */
    IDCU_ERR_MODULE_STOP    = -20,  /**< 模块停止失败 */
    IDCU_ERR_MODULE_CRASH   = -21,  /**< 模块崩溃 */
    IDCU_ERR_DEPENDENCY     = -22,  /**< 依赖错误 */
    IDCU_ERR_CIRCULAR_DEP   = -23,  /**< 循环依赖 */
    IDCU_ERR_VERSION_MISMATCH = -24, /**< 版本不匹配 */
    IDCU_ERR_NOT_INITIALIZED = -25, /**< 未初始化 */
    IDCU_ERR_LOCK_FAILED    = -26,  /**< 加锁失败 */
    IDCU_ERR_UNLOCK_FAILED  = -27,  /**< 解锁失败 */
    IDCU_ERR_MSG_TIMEOUT    = -28,  /**< 消息超时 */
    IDCU_ERR_MSG_DROPPED    = -29,  /**< 消息丢弃 */
    IDCU_ERR_HEALTH_CHECK_FAIL = -30, /**< 健康检查失败 */
    IDCU_ERR_CONFIG_PARSE   = -31,  /**< 配置解析错误 */

    // ========================================
    // 扩展错误码 (新增，向后兼容)
    // ========================================
    IDCU_ERR_TIMEOUT        = -32,  /**< 超时 */
    IDCU_ERR_CANCELLED      = -33,  /**< 已取消 */
    IDCU_ERR_CONFIG_INVALID = -34,  /**< 配置无效 */
    IDCU_ERR_DEADLOCK       = -35,  /**< 死锁检测 */
    IDCU_ERR_SANDBOX_ACCESS = -36, /**< 沙箱访问拒绝 */
    IDCU_ERR_SANDBOX_VIOLATION = -37, /**< 沙箱违规 */
    IDCU_ERR_NETWORK_INIT   = -38,  /**< 网络初始化失败 */
    IDCU_ERR_NETWORK_CONNECT = -39, /**< 网络连接失败 */
    IDCU_ERR_NETWORK_SEND   = -40,  /**< 网络发送失败 */
    IDCU_ERR_NETWORK_RECV   = -41,  /**< 网络接收失败 */
    IDCU_ERR_NETWORK_TIMEOUT = -42, /**< 网络超时 */
    IDCU_ERR_METRICS_COLLECT   = -43, /**< 指标收集失败 */
    IDCU_ERR_FILE_OPEN      = -44,  /**< 文件打开失败 */
    IDCU_ERR_FILE_READ      = -45,  /**< 文件读取失败 */
    IDCU_ERR_FILE_WRITE     = -46,  /**< 文件写入失败 */
    IDCU_ERR_FILE_DELETE    = -47,  /**< 文件删除失败 */
    IDCU_ERR_CORO_SCHEDULE  = -48,  /**< 协程调度失败 */
    IDCU_ERR_CORO_YIELD     = -49   /**< 协程让出失败 */
} idcu_ErrorCode;

/**
 * @brief 将错误码转换为可读的字符串描述
 * 
 * @param err_code 错误码
 * @return const char* 错误描述字符串
 */
const char* idcu_err_to_str(int err_code);

/**
 * @brief 检查错误码是否表示成功
 * 
 * @param err_code 错误码
 * @return int 1表示成功，0表示失败
 */
static inline int idcu_err_is_ok(int err_code) {
    return err_code == IDCU_ERR_OK;
}

#endif // IDCU_COMMON_ERROR_CODE_H