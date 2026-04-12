#ifndef IDCU_COMMON_ERROR_CODE_H
#define IDCU_COMMON_ERROR_CODE_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @file error_code.h
 * @brief IDCU 错误码定义和错误处理
 *
 * 本模块定义了 IDCU Agent 项目中使用的所有错误码，
 * 以及错误信息管理、错误链、堆栈跟踪等功能。
 */

/**
 * @brief IDCU 错误码枚举
 *
 * 所有错误码均为负数，0 表示成功。
 */
typedef enum
{
    IDCU_ERR_OK      = 0,  /**< 操作成功 */
    IDCU_ERR_SUCCESS = 0,  /**< 操作成功（别名） */

    IDCU_ERR_GENERAL           = -1,  /**< 通用错误 */
    IDCU_ERR_INVALID_PARAM     = -2,  /**< 无效参数 */
    IDCU_ERR_INVALID_ARG       = -2,  /**< 无效参数（别名） */
    IDCU_ERR_NO_MEMORY         = -3,  /**< 内存不足 */
    IDCU_ERR_OUT_OF_MEM        = -3,  /**< 内存不足（别名） */
    IDCU_ERR_PERM_DENIED       = -4,  /**< 权限被拒绝 */
    IDCU_ERR_QUEUE_FULL        = -5,  /**< 队列已满 */
    IDCU_ERR_QUEUE_EMPTY       = -6,  /**< 队列为空 */
    IDCU_ERR_MODULE_NOT_FOUND  = -7,  /**< 模块未找到 */
    IDCU_ERR_MODULE_INIT       = -8,  /**< 模块初始化失败 */
    IDCU_ERR_CONFIG_LOAD       = -9,  /**< 配置加载失败 */
    IDCU_ERR_MSG_SEND          = -10, /**< 消息发送失败 */
    IDCU_ERR_MSG_RECV          = -11, /**< 消息接收失败 */
    IDCU_ERR_CORO_CREATE       = -12, /**< 协程创建失败 */
    IDCU_ERR_SANDBOX_INIT      = -13, /**< 沙箱初始化失败 */
    IDCU_ERR_MODULE_LOAD       = -14, /**< 模块加载失败 */
    IDCU_ERR_MODULE_INVALID    = -15, /**< 无效模块 */
    IDCU_ERR_NOT_FOUND         = -16, /**< 资源未找到 */
    IDCU_ERR_ALREADY_EXISTS    = -17, /**< 资源已存在 */
    IDCU_ERR_BUSY              = -18, /**< 系统忙 */
    IDCU_ERR_MODULE_RUN        = -19, /**< 模块运行失败 */
    IDCU_ERR_MODULE_STOP       = -20, /**< 模块停止失败 */
    IDCU_ERR_MODULE_CRASH      = -21, /**< 模块崩溃 */
    IDCU_ERR_DEPENDENCY        = -22, /**< 依赖错误 */
    IDCU_ERR_CIRCULAR_DEP      = -23, /**< 循环依赖 */
    IDCU_ERR_VERSION_MISMATCH  = -24, /**< 版本不匹配 */
    IDCU_ERR_NOT_INITIALIZED   = -25, /**< 未初始化 */
    IDCU_ERR_LOCK_FAILED       = -26, /**< 加锁失败 */
    IDCU_ERR_UNLOCK_FAILED     = -27, /**< 解锁失败 */
    IDCU_ERR_MSG_TIMEOUT       = -28, /**< 消息超时 */
    IDCU_ERR_MSG_DROPPED       = -29, /**< 消息被丢弃 */
    IDCU_ERR_HEALTH_CHECK_FAIL = -30, /**< 健康检查失败 */
    IDCU_ERR_CONFIG_PARSE      = -31, /**< 配置解析失败 */
    IDCU_ERR_TIMEOUT           = -32, /**< 超时 */
    IDCU_ERR_CANCELLED         = -33, /**< 已取消 */
    IDCU_ERR_CONFIG_INVALID    = -34, /**< 无效配置 */
    IDCU_ERR_DEADLOCK          = -35, /**< 死锁 */
    IDCU_ERR_SANDBOX_ACCESS    = -36, /**< 沙箱访问错误 */
    IDCU_ERR_SANDBOX_VIOLATION = -37, /**< 沙箱违规 */
    IDCU_ERR_NETWORK_INIT      = -38, /**< 网络初始化失败 */
    IDCU_ERR_NETWORK_CONNECT   = -39, /**< 网络连接失败 */
    IDCU_ERR_NETWORK_SEND      = -40, /**< 网络发送失败 */
    IDCU_ERR_NETWORK_RECV      = -41, /**< 网络接收失败 */
    IDCU_ERR_NETWORK_TIMEOUT   = -42, /**< 网络超时 */
    IDCU_ERR_METRICS_COLLECT   = -43, /**< 指标收集失败 */
    IDCU_ERR_FILE_OPEN         = -44, /**< 文件打开失败 */
    IDCU_ERR_FILE_READ         = -45, /**< 文件读取失败 */
    IDCU_ERR_FILE_WRITE        = -46, /**< 文件写入失败 */
    IDCU_ERR_FILE_DELETE       = -47, /**< 文件删除失败 */
    IDCU_ERR_CORO_SCHEDULE     = -48, /**< 协程调度失败 */
    IDCU_ERR_CORO_YIELD        = -49, /**< 协程让出失败 */
    IDCU_ERR_BUFFER_TOO_SMALL  = -50, /**< 缓冲区太小 */
    IDCU_ERR_VERSION_TOO_OLD   = -51, /**< 版本过旧 */
    IDCU_ERR_VERSION_TOO_NEW   = -52, /**< 版本过新 */
    IDCU_ERR_VERSION_INCOMPATIBLE = -53, /**< 版本不兼容 */
    IDCU_ERR_MEMORY            = -54, /**< 内存错误 */
    IDCU_ERR_LIMIT_EXCEEDED    = -55, /**< 超出限制 */
    IDCU_ERR_IO_ERROR          = -56, /**< IO 错误 */
    IDCU_ERR_FILE_ERROR        = -56, /**< 文件错误（别名） */
    IDCU_ERR_SECURITY_ERROR    = -57, /**< 安全错误 */
    IDCU_ERR_RESOURCE_EXHAUSTED = -58, /**< 资源耗尽 */
    IDCU_ERR_VERIFICATION_FAILED = -59, /**< 验证失败 */
    IDCU_ERR_UNKNOWN           = -60, /**< 未知错误 */
    IDCU_ERR_INVALID_STATE     = -61, /**< 无效状态 */
    IDCU_ERR_OUT_OF_RANGE      = -62  /**< 超出范围 */
} idcu_ErrorCode;

#define IDCU_SUCCESS                    IDCU_ERR_SUCCESS
#define IDCU_ERROR_INVALID_PARAM        IDCU_ERR_INVALID_PARAM
#define IDCU_ERROR_MEMORY               IDCU_ERR_MEMORY
#define IDCU_ERROR_BUFFER_TOO_SMALL     IDCU_ERR_BUFFER_TOO_SMALL
#define IDCU_ERROR_VERSION_TOO_OLD      IDCU_ERR_VERSION_TOO_OLD
#define IDCU_ERROR_VERSION_TOO_NEW      IDCU_ERR_VERSION_TOO_NEW
#define IDCU_ERROR_VERSION_INCOMPATIBLE IDCU_ERR_VERSION_INCOMPATIBLE

#define IDCU_ERROR_CONTEXT_MAX_LEN 256
#define IDCU_ERROR_STACKTRACE_MAX_FRAMES 32
#define IDCU_ERROR_CHAIN_MAX_DEPTH 16

/**
 * @brief 错误信息结构体（前向声明）
 */
typedef struct idcu_ErrorInfo idcu_ErrorInfo;

/**
 * @brief 错误信息结构体
 *
 * 包含完整的错误信息，包括错误码、上下文、源位置、
 * 错误链和堆栈跟踪（调试模式）。
 */
