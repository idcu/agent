# IDCU Agent 开发者指南

欢迎来到 IDCU Agent 开发者世界！这篇文档会手把手教你如何修改代码、添加新功能。不管你是 C 语言新手还是老手，都能看懂！

---

## 目录

1. [架构介绍](#架构介绍)
2. [项目结构详解](#项目结构详解)
3. [核心模块说明](#核心模块说明)
4. [如何添加新模块](#如何添加新模块)
5. [前缀命名空间](#前缀命名空间)
6. [API 参考](#api-参考)
7. [编译调试技巧](#编译调试技巧)
8. [代码规范](#代码规范)
9. [测试指南](#测试指南)

---

## 架构介绍

在开始写代码之前，让我们先理解一下这个系统是怎么设计的。

### 什么是微内核架构？

简单说，微内核就像一个**万能插座板**：
- **微内核** = 插座板本身，很小，只提供最基本的功能
- **模块** = 各种电器（台灯、电视、空调等），插上去就能用

这种设计的好处是：
- 核心代码很小，容易维护
- 想加新功能，写个模块就行，不用改核心
- 模块之间互不影响，出问题容易定位

### IDCU Agent 的架构图

```
┌─────────────────────────────────────────────────────┐
│              应用层 (Application)                    │
├─────────────────────────────────────────────────────┤
│              模块层 (Modules)                        │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐              │
│  │ log     │ │collect  │ │  alert  │              │
│  └─────────┘ └─────────┘ └─────────┘              │
├─────────────────────────────────────────────────────┤
│           微内核层 (Micro Kernel)                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ 微内核 (Micro Kernel) - 核心管理              │  │
│  ├──────────────────────────────────────────────┤  │
│  │ 调度器 (协程 + 消息总线)                      │  │
│  ├──────────────────────────────────────────────┤  │
│  │ 模块系统 (注册 + 加载 + 版本管理)             │  │
│  ├──────────────────────────────────────────────┤  │
│  │ 工具组件 + 通用基础                           │  │
│  ├──────────────────────────────────────────────┤  │
│  │ SDK + 测试框架                                │  │
│  └──────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
```

### 核心组件列表

| 组件 | 功能 | 头文件位置 |
|------|------|-----------|
| 微内核 | 系统核心管理 | `modules/core/micro-kernel/include/micro_kernel.h` |
| 协程调度器 | 管理任务执行 | `modules/core/scheduler/include/coroutine.h` |
| 消息总线 | 模块之间的消息传递 | `modules/core/scheduler/include/msg_bus.h` |
| 模块管理系统 | 模块加载、注册、版本管理 | `modules/core/module-system/include/` |
| 工具组件 | 日志、配置、JSON、内存池 | `modules/core/utils/include/` |
| 通用基础 | 原子操作、锁、错误码 | `modules/core/common/include/` |
| SDK | 简化模块开发接口 | `modules/core/sdk/include/sdk.h` |
| 测试框架 | 单元测试支持 | `modules/core/test-framework/include/test_framework.h` |
| 监控系统 | 健康检查、指标收集 | `modules/services/monitor/include/` |
| 网络系统 | 网络通信、分布式节点 | `modules/services/network/include/` |
| 沙箱 | 模块隔离和安全 | `modules/services/security/include/sandbox.h` |

---

## 项目结构详解

让我们深入了解一下每个目录是做什么的。

```
idcu-agent/
├── app/                          # 应用程序入口
│   ├── main.c                    # 主程序入口（看这个可以了解启动流程）
│   └── benchmark.c               # 性能基准测试
├── config/                       # 配置文件
│   └── agent.cfg                 # 主配置文件
├── modules/                      # 模块化结构（最重要的目录！）
│   ├── core/                     # 核心基础设施
│   │   ├── common/               # 通用基础组件
│   │   ├── utils/                # 通用工具组件
│   │   ├── module-system/        # 模块管理系统
│   │   ├── scheduler/            # 调度器模块
│   │   ├── micro-kernel/         # 微内核核心
│   │   ├── sdk/                  # 软件开发工具包
│   │   └── test-framework/       # 测试框架
│   ├── services/                 # 服务模块组（提供基础服务）
│   │   ├── monitor/              # 监控系统
│   │   ├── network/              # 网络系统
│   │   ├── storage/              # 存储服务
│   │   ├── cache/                # 缓存服务
│   │   ├── security/             # 安全系统
│   │   └── plugin/               # 插件系统
│   └── business/                 # 业务模块组（具体功能在这里！）
│       ├── core-module/          # 核心基础模块
│       ├── log/                  # 日志模块
│       ├── alert/                # 告警模块
│       ├── collect/              # 采集模块
│       ├── config/               # 配置模块
│       ├── examples/             # 示例模块（看这里学习！）
│       │   ├── simple-example/   # 简单示例
│       │   ├── messaging-example/ # 消息示例
│       │   └── advanced-example/ # 高级示例
│       └── dynamic/              # 动态加载模块示例
├── module-repo/                  # 模块仓库
├── tests/                        # 测试用例
│   ├── unit/                     # 单元测试
│   └── integration/              # 集成测试
├── examples/                     # 示例程序
├── docs/                         # 文档目录
├── scripts/                      # 构建脚本
└── CMakeLists.txt                # CMake 构建配置
```

### 关键文件说明

#### core/ 目录 - 核心基础设施

这是整个系统的基石，建议先看懂这些代码：

**common/ - 通用基础组件**
- `atomic.h/c` - 原子操作（多线程安全的变量操作）
- `lock.h/c` - 锁机制（互斥锁、读写锁、条件变量）
- `error_code.h/c` - 错误码定义和处理
- `config.h` - 编译时配置

**utils/ - 通用工具组件**
- `log.h/c` - 日志系统（多级别日志、格式化输出）
- `config_manager.h/c` - 配置管理（读写配置文件）
- `json_parser.h/c` - JSON 解析器
- `memory_pool.h/c` - 内存池（高效内存分配）
- `permission_manager.h/c` - 权限管理

**module-system/ - 模块管理系统**
- `module_def.h` - 模块接口定义（重点看！）
- `module_registry.h/c` - 模块注册和管理
- `module_category.h/c` - 模块分类
- `dynamic_module.h/c` - 动态模块加载
- `module_version.h/c` - 版本管理

**scheduler/ - 调度器模块**
- `coroutine.h/c` - 协程调度（轻量级线程）
- `msg_bus.h/c` - 消息总线（模块间通信）
- `context.h/c` - 上下文管理

**micro-kernel/ - 微内核核心**
- `micro_kernel.h/c` - 微内核主接口（系统的心脏！）

**sdk/ - 软件开发工具包**
- `sdk.h/c` - 简化模块开发的接口

**test-framework/ - 测试框架**
- `test_framework.h/c` - 单元测试框架

#### services/ 目录 - 核心服务实现

这些是可复用的服务组件：

**monitor/ - 监控系统**
- `health_check.h/c` - 健康检查
- `metrics.h/c` - 指标收集
- `alert_manager.h/c` - 告警管理
- `notifier.h/c` - 通知器
- `prometheus_exporter.h/c` - Prometheus 指标导出

**network/ - 网络系统**
- `network_layer.h/c` - 网络层（TCP/UDP Socket）
- `connection_pool.h/c` - 连接池
- `http_server.h/c` - HTTP 服务器
- `management_api.h/c` - 管理 API
- `distributed_node.h/c` - 分布式节点
- `node_discovery.h/c` - 节点发现

**security/ - 安全系统**
- `sandbox.h/c` - 沙箱基础版
- `sandbox_enhanced.h/c` - 沙箱增强版

#### business/ 目录 - 业务模块

这里是具体的功能模块，也是你最可能添加代码的地方：

- `core-module/` - 核心基础模块
- `log/` - 日志模块
- `alert/` - 告警模块
- `collect/` - 采集模块
- `config/` - 配置模块
- `examples/` - 示例模块（建议先看这些！）

---

## 核心模块说明

让我们深入了解一些核心模块的工作原理。

### 1. 通用基础组件 (common)

**文件位置**: `modules/core/common/`

#### 原子操作 (atomic.h/c)

原子操作是指不会被线程调度打断的操作。比如两个线程同时给一个变量加 1，如果不用原子操作，结果可能不对。

**常用函数：**
```c
idcu_atomic_init(&var, 0);           // 初始化原子变量
idcu_atomic_inc(&var);                // 原子加 1
idcu_atomic_dec(&var);                // 原子减 1
int val = idcu_atomic_get(&var);      // 获取值
```

#### 锁机制 (lock.h/c)

锁用于保护共享资源，防止多个线程同时访问导致数据不一致。

**互斥锁示例：**
```c
idcu_Mutex mutex;
idcu_mutex_init(&mutex);

idcu_mutex_lock(&mutex);
// 在这里访问共享资源
idcu_mutex_unlock(&mutex);

idcu_mutex_destroy(&mutex);
```

#### 错误码 (error_code.h/c)

项目使用统一的错误码体系：

```c
typedef enum {
    IDCU_ERR_OK           =  0,  // 成功
    IDCU_ERR_FAIL         = -1,  // 通用失败
    IDCU_ERR_NO_MEMORY    = -2,  // 内存不足
    IDCU_ERR_INVALID_ARG  = -3,  // 参数无效
    IDCU_ERR_TIMEOUT      = -4,  // 超时
    // ... 更多错误码
} idcu_ErrorCode;
```

### 2. 工具组件 (utils)

**文件位置**: `modules/core/utils/`

#### 日志系统 (log.h/c)

日志系统支持多个级别：

```c
// 初始化日志
idcu_log_init("agent.log", IDCU_LOG_INFO);

// 输出日志
idcu_log_debug("Debug message: %d", 123);
idcu_log_info("Info message");
idcu_log_warn("Warning message");
idcu_log_error("Error message");

// 关闭日志
idcu_log_shutdown();
```

#### 配置管理 (config_manager.h/c)

读取和管理配置文件：

```c
idcu_ConfigManager* cfg = idcu_config_create();
idcu_config_load(cfg, "config/agent.cfg");

// 读取配置
const char* log_level = idcu_config_get(cfg, "general", "log_level");
int max_modules = idcu_config_get_int(cfg, "general", "max_modules", 32);

idcu_config_destroy(cfg);
```

### 3. 模块管理系统 (module-system)

**文件位置**: `modules/core/module-system/`

这是最重要的部分之一！让我们详细了解。

#### 模块定义 (module_def.h)

每个模块都需要实现这个接口：

```c
typedef struct {
    const char*         name;           // 模块名
    idcu_ModuleVersion  version;        // 模块版本
    const idcu_ModuleDependency* dependencies; // 依赖的模块
    int                 dependency_count; // 依赖数量
    int (*init)(void);                  // 初始化函数
    int (*run)(void);                   // 运行函数（会被反复调用）
    int (*stop)(void);                  // 停止函数
} idcu_ModuleInterface;
```

**模块生命周期：**
1. `init()` - 模块加载时调用一次，做初始化工作
2. `run()` - 内核主循环中反复调用，执行主要逻辑
3. `stop()` - 模块卸载时调用，做清理工作

**注册模块的宏：**
```c
// 最简单的方式
IDCU_REGISTER_MODULE(
    my_module,                    // 模块名
    IDCU_MODULE_VERSION(1, 0, 0), // 版本
    my_module_init,               // 初始化函数
    my_module_run,                // 运行函数
    my_module_stop                // 停止函数
);
```

### 4. 调度器模块 (scheduler)

**文件位置**: `modules/core/scheduler/`

#### 协程 (coroutine.h/c)

协程是轻量级的线程，可以在用户态调度，开销很小。

```c
idcu_CoroScheduler* scheduler = idcu_coro_create();

// 创建协程
idcu_coro_spawn(scheduler, my_coroutine_func, arg, IDCU_CORO_PRIO_NORMAL);

// 运行调度器
idcu_coro_run(scheduler);

idcu_coro_destroy(scheduler);
```

#### 消息总线 (msg_bus.h/c)

消息总线用于模块之间的通信：

```c
idcu_MessageBus* bus = idcu_msg_bus_create();

// 发送消息
idcu_Message msg;
msg.type = IDCU_MSG_TYPE_DATA;
msg.sender_id = my_module_id;
msg.receiver_id = target_module_id;
msg.data = my_data;
msg.data_size = sizeof(my_data);

idcu_msg_bus_send(bus, &msg);

// 接收消息
idcu_Message recv_msg;
idcu_msg_bus_recv(bus, my_module_id, &recv_msg);

idcu_msg_bus_destroy(bus);
```

### 5. 微内核核心 (micro-kernel)

**文件位置**: `modules/core/micro-kernel/`

这是系统最核心的部分，看看 `main.c` 你就懂了：

```c
int main(void)
{
    printf("idcu/agent hard real-time microkernel start...\n");
    
    // 1. 初始化日志
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    // 2. 初始化微内核
    idcu_kernel_init(&g_kernel);
    
    // 3. 设置信号处理器（处理 Ctrl+C）
    idcu_kernel_set_signal_handler(&g_kernel);
    
    // 4. 启动所有模块
    idcu_kernel_start_modules(&g_kernel);
    
    // 5. 运行主循环
    idcu_kernel_run(&g_kernel);
    
    // 6. 清理
    idcu_log_shutdown();
    printf("idcu/agent hard real-time microkernel stopped.\n");
    return 0;
}
```

**微内核主要接口：**
- `idcu_kernel_init()` - 初始化微内核
- `idcu_kernel_start_modules()` - 启动所有模块
- `idcu_kernel_run()` - 运行主循环
- `idcu_kernel_stop()` - 停止所有模块
- `idcu_kernel_hotplug_load()` - 热加载模块
- `idcu_kernel_hotplug_unload()` - 热卸载模块

---

## 如何添加新模块

这是最重要的部分！跟着下面的步骤，你就能添加自己的模块了。

### 第一步：创建模块目录

假设你要创建一个叫 `hello` 的模块：

1. 在 `modules/business/` 目录下创建 `hello/` 目录
2. 在 `hello/` 目录下创建 `src/` 子目录
3. 完整的结构应该是：
   ```
   modules/business/hello/
   ├── src/
   │   └── hello_module.c
   ├── CMakeLists.txt
   ├── README.md
   └── module.json
   ```

### 第二步：写模块代码

在 `modules/business/hello/src/hello_module.c` 中写代码。

#### 方式一：使用基础接口（简单）

```c
#include "module_def.h"
#include "log.h"
#include <stdio.h>

static int g_hello_counter = 0;

// 初始化函数 - 模块加载时调用一次
static int hello_init(void)
{
    idcu_log_info("[hello] Initializing hello module");
    g_hello_counter = 0;
    printf("[hello] Module initialized!\n");
    return IDCU_ERR_OK;
}

// 运行函数 - 会被内核反复调用
static int hello_run(void)
{
    // 每隔一段时间打印一次（避免刷屏）
    if (g_hello_counter % 1000000 == 0) {
        printf("[hello] I'm running! Count: %d\n", 
               g_hello_counter / 1000000);
    }
    g_hello_counter++;
    return IDCU_ERR_OK;
}

// 停止函数 - 模块卸载时调用
static int hello_stop(void)
{
    printf("[hello] Module stopped!\n");
    return IDCU_ERR_OK;
}

// 注册模块！这一行很重要！
IDCU_REGISTER_MODULE(
    hello,
    IDCU_MODULE_VERSION(1, 0, 0),
    hello_init,
    hello_run,
    hello_stop
);
```

#### 方式二：使用 SDK（推荐，更简单）

SDK 封装了很多常用功能，让模块开发更简单：

```c
#include "sdk.h"
#include <stdio.h>
#include <stdlib.h>

// 模块私有数据
typedef struct {
    int counter;
} HelloModuleData;

// 初始化
static int hello_init(idcu_SdkContext* ctx)
{
    idcu_sdk_log_info(ctx, "Initializing hello module");
    
    HelloModuleData* data = (HelloModuleData*)malloc(sizeof(HelloModuleData));
    if (!data) {
        idcu_sdk_log_error(ctx, "Failed to allocate memory");
        return IDCU_ERR_NO_MEMORY;
    }
    
    data->counter = 0;
    idcu_sdk_set_user_data(ctx, data);
    
    return IDCU_ERR_OK;
}

// 启动
static int hello_start(idcu_SdkContext* ctx)
{
    idcu_sdk_log_info(ctx, "Starting hello module");
    return IDCU_ERR_OK;
}

// 停止
static int hello_stop(idcu_SdkContext* ctx)
{
    idcu_sdk_log_info(ctx, "Stopping hello module");
    return IDCU_ERR_OK;
}

// 销毁
static void hello_destroy(idcu_SdkContext* ctx)
{
    idcu_sdk_log_info(ctx, "Destroying hello module");
    
    HelloModuleData* data = (HelloModuleData*)idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

// 使用 SDK 注册模块
IDCU_SDK_MODULE_DEFINE(
    hello,                    // 模块名
    "1.0.0",                  // 版本
    "A hello world module",   // 描述
    hello_init,               // 初始化
    hello_start,              // 启动
    hello_stop,               // 停止
    hello_destroy             // 销毁
);
```

### 第三步：创建 CMakeLists.txt

在 `modules/business/hello/` 目录下创建 `CMakeLists.txt`：

```cmake
add_library(idcu_business_hello STATIC
    src/hello_module.c
)

target_include_directories(idcu_business_hello PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modules/core/common/include
    ${CMAKE_SOURCE_DIR}/modules/core/utils/include
    ${CMAKE_SOURCE_DIR}/modules/core/module-system/include
    ${CMAKE_SOURCE_DIR}/modules/core/sdk/include
)

target_link_libraries(idcu_business_hello PRIVATE
    idcu_core_common
    idcu_core_utils
    idcu_core_module_system
    idcu_core_sdk
)
```

### 第四步：更新主 CMakeLists.txt

在项目根目录的 `CMakeLists.txt` 中，添加你的模块：

找到 "业务模块" 部分，添加：
```cmake
add_subdirectory(modules/business/hello)
```

然后在链接选项中添加：
```cmake
idcu_business_hello
```

（可以参考其他业务模块的写法）

### 第五步：更新配置文件

在 `config/agent.cfg` 中，添加你的模块：

```ini
[modules]
business = ... , hello_module   # 把你的模块加在这里

# 启用模块
enable_hello_module = true

# 设置优先级
hello_module.priority = normal
```

### 第六步：编译测试

现在编译并运行，看看你的模块是不是正常工作了！

```bash
# Windows
scripts\build.bat

# Linux
./scripts/build.sh

# 运行
cd out
./idcu_agent
```

如果一切顺利，你会看到你的模块输出消息！

### 参考现有模块

如果不确定怎么写，可以参考现有的模块：
- `modules/business/examples/simple-example/` - 简单示例
- `modules/business/examples/messaging-example/` - 消息示例
- `modules/business/core-module/` - 核心模块

---

## 前缀命名空间

为了避免命名冲突，本项目所有公共符号都使用 `idcu_` 前缀。

### 命名规则

| 符号类型 | 规则 | 示例 |
|---------|------|------|
| 类型定义 | `idcu_TypeName` | `idcu_Mutex`, `idcu_ErrorCode` |
| 函数名 | `idcu_function_name()` | `idcu_mutex_init()`, `idcu_log_info()` |
| 宏定义 | `IDCU_MACRO_NAME` | `IDCU_ERR_OK`, `IDCU_CONFIG_MAX_MODULES` |
| 枚举值 | `IDCU_ENUM_VALUE` | `IDCU_MOD_STATE_INITED` |
| 结构体成员 | 无需前缀 | `name`, `init` |
| 局部变量 | 无需前缀 | `count`, `result` |
| 静态函数 | 无需前缀 | `my_helper_func()` |
| 全局变量 | 加 `g_` 前缀 | `g_kernel`, `g_counter` |
| 静态变量 | 加 `s_` 前缀（可选） | `s_initialized` |

### 为什么要用前缀？

想象一下，如果你的模块叫 `log.h`，里面有个函数叫 `init()`，而另一个模块也叫 `init()`，编译器就不知道该用哪个了。加上前缀就不会有这个问题。

---

## API 参考

### 模块生命周期

| 函数 | 说明 | 调用时机 |
|------|------|---------|
| `init()` | 模块加载时调用一次 | 启动时 |
| `run()` | 内核主循环中反复调用 | 运行时 |
| `stop()` | 模块卸载时调用 | 停止时 |

### 常用 API 速查

#### 日志相关
```c
idcu_log_init(const char* file, idcu_LogLevel level);
idcu_log_debug(const char* fmt, ...);
idcu_log_info(const char* fmt, ...);
idcu_log_warn(const char* fmt, ...);
idcu_log_error(const char* fmt, ...);
idcu_log_shutdown();
```

#### 模块相关
```c
idcu_kernel_init(idcu_MicroKernel* k);
idcu_kernel_start_modules(idcu_MicroKernel* k);
idcu_kernel_run(idcu_MicroKernel* k);
idcu_kernel_stop(idcu_MicroKernel* k);
idcu_kernel_hotplug_load(idcu_MicroKernel* k, const char* name, const char* path);
idcu_kernel_hotplug_unload(idcu_MicroKernel* k, const char* name);
```

#### 消息总线
```c
idcu_MessageBus* idcu_msg_bus_create(void);
int idcu_msg_bus_send(idcu_MessageBus* bus, idcu_Message* msg);
int idcu_msg_bus_recv(idcu_MessageBus* bus, uint32_t receiver_id, idcu_Message* msg);
void idcu_msg_bus_destroy(idcu_MessageBus* bus);
```

#### 协程
```c
idcu_CoroScheduler* idcu_coro_create(void);
int idcu_coro_spawn(idcu_CoroScheduler* sched, idcu_CoroFunc func, void* arg, idcu_CoroPrio prio);
void idcu_coro_run(idcu_CoroScheduler* sched);
void idcu_coro_destroy(idcu_CoroScheduler* sched);
```

### 更多 API

请参考各个头文件中的详细注释，特别是：
- `modules/core/common/include/`
- `modules/core/utils/include/`
- `modules/core/module-system/include/`
- `modules/core/scheduler/include/`
- `modules/core/micro-kernel/include/`
- `modules/core/sdk/include/`

---

## 编译调试技巧

### 调试编译

想调试代码时，可以用 Debug 模式编译：

```bash
# Windows
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ..
mingw32-make

# Linux
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### 代码格式化

项目带了 `.clang-format` 配置文件，可以一键格式化代码：

```bash
# 格式化单个文件
clang-format -i your_file.c

# 格式化整个目录
find . -name "*.c" -o -name "*.h" | xargs clang-format -i
```

### 静态分析

项目带了 `.clang-tidy` 配置文件，可以做静态分析：

```bash
clang-tidy your_file.c -- -Iinclude
```

### 常见调试技巧

#### 1. 多打日志
在关键位置加日志，看看执行到哪了：

```c
idcu_log_debug("[hello] Entering function, count=%d", count);
// ... 代码 ...
idcu_log_debug("[hello] Function done, result=%d", result);
```

#### 2. 用 GDB 调试（Linux）

```bash
gdb ./out/idcu_agent
(gdb) break main.c:18  # 在第18行打断点
(gdb) run                 # 运行
(gdb) next                # 单步执行
(gdb) print count         # 查看变量值
(gdb) backtrace           # 查看调用栈
(gdb) continue            # 继续运行
```

#### 3. 用 Visual Studio 调试（Windows）

如果你用 Visual Studio，可以生成 VS 解决方案：

```bash
mkdir build
cd build
cmake -G "Visual Studio 16 2019" ..
```

然后打开生成的 `.sln` 文件，就可以用 VS 调试了。

---

## 代码规范

写代码时请遵循这些规范，保持代码风格一致。

### 命名规范

- **文件名**：全小写，用下划线分隔，如 `micro_kernel.c`
- **公共函数名**：加 `idcu_` 前缀，全小写，用下划线分隔，如 `idcu_kernel_init()`
- **宏定义**：加 `IDCU_` 前缀，全大写，用下划线分隔，如 `IDCU_MODULE_VERSION()`
- **全局变量**：加 `g_` 前缀，如 `g_kernel`
- **静态变量**：加 `s_` 前缀（可选），如 `s_initialized`
- **结构体/枚举**：用大驼峰，加 `idcu_` 前缀，如 `idcu_ModuleState`

### 注释规范

- **函数注释**：说明函数做什么，参数是什么，返回值是什么
- **复杂逻辑**：写清楚为什么这么做，而不是做了什么
- **不用写废话**：代码本身能说明的不用注释

**好的注释示例：**
```c
/**
 * 初始化微内核
 * @param k 微内核实例指针
 * @return 成功返回 IDCU_ERR_OK，失败返回错误码
 */
int idcu_kernel_init(idcu_MicroKernel* k)
{
    // ...
}
```

### 代码风格

看现有代码的风格，保持一致：

```c
// 大括号换行
if (condition)
{
    do_something();
}
else
{
    do_something_else();
}

// 指针靠近类型
int* ptr;

// 运算符两边加空格
a = b + c;

// 逗号后面加空格
func(a, b, c);

// 缩进用 4 个空格（不要用 Tab）
```

### 错误处理

总是检查返回值，不要忽略错误：

```c
// 不好的写法
do_something();

// 好的写法
int ret = do_something();
if (ret != IDCU_ERR_OK)
{
    idcu_log_error("Failed to do something: %d", ret);
    return ret;
}
```

---

## 测试指南

### 单元测试

项目有自己的测试框架，在 `modules/core/test-framework/`。

**写一个简单的测试：**

```c
#include "test_framework.h"

TEST_CASE(test_addition)
{
    int a = 1;
    int b = 2;
    ASSERT_EQUAL(a + b, 3);
}

TEST_CASE(test_string)
{
    const char* str = "hello";
    ASSERT_STRING_EQUAL(str, "hello");
}

int main(void)
{
    RUN_TEST(test_addition);
    RUN_TEST(test_string);
    PRINT_TEST_SUMMARY();
    return 0;
}
```

### 编译和运行测试

```bash
mkdir build
cd build
cmake -DBUILD_TESTS=ON ..
make
ctest --output-on-failure
```

### 测试覆盖率（Linux）

```bash
mkdir build
cd build
cmake -DBUILD_TESTS=ON -DENABLE_CODE_COVERAGE=ON ..
make
make coverage
```

然后在浏览器中打开 `build/coverage_report/index.html` 查看覆盖率报告。

---

## 下一步

1. 先看懂现有的示例模块：
   - `modules/business/examples/simple-example/src/simple_example.c`
   - `modules/business/examples/messaging-example/`
   - `modules/business/core-module/src/core_module.c`

2. 试试修改它们，看看效果

3. 按照上面的步骤创建自己的模块

4. 探索服务模块代码，了解核心功能是如何工作的

5. 阅读其他教程：
   - [教程一：快速入门](./tutorial_01_quick_start.md)
   - [教程二：模块开发](./tutorial_02_module_development.md)
   - [教程三：消息总线](./tutorial_03_message_bus.md)
   - [教程四：协程调度](./tutorial_04_coroutine.md)
   - [教程五：沙箱安全](./tutorial_05_sandbox.md)

---

## 祝你开发愉快！

记住：
- 多看现有代码，代码是最好的文档！
- 有问题先看注释和文档
- 多写测试，保证代码质量
- 代码风格保持一致

有问题欢迎在社区提问！🎉
