#ifndef ERROR_CODE_H
#define ERROR_CODE_H

// 基础错误码
typedef enum {
    ERR_SUCCESS        = 0,    // 成功
    ERR_GENERAL        = -1,   // 通用错误
    ERR_INVALID_PARAM  = -2,   // 参数无效
    ERR_OUT_OF_MEM     = -3,   // 内存不足
    ERR_PERM_DENIED    = -4,   // 权限拒绝
    ERR_QUEUE_FULL     = -5,   // 队列满
    ERR_QUEUE_EMPTY    = -6,   // 队列空
    ERR_MODULE_NOT_FOUND = -7, // 模块未找到
    ERR_MODULE_INIT    = -8,   // 模块初始化失败
    ERR_CONFIG_LOAD    = -9,   // 配置加载失败
    ERR_MSG_SEND       = -10,  // 消息发送失败
    ERR_MSG_RECV       = -11,  // 消息接收失败
    ERR_CORO_CREATE    = -12,  // 协程创建失败
    ERR_SANDBOX_INIT   = -13   // 沙箱初始化失败
} ErrorCode;

// 错误信息转换
const char* err_to_str(int err_code);

#endif // ERROR_CODE_H