struct idcu_ErrorInfo
{
    int         error_code;                        /**< 错误码 */
    char        context[IDCU_ERROR_CONTEXT_MAX_LEN]; /**< 错误上下文描述 */
    const char* file;                              /**< 发生错误的源文件 */
    int         line;                              /**< 发生错误的行号 */
    idcu_ErrorInfo* cause;                         /**< 错误链中的前置错误 */
    int         stacktrace_frame_count;           /**< 堆栈跟踪帧数 */
    void*       stacktrace_frames[IDCU_ERROR_STACKTRACE_MAX_FRAMES]; /**< 堆栈跟踪帧 */
};

/**
 * @brief 将错误码转换为字符串描述
 * @param err_code 错误码
 * @return 错误描述字符串
 */
const char* idcu_err_to_str(int err_code);

/**
 * @brief 检查错误码是否表示成功
 * @param err_code 错误码
 * @return true 表示成功，false 表示失败
 */
static inline int idcu_err_is_ok(int err_code)
{
    return err_code == IDCU_ERR_OK;
}

/**
 * @brief 设置最近的错误信息
 * @param err_code 错误码
 * @param context 错误上下文描述
 * @param file 源文件路径
 * @param line 源文件行号
 */
void idcu_err_set_last_error(int err_code, const char* context, const char* file, int line);

/**
 * @brief 获取最近的错误信息
 * @return 错误信息指针，若无错误则返回 NULL
 */
const idcu_ErrorInfo* idcu_err_get_last_error(void);

/**
 * @brief 清除最近的错误信息
 */
void idcu_err_clear_last_error(void);

/**
 * @brief 设置带错误链的最近错误信息
 * @param err_code 错误码
 * @param context 错误上下文描述
 * @param file 源文件路径
 * @param line 源文件行号
 * @param cause 前置错误（错误链）
 */
void idcu_err_set_last_error_with_cause(int err_code, const char* context, 
                                         const char* file, int line, idcu_ErrorInfo* cause);

/**
 * @brief 克隆错误信息
 * @param error 源错误信息
 * @return 克隆的错误信息，失败返回 NULL
 */
idcu_ErrorInfo* idcu_err_clone_error(const idcu_ErrorInfo* error);

/**
 * @brief 释放错误信息
 * @param error 要释放的错误信息
 */
void idcu_err_free_error(idcu_ErrorInfo* error);

/**
 * @brief 检查错误是否有前置错误（错误链）
 * @param error 错误信息
 * @return true 表示有前置错误
 */
bool idcu_err_has_cause(const idcu_ErrorInfo* error);

/**
 * @brief 获取错误链中的前置错误
 * @param error 错误信息
 * @return 前置错误指针
 */
const idcu_ErrorInfo* idcu_err_get_cause(const idcu_ErrorInfo* error);

/**
 * @brief 格式化错误信息为字符串
 * @param error 错误信息
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 写入的字符数
 */
int idcu_err_format_error(const idcu_ErrorInfo* error, char* buffer, size_t buffer_size);

/**
 * @brief 格式化完整错误链为字符串
 * @param error 错误信息
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 写入的字符数
 */
int idcu_err_format_error_chain(const idcu_ErrorInfo* error, char* buffer, size_t buffer_size);

#ifdef IDCU_DEBUG
void idcu_err_capture_stacktrace(idcu_ErrorInfo* error);
int idcu_err_format_stacktrace(const idcu_ErrorInfo* error, char* buffer, size_t buffer_size);
#endif

#define IDCU_ERR_SET(code, ctx) idcu_err_set_last_error((code), (ctx), __FILE__, __LINE__)
#define IDCU_ERR_SET_WITH_CAUSE(code, ctx, cause) \
    idcu_err_set_last_error_with_cause((code), (ctx), __FILE__, __LINE__, (cause))

#endif
