# IDCU C SDK 使用指南

## 概述

IDCU C SDK 是一个简化模块开发的工具库，提供了简洁的 API 来创建和管理 IDCU 代理模块。

## 版本信息

- SDK 版本: 1.0.0
- 兼容性: IDCU Agent v1.0+

## 快速开始

### 1. 引入头文件

```c
#include "sdk/sdk.h"
```

### 2. 定义模块生命周期函数

```c
static int my_module_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing my module");
    return IDCU_ERR_OK;
}

static int my_module_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting my module");
    return IDCU_ERR_OK;
}

static int my_module_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping my module");
    return IDCU_ERR_OK;
}

static void my_module_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying my module");
}
```

### 3. 注册模块

```c
IDCU_SDK_MODULE_DEFINE(
    my_module,
    "1.0.0",
    "My module description",
    my_module_init,
    my_module_start,
    my_module_stop,
    my_module_destroy
);
```

## 核心 API

### 上下文管理

#### idcu_sdk_create_context()
创建 SDK 上下文。

**返回值:**
- 成功: 指向 `idcu_SdkContext` 的指针
- 失败: NULL

#### idcu_sdk_destroy_context(ctx)
销毁 SDK 上下文。

**参数:**
- `ctx`: SDK 上下文指针

### 用户数据

#### idcu_sdk_set_user_data(ctx, user_data)
设置用户自定义数据。

**参数:**
- `ctx`: SDK 上下文指针
- `user_data`: 用户数据指针

#### idcu_sdk_get_user_data(ctx)
获取用户自定义数据。

**参数:**
- `ctx`: SDK 上下文指针

**返回值:**
- 用户数据指针

### 模块信息

#### idcu_sdk_get_module_id(ctx)
获取当前模块 ID。

**参数:**
- `ctx`: SDK 上下文指针

**返回值:**
- 模块 ID (uint32_t)

#### idcu_sdk_get_module_name(ctx)
获取当前模块名称。

**参数:**
- `ctx`: SDK 上下文指针

**返回值:**
- 模块名称字符串

### 消息传递

#### idcu_sdk_send_message(ctx, target_module, priority, data, size)
向指定模块发送消息。

**参数:**
- `ctx`: SDK 上下文指针
- `target_module`: 目标模块名称
- `priority`: 消息优先级
- `data`: 消息数据指针
- `size`: 消息数据大小

**返回值:**
- 成功: IDCU_ERR_OK
- 失败: 错误码

#### idcu_sdk_broadcast_message(ctx, priority, data, size)
向所有模块广播消息。

**参数:**
- `ctx`: SDK 上下文指针
- `priority`: 消息优先级
- `data`: 消息数据指针
- `size`: 消息数据大小

**返回值:**
- 成功: IDCU_ERR_OK
- 失败: 错误码

#### idcu_sdk_register_message_handler(ctx, handler, user_data)
注册消息处理函数。

**参数:**
- `ctx`: SDK 上下文指针
- `handler`: 消息处理函数
- `user_data`: 传递给处理函数的用户数据

**返回值:**
- 成功: IDCU_ERR_OK
- 失败: 错误码

### 日志

#### idcu_sdk_log(ctx, level, fmt, ...)
输出日志。

**参数:**
- `ctx`: SDK 上下文指针
- `level`: 日志级别
- `fmt`: 格式化字符串
- `...`: 可变参数

**快捷宏:**
- `idcu_sdk_log_debug(ctx, fmt, ...)`
- `idcu_sdk_log_info(ctx, fmt, ...)`
- `idcu_sdk_log_warn(ctx, fmt, ...)`
- `idcu_sdk_log_error(ctx, fmt, ...)`
- `idcu_sdk_log_fatal(ctx, fmt, ...)`

## 示例代码

### 简单示例

参见 `modules/sdk_examples/simple_example.c`

### 消息传递示例

参见 `modules/sdk_examples/messaging_example.c`

## 编译

将 SDK 源文件添加到你的构建系统中：

- 头文件: `include/sdk/sdk.h`
- 源文件: `src/sdk/sdk.c`

## 最佳实践

1. **始终在 init 中分配资源，在 destroy 中释放**
2. **使用 idcu_sdk_set_user_data 存储模块状态**
3. **合理使用日志级别**
4. **处理所有可能的错误返回值**

## 许可证

本 SDK 遵循 IDCU Agent 项目的许可证。
