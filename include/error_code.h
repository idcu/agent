#ifndef ERROR_CODE_H
#define ERROR_CODE_H

// 基础错误码
typedef enum {
    ERR_OK             = 0,    // 成功（别名）
    ERR_SUCCESS        = 0,    // 成功
    ERR_GENERAL        = -1,   // 通用错误
    ERR_INVALID_PARAM  = -2,   // 参数无效
    ERR_INVALID_ARG    = -2,   // 参数无效（别名）
    ERR_NO_MEMORY      = -3,   // 内存不足（主要名称）
    ERR_OUT_OF_MEM     = -3,   // 内存不足（别名）
    ERR_PERM_DENIED    = -4,   // 权限拒绝
    ERR_QUEUE_FULL     = -5,   // 队列满
    ERR_QUEUE_EMPTY    = -6,   // 队列空
    ERR_MODULE_NOT_FOUND = -7, // 模块未找到
    ERR_MODULE_INIT    = -8,   // 模块初始化失败
    ERR_CONFIG_LOAD    = -9,   // 配置加载失败
    ERR_MSG_SEND       = -10,  // 消息发送失败
    ERR_MSG_RECV       = -11,  // 消息接收失败
    ERR_CORO_CREATE    = -12,  // 协程创建失败
    ERR_SANDBOX_INIT   = -13,  // 沙箱初始化失败
    ERR_MODULE_LOAD    = -14,  // 模块加载失败
    ERR_MODULE_INVALID = -15,  // 模块无效
    ERR_NOT_FOUND      = -16,  // 未找到
    ERR_ALREADY_EXISTS = -17,  // 已存在
    ERR_BUSY           = -18,  // 忙
    ERR_MODULE_RUN     = -19,  // 模块运行失败
    ERR_MODULE_STOP    = -20,  // 模块停止失败
    ERR_MODULE_CRASH   = -21,  // 模块崩溃
    ERR_DEPENDENCY     = -22,  // 依赖错误
    ERR_CIRCULAR_DEP   = -23,  // 循环依赖
    ERR_VERSION_MISMATCH = -24, // 版本不匹配
    ERR_NOT_INITIALIZED = -25, // 未初始化
    ERR_LOCK_FAILED    = -26,  // 加锁失败
    ERR_UNLOCK_FAILED  = -27,  // 解锁失败
    ERR_MSG_TIMEOUT    = -28,  // 消息超时
    ERR_MSG_DROPPED    = -29,  // 消息丢弃
    ERR_HEALTH_CHECK_FAIL = -30, // 健康检查失败
    ERR_CONFIG_PARSE   = -31   // 配置解析错误
} ErrorCode;

// 错误信息转换
const char* err_to_str(int err_code);

#endif // ERROR_CODE_H