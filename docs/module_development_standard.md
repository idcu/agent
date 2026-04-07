# 模块开发规范

## 1. 概述

本文档定义了 IDCU Agent 模块开发的标准规范，包括模块生命周期管理、通信接口、异常处理、依赖管理和沙箱安全等核心内容。

## 2. 模块生命周期接口

### 2.1 生命周期状态

模块具有以下生命周期状态：

| 状态 | 描述 |
|------|------|
| UNINIT | 未初始化状态 |
| INITED | 初始化完成状态 |
| RUNNING | 运行中状态 |
| STOPPED | 已停止状态 |
| ERROR | 错误状态 |

### 2.2 生命周期回调函数

模块应实现以下生命周期回调函数：

```c
#include "module_lifecycle.h"

int on_pre_init(idcu_ModuleLifecycleContext* ctx) {
    // 预初始化：资源准备、配置读取
    return IDCU_ERR_OK;
}

int on_init(idcu_ModuleLifecycleContext* ctx) {
    // 初始化：核心资源分配、组件初始化
    return IDCU_ERR_OK;
}

int on_post_init(idcu_ModuleLifecycleContext* ctx) {
    // 后初始化：依赖注入、服务注册
    return IDCU_ERR_OK;
}

int on_pre_start(idcu_ModuleLifecycleContext* ctx) {
    // 预启动：启动前准备
    return IDCU_ERR_OK;
}

int on_start(idcu_ModuleLifecycleContext* ctx) {
    // 启动：开始主业务逻辑
    return IDCU_ERR_OK;
}

int on_post_start(idcu_ModuleLifecycleContext* ctx) {
    // 后启动：启动完成后的处理
    return IDCU_ERR_OK;
}

int on_pre_stop(idcu_ModuleLifecycleContext* ctx) {
    // 预停止：停止前准备
    return IDCU_ERR_OK;
}

int on_stop(idcu_ModuleLifecycleContext* ctx) {
    // 停止：停止主业务逻辑
    return IDCU_ERR_OK;
}

int on_post_stop(idcu_ModuleLifecycleContext* ctx) {
    // 后停止：停止后的清理
    return IDCU_ERR_OK;
}

void on_pre_destroy(idcu_ModuleLifecycleContext* ctx) {
    // 预销毁：销毁前准备
}

void on_destroy(idcu_ModuleLifecycleContext* ctx) {
    // 销毁：释放核心资源
}

void on_post_destroy(idcu_ModuleLifecycleContext* ctx) {
    // 后销毁：最终清理
}
```

## 3. 模块通信接口

### 3.1 消息类型

模块间通信支持以下消息类型：

| 类型 | 描述 |
|------|------|
| REQUEST | 请求消息，需要响应 |
| RESPONSE | 响应消息，对应请求 |
| NOTIFICATION | 通知消息，单向 |
| EVENT | 事件消息，广播 |

### 3.2 发送请求

```c
#include "module_communication.h"

void response_handler(idcu_ModuleCommContext* ctx,
                      const idcu_ModuleMessage* response,
                      int error_code,
                      void* user_data) {
    if (error_code == IDCU_ERR_OK) {
        // 处理响应
    }
}

uint64_t request_id;
int ret = idcu_module_comm_send_request(
    ctx,
    "target_module",
    "service/request",
    IDCU_MSG_PRIO_NORMAL,
    payload_data,
    payload_size,
    response_handler,
    user_data,
    &request_id
);
```

### 3.3 发送通知

```c
int ret = idcu_module_comm_send_notification(
    ctx,
    "target_module",
    "event/notify",
    IDCU_MSG_PRIO_NORMAL,
    data,
    data_size
);
```

### 3.4 广播事件

```c
int ret = idcu_module_comm_broadcast_event(
    ctx,
    "system/event",
    IDCU_MSG_PRIO_NORMAL,
    event_data,
    event_size
);
```

### 3.5 注册主题处理器

```c
void topic_handler(idcu_ModuleCommContext* ctx,
                   const idcu_ModuleMessage* msg,
                   void* user_data) {
    // 处理接收到的消息
}

int ret = idcu_module_comm_register_topic_handler(
    ctx,
    "service/request",
    topic_handler,
    user_data
);
```

## 4. 异常处理接口

### 4.1 错误码

模块系统定义了以下错误码：

| 错误码 | 描述 |
|--------|------|
| IDCU_MOD_ERR_INVALID_STATE | 无效的模块状态 |
| IDCU_MOD_ERR_DEPENDENCY_MISSING | 缺少依赖 |
| IDCU_MOD_ERR_DEPENDENCY_VERSION_MISMATCH | 依赖版本不匹配 |
| IDCU_MOD_ERR_DEPENDENCY_CYCLE | 检测到依赖循环 |
| IDCU_MOD_ERR_RESOURCE_EXHAUSTED | 资源耗尽 |
| IDCU_MOD_ERR_RESOURCE_LIMIT_EXCEEDED | 资源限制超出 |
| IDCU_MOD_ERR_INITIALIZATION_FAILED | 初始化失败 |
| IDCU_MOD_ERR_START_FAILED | 启动失败 |
| IDCU_MOD_ERR_STOP_FAILED | 停止失败 |
| IDCU_MOD_ERR_COMMUNICATION_FAILED | 通信失败 |
| IDCU_MOD_ERR_TIMEOUT | 超时 |
| IDCU_MOD_ERR_PERMISSION_DENIED | 权限拒绝 |
| IDCU_MOD_ERR_INVALID_CONFIG | 无效配置 |
| IDCU_MOD_ERR_MODULE_NOT_FOUND | 模块未找到 |
| IDCU_MOD_ERR_MODULE_ALREADY_LOADED | 模块已加载 |
| IDCU_MOD_ERR_SANDBOX_VIOLATION | 沙箱违规 |

