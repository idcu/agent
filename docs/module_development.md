# 模块开发手册 (Module Development Guide)

本手册提供了 IDCU Agent 模块开发的全面信息，包括模块编写规范、微内核接口调用、沙箱和消息总线使用方法、以及热加载流程。

---

## 目录
1. [模块生命周期](#模块生命周期)
2. [模块编写规范](#模块编写规范)
3. [微内核接口](#微内核接口)
4. [消息总线使用](#消息总线使用)
5. [沙箱安全](#沙箱安全)
6. [热加载流程](#热加载流程)
7. [最佳实践](#最佳实践)

---

## 模块生命周期

每个 IDCU Agent 模块都有三个关键生命周期阶段：

### 1. 初始化（`init()`）
模块加载时调用一次，用于：
- 分配资源
- 初始化数据结构
- 读取配置
- 注册消息订阅

```c
#include "module_def.h"
#include "log.h"

static int my_module_init(void)
{
    idcu_log_info("[my_module] 正在初始化...");
    
    // 在这里初始化资源
    return 0; // 返回 0 表示成功
}
```

### 2. 运行（`run()`）
在内核主循环中反复调用，注意：
- **不要使用无限循环！** 这会阻塞整个系统
- 快速返回
- 使用状态机处理复杂逻辑
- 使用时间检查来执行周期性任务

```c
static int g_counter = 0;

static int my_module_run(void)
{
    // 好的做法：非阻塞，快速返回
    if (++g_counter % 1000000 == 0) {
        idcu_log_info("[my_module] 正在运行...");
    }
    return 0;
}
```

### 3. 停止（`stop()`）
模块卸载时调用一次，用于：
- 释放资源
- 保存状态
- 清理

```c
static int my_module_stop(void)
{
    idcu_log_info("[my_module] 正在停止...");
    // 在这里清理资源
    return 0;
}
```

---

## 模块编写规范

### 模块注册宏
使用 `IDCU_REGISTER_MODULE` 宏注册你的模块：

```c
IDCU_REGISTER_MODULE(
    module_name,              // 模块名（唯一标识）
    IDCU_MODULE_VERSION(1, 0, 0),  // 版本（主版本、次版本、补丁版本）
    my_module_init,           // 初始化函数
    my_module_run,            // 运行函数
    my_module_stop            // 停止函数
);
```

### 模块结构
典型的模块目录结构：
```
modules/business/my_module/
├── CMakeLists.txt
├── README.md
├── include/
│   └── my_module/
│       └── my_module.h
└── src/
    └── my_module.c
```

### CMakeLists.txt 示例
```cmake
add_library(idcu_business_my_module STATIC
    src/my_module.c
)

target_include_directories(idcu_business_my_module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modules/core/module-system/include
    ${CMAKE_SOURCE_DIR}/libs/idcu-log/include
    ${CMAKE_SOURCE_DIR}/libs/idcu-config/include
)

target_link_libraries(idcu_business_my_module PRIVATE
    idcu::log
    idcu::config
)
```

---

## 微内核接口

### SDK 函数
SDK 提供了对微内核功能的简化访问：

```c
#include "sdk.h"

// 向另一个模块发送消息
idcu_sdk_send_message("target_module", message_data, data_size);

// 接收消息
idcu_sdk_recv_message(&message);

// 记录日志
idcu_sdk_log_info("消息: %s", text);

// 读取配置
int value = idcu_sdk_config_get_int("my_module", "key", default_value);
```

---

## 消息总线使用

### 发送消息
```c
#include "msg_bus.h"

// 创建并发送消息
idcu_Message msg;
msg.type = IDCU_MSG_TYPE_DATA;
msg.sender_id = my_module_id;
msg.recipient_id = target_module_id;
msg.data_len = sizeof(my_data);
memcpy(msg.data, &my_data, sizeof(my_data));

idcu_msg_bus_send(&msg);
```

### 接收消息
```c
#include "msg_bus.h"

idcu_Message msg;
if (idcu_msg_bus_recv(&msg) == 0) {
    // 处理消息
    process_message(&msg);
}
```

### 消息类型
- `IDCU_MSG_TYPE_DATA` - 通用数据消息
- `IDCU_MSG_TYPE_COMMAND` - 命令消息
- `IDCU_MSG_TYPE_RESPONSE` - 响应消息
- `IDCU_MSG_TYPE_EVENT` - 事件通知

---

## 沙箱安全

### 权限配置
在 `agent.cfg` 中配置模块权限：
```ini
[permissions]
my_module = send, recv, log, config
```

可用权限：
- `send` - 发送消息
- `recv` - 接收消息
- `run` - 执行模块
- `log` - 写日志
- `config` - 读配置
- `module_mgr` - 管理模块
- `hw` - 硬件访问

### 沙箱 API
```c
#include "sandbox.h"

// 检查模块是否有权限
if (idcu_sandbox_check_permission(module_id, "send")) {
    // 有权限
}

// 设置资源限制
idcu_sandbox_set_quota(module_id, IDCU_QUOTA_MEMORY, 1048576);
```

---

## 热加载流程

### 动态模块
IDCU Agent 支持运行时加载/卸载模块（DLL/SO）：

#### 创建动态模块
1. 遵循标准模块结构
2. 在 CMakeLists.txt 中使用 `SHARED` 代替 `STATIC`：
   ```cmake
   add_library(dynamic_sample SHARED src/sample_module.c)
   ```

#### 运行时加载
```c
// 热加载模块
idcu_kernel_hotplug_load("path/to/dynamic_module.dll");

// 热卸载模块
idcu_kernel_hotplug_unload("dynamic_module");
```

#### 热加载模块的配置
```ini
[modules]
dynamic = dynamic_sample

enable_dynamic_sample = true
```

---

## 最佳实践

### 1. 资源管理
始终配对分配和释放：
```c
// 好的做法
static char* g_buffer = NULL;

static int init(void) {
    g_buffer = malloc(1024);
    return g_buffer ? 0 : -1;
}

static int stop(void) {
    if (g_buffer) {
        free(g_buffer);
        g_buffer = NULL;
    }
    return 0;
}
```

### 2. 错误处理
检查返回值并优雅地处理错误：
```c
FILE* fp = fopen("data.txt", "r");
if (!fp) {
    idcu_log_error("[my_module] 打开文件失败");
    return -1;
}
// 使用文件...
fclose(fp);
```

### 3. 配置
使用配置系统而不是硬编码值：
```c
// 从配置读取，带默认值
int interval = idcu_config_get_int("my_module", "interval_ms", 1000);
```

### 4. 日志
使用适当的日志级别：
```c
idcu_log_debug("[my_module] 详细调试信息");
idcu_log_info("[my_module] 正常操作");
idcu_log_warn("[my_module] 警告：内存不足");
idcu_log_error("[my_module] 错误：操作失败");
```

### 5. 线程安全
微内核是单线程的，所以模块数据不需要锁。但是，如果你手动创建线程，则必须使用适当的同步机制。

---

## 下一步
- 查看 [架构设计文档](./architecture_en.md) 了解整体系统
- 探索 `modules/business/` 中的现有模块作为示例
- 阅读 [API 参考文档](./api/index.md) 获取详细的接口文档
