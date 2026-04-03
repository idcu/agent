#ifndef IDCU_COMMON_ERROR_CODE_H
#define IDCU_COMMON_ERROR_CODE_H

// 基础错误码
typedef enum {
    IDCU_ERR_OK             = 0,    // 成功（别名）
    IDCU_ERR_SUCCESS        = 0,    // 成功
    IDCU_ERR_GENERAL        = -1,   // 通用错误
    IDCU_ERR_INVALID_PARAM  = -2,   // 参数无效
    IDCU_ERR_INVALID_ARG    = -2,   // 参数无效（别名）
    IDCU_ERR_NO_MEMORY      = -3,   // 内存不足（主要名称）
    IDCU_ERR_OUT_OF_MEM     = -3,   // 内存不足（别名）
    IDCU_ERR_PERM_DENIED    = -4,   // 权限拒绝
    IDCU_ERR_QUEUE_FULL     = -5,   // 队列满
    IDCU_ERR_QUEUE_EMPTY    = -6,   // 队列空
    IDCU_ERR_MODULE_NOT_FOUND = -7, // 模块未找到
    IDCU_ERR_MODULE_INIT    = -8,   // 模块初始化失败
    IDCU_ERR_CONFIG_LOAD    = -9,   // 配置加载失败
    IDCU_ERR_MSG_SEND       = -10,  // 消息发送失败
    IDCU_ERR_MSG_RECV       = -11,  // 消息接收失败
    IDCU_ERR_CORO_CREATE    = -12,  // 协程创建失败
    IDCU_ERR_SANDBOX_INIT   = -13,  // 沙箱初始化失败
    IDCU_ERR_MODULE_LOAD    = -14,  // 模块加载失败
    IDCU_ERR_MODULE_INVALID = -15,  // 模块无效
    IDCU_ERR_NOT_FOUND      = -16,  // 未找到
    IDCU_ERR_ALREADY_EXISTS = -17,  // 已存在
    IDCU_ERR_BUSY           = -18,  // 忙
    IDCU_ERR_MODULE_RUN     = -19,  // 模块运行失败
    IDCU_ERR_MODULE_STOP    = -20,  // 模块停止失败
    IDCU_ERR_MODULE_CRASH   = -21,  // 模块崩溃
    IDCU_ERR_DEPENDENCY     = -22,  // 依赖错误
    IDCU_ERR_CIRCULAR_DEP   = -23,  // 循环依赖
    IDCU_ERR_VERSION_MISMATCH = -24, // 版本不匹配
    IDCU_ERR_NOT_INITIALIZED = -25, // 未初始化
    IDCU_ERR_LOCK_FAILED    = -26,  // 加锁失败
    IDCU_ERR_UNLOCK_FAILED  = -27,  // 解锁失败
    IDCU_ERR_MSG_TIMEOUT    = -28,  // 消息超时
    IDCU_ERR_MSG_DROPPED    = -29,  // 消息丢弃
    IDCU_ERR_HEALTH_CHECK_FAIL = -30, // 健康检查失败
    IDCU_ERR_CONFIG_PARSE   = -31   // 配置解析错误
} idcu_ErrorCode;

// 错误信息转换
const char* idcu_err_to_str(int err_code);

#endif // IDCU_COMMON_ERROR_CODE_H