### 4.2 错误处理

```c
#include "module_error.h"

void error_handler(idcu_ModuleErrorContext* ctx,
                   const idcu_ModuleError* error,
                   void* user_data) {
    // 处理错误
}

idcu_module_error_set_handler(ctx, error_handler, user_data);

// 抛出错误
IDCU_MOD_RAISE(ctx, IDCU_MOD_ERR_INITIALIZATION_FAILED, "Failed to initialize");
```

## 5. 模块依赖管理

### 5.1 声明依赖

模块可以在 `module.json` 或代码中声明依赖：

```json
{
  "name": "my_module",
  "version": "1.0.0",
  "dependencies": [
    {
      "name": "log",
      "version": ">=1.0.0",
      "optional": false
    },
    {
      "name": "config",
      "version": ">=2.0.0,<3.0.0",
      "optional": true
    }
  ]
}
```

### 5.2 兼容性模式

| 模式 | 描述 |
|------|------|
| EXACT | 精确版本匹配 |
| GTE | 大于等于指定版本 |
| LTE | 小于等于指定版本 |
| RANGE | 版本范围 |

### 5.3 解析依赖

```c
idcu_ModuleDependency dep;
int ret = idcu_module_dependency_parse_from_string("log@1.0.0", &dep);

idcu_ModuleDependencyNode* order[IDCU_MAX_MODULES_GRAPH];
int count;
ret = idcu_kernel_resolve_dependencies(kernel, "my_module", order, &count);
```

## 6. 模块沙箱与资源限制

### 6.1 资源类型

| 资源类型 | 描述 |
|----------|------|
| MEMORY | 内存限制（字节） |
| CPU | CPU 限制（百分比） |
| FILE_DESCRIPTORS | 文件描述符限制 |
| THREADS | 线程数限制 |
| NETWORK | 网络访问权限 |

### 6.2 设置资源限制

```c
#include "module_sandbox.h"

idcu_SandboxResourceLimits limits = {
    .memory_limit_bytes = 100 * 1024 * 1024,  // 100 MB
    .cpu_limit_percent = 50,
    .fd_limit = 256,
    .thread_limit = 16,
    .network_allowed = true
};

idcu_kernel_set_module_sandbox_limits(kernel, "my_module", &limits);
```

### 6.3 资源追踪

```c
// 分配内存
idcu_module_sandbox_track_memory_alloc(sandbox, size);

// 释放内存
idcu_module_sandbox_track_memory_free(sandbox, size);

// 分配文件描述符
idcu_module_sandbox_track_fd_alloc(sandbox);

// 释放文件描述符
idcu_module_sandbox_track_fd_free(sandbox);

// 创建线程
idcu_module_sandbox_track_thread_create(sandbox);

// 销毁线程
idcu_module_sandbox_track_thread_destroy(sandbox);
```

## 7. 资源释放规范

### 7.1 内存管理

- 所有动态分配的内存必须在 `on_destroy` 中释放
- 使用配对的分配/释放函数（malloc/free, new/delete）
- 避免内存泄漏，使用工具检测

### 7.2 文件描述符

- 所有打开的文件描述符必须关闭
- 使用 RAII 模式或确保在错误路径中也能关闭
- 监控文件描述符数量

### 7.3 线程管理

- 所有创建的线程必须正确 join 或 detach
- 使用线程池避免创建过多线程
- 确保线程安全访问共享资源

### 7.4 网络资源

- 所有 socket 必须正确关闭
- 释放连接池中的连接
- 取消挂起的网络请求

## 8. 模块版本规范

### 8.1 版本号格式

使用语义化版本号（Semantic Versioning）：

```
MAJOR.MINOR.PATCH[-PRE_RELEASE]
```

- **MAJOR**：不兼容的 API 变更
- **MINOR**：向后兼容的功能新增
- **PATCH**：向后兼容的 bug 修复
- **PRE_RELEASE**：预发布版本标识（可选）

### 8.2 版本兼容性

- 主版本号不同表示不兼容
- 次版本号不同表示新增功能但兼容
- 修订号不同表示 bug 修复
- 预发布版本可能不稳定

## 9. 最佳实践

### 9.1 编码规范

- 遵循项目的编码风格
- 使用有意义的变量和函数名
- 添加必要的注释
- 保持函数简短和单一职责

### 9.2 错误处理

- 总是检查返回值
- 提供有意义的错误信息
- 使用错误码而不是异常
- 实现 graceful degradation

### 9.3 性能考虑

- 避免不必要的复制
- 使用高效的数据结构
- 考虑内存使用和 CPU 消耗
- 使用异步操作避免阻塞

### 9.4 测试

- 编写单元测试
- 测试边界条件
- 测试错误路径
- 进行集成测试

## 10. 附录

### 10.1 相关文件

- `modules/core/sdk/include/module_lifecycle.h` - 生命周期接口
- `modules/core/sdk/include/module_communication.h` - 通信接口
- `modules/core/sdk/include/module_error.h` - 异常处理
- `modules/core/sdk/include/module_sandbox.h` - 沙箱接口
- `modules/core/module-system/include/module_dependency.h` - 依赖管理

### 10.2 示例模块

参考示例模块：
- `modules/business/examples/simple-example/`
- `examples/basic/`

### 10.3 联系方式

如有问题，请参考：
- 项目文档：`docs/`
- 社区指南：`docs/community_guide.md`
- 开发指南：`docs/developer_guide.md`
