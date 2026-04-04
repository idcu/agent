# IDCU Agent Developer Guide

欢迎来到 IDCU Agent 开发者世界！这篇文档会教你如何修改代码、添加新功能。

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

---

## 架构介绍

### 什么是微内核架构？

简单说，微内核就像一个"插座板"，核心很小，只负责最基本的功能，其他功能都通过"插件"（模块）来实现。

### IDCU Agent 的架构

```
┌─────────────────────────────────────────────────────┐
│              应用层 (Application)                    │
├─────────────────────────────────────────────────────┤
│              模块层 (Modules)                        │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐              │
│  │ log     │ │collect  │ │ ...     │              │
│  └─────────┘ └─────────┘ └─────────┘              │
├─────────────────────────────────────────────────────┤
│           微内核层 (Micro Kernel)                     │
│  ┌──────────────────────────────────────────────┐  │
│  │ 微内核 (Micro Kernel)                        │  │
│  ├──────────────────────────────────────────────┤  │
│  │ 调度器 + 模块系统                             │  │
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

```
idcu-agent/
├── app/                          # 应用程序入口
│   ├── main.c                    # 主程序入口
│   └── benchmark.c               # 性能基准测试
├── modules/                      # 模块化结构
│   ├── core/                     # 核心基础设施 (微内核架构基础)
│   │   ├── common/               # 通用基础组件
│   │   │   ├── include/          # 原子操作、锁、错误码、编译配置
│   │   │   ├── src/              # 源代码
│   │   │   ├── CMakeLists.txt
│   │   │   └── README.md
│   │   ├── utils/                # 通用工具组件
│   │   │   ├── include/          # 日志、配置、JSON、内存池、权限
│   │   │   ├── src/              # 源代码
│   │   │   ├── tests/            # 测试
│   │   │   ├── CMakeLists.txt
│   │   │   └── README.md
│   │   ├── module-system/        # 模块管理系统
│   │   │   ├── include/          # 模块定义、注册表、分类、动态加载、版本
│   │   │   ├── src/              # 源代码
│   │   │   ├── tests/            # 测试
│   │   │   ├── CMakeLists.txt
│   │   │   └── README.md
│   │   ├── scheduler/            # 调度器模块
│   │   │   ├── include/          # 协程、消息总线、上下文
│   │   │   ├── src/              # 源代码
│   │   │   ├── tests/            # 测试
│   │   │   ├── CMakeLists.txt
│   │   │   └── README.md
│   │   ├── micro-kernel/         # 微内核核心
│   │   │   ├── include/          # 微内核主接口
│   │   │   ├── src/              # 源代码
│   │   │   ├── tests/            # 测试
│   │   │   ├── CMakeLists.txt
│   │   │   └── README.md
│   │   ├── sdk/                  # 软件开发工具包
│   │   │   ├── include/          # SDK 接口
│   │   │   ├── src/              # 源代码
│   │   │   ├── CMakeLists.txt
│   │   │   └── README.md
│   │   └── test-framework/       # 测试框架
│   │       ├── include/          # 测试框架接口
│   │       ├── src/              # 源代码
│   │       ├── CMakeLists.txt
│   │       └── README.md
│   ├── services/                 # 服务模块组
│   │   ├── monitor/              # 监控系统
│   │   │   ├── include/          # 头文件 (健康检查、指标、告警、通知、Prometheus导出)
│   │   │   ├── src/              # 源代码
│   │   │   ├── tests/            # 测试
│   │   │   ├── CMakeLists.txt
│   │   │   ├── README.md
│   │   │   └── module.json
│   │   ├── network/              # 网络相关
│   │   │   ├── include/          # 头文件 (网络层、连接池、HTTP服务器、分布式节点、节点发现、管理API)
│   │   │   ├── src/              # 源代码
│   │   │   ├── tests/            # 测试
│   │   │   ├── CMakeLists.txt
│   │   │   ├── README.md
│   │   │   └── module.json
│   │   ├── security/             # 安全相关
│   │   │   ├── include/          # 头文件 (沙箱基础版、增强版)
│   │   │   ├── src/              # 源代码
│   │   │   ├── tests/            # 测试
│   │   │   ├── CMakeLists.txt
│   │   │   ├── README.md
│   │   │   └── module.json
│   │   └── plugin/               # 插件系统
│   │       ├── include/          # 头文件
│   │       ├── src/              # 源代码
│   │       ├── tests/            # 测试
│   │       ├── CMakeLists.txt
│   │       ├── README.md
│   │       └── module.json
│   └── business/                 # 业务模块组
│       ├── core-module/          # 核心基础模块
│       ├── alert/                # 告警模块
│       ├── collect/              # 采集模块
│       ├── config/               # 配置模块
│       ├── data-collector/       # 数据采集模块
│       ├── healthcheck/          # 健康检查模块
│       ├── heartbeat/            # 心跳模块
│       ├── http-management/      # HTTP管理模块
│       ├── log/                  # 日志模块
│       ├── metrics/              # 指标模块
│       ├── examples/             # 示例模块
│       │   ├── simple-example/   # 简单示例
│       │   ├── messaging-example/ # 消息示例
│       │   └── advanced-example/ # 高级示例
│       └── dynamic/              # 动态加载模块示例
├── module-repo/                  # 模块仓库
│   ├── official/                 # 官方模块
│   ├── templates/                # 模块模板
│   └── index.json
├── config/                       # 配置文件
│   └── agent.cfg
├── tests/                        # 测试用例
│   ├── unit/                     # 单元测试
│   └── integration/              # 集成测试
├── examples/                     # 示例程序
├── docs/                         # 文档目录
├── scripts/                      # 构建和辅助脚本
│   ├── build.bat                 # Windows 编译脚本
│   └── build.sh                  # Linux 编译脚本
├── templates/                    # 模块模板
├── CMakeLists.txt                # CMake 构建脚本
├── .clang-format                 # 代码格式化配置
├── .clang-tidy                   # 代码静态分析配置
└── .editorconfig                 # 编辑器配置
```

### 关键文件说明

#### core/ 目录 - 核心基础设施
- `common/` - 通用基础组件（原子操作、锁机制、错误码、编译配置）
- `utils/` - 通用工具组件（日志系统、配置管理、JSON 解析、内存池、权限管理）
- `module-system/` - 模块管理系统（模块定义、注册表、分类、动态加载、版本管理）
- `scheduler/` - 调度器模块（协程调度、消息总线、上下文管理）
- `micro-kernel/` - 微内核核心（系统核心管理、模块生命周期、热插拔）
- `sdk/` - 软件开发工具包（简化模块开发接口）
- `test-framework/` - 测试框架（单元测试支持）

#### services/ 目录 - 核心服务实现
- `monitor/` - 监控系统，包括健康检查、指标收集、告警管理、Prometheus 导出
- `network/` - 网络系统，包括网络层、HTTP 服务器、分布式节点、节点发现、管理 API
- `security/` - 安全系统，包括沙箱基础版和增强版
- `plugin/` - 插件系统

#### business/ 目录 - 业务模块
- 各个业务功能模块的实现
- 包含示例模块和测试模块

---

## 核心模块说明

### 1. 核心基础设施模块

#### 通用基础组件 (common)
**文件位置**: `modules/core/common/`

提供项目通用的基础工具组件：
- **原子操作** (`atomic.h/c`) - 原子变量操作、原子计数器、无锁数据结构支持
- **锁机制** (`lock.h/c`) - 互斥锁、读写锁、条件变量
- **错误码** (`error_code.h/c`) - 错误码定义、错误码转字符串、统一错误处理
- **编译时配置** (`config.h`) - 编译时配置选项

#### 工具组件 (utils)
**文件位置**: `modules/core/utils/`

提供项目通用的工具组件：
- **日志系统** (`log.h/c`) - 多级别日志 (DEBUG/INFO/WARN/ERROR)、日志格式化、日志输出管理
- **配置管理** (`config_manager.h/c`) - 配置文件读取、配置项查询、配置更新
- **JSON 解析器** (`json_parser.h/c`) - JSON 解析、JSON 生成、JSON 数据操作
- **内存池** (`memory_pool.h/c`) - 高效内存分配、内存复用、内存泄漏检测
- **权限管理** (`permission_manager.h/c`) - 权限定义、权限检查、权限管理

#### 模块管理系统 (module-system)
**文件位置**: `modules/core/module-system/`

提供完整的模块管理功能：
- **模块定义** (`module_def.h`) - 模块接口定义、模块状态管理、模块注册宏
- **模块注册表** (`module_registry.h/c`) - 模块注册和管理、依赖关系图构建、拓扑排序、配置应用
- **模块分类** (`module_category.h/c`) - 模块分类和层级管理、模块配置加载
- **动态模块** (`dynamic_module.h/c`) - 运行时加载 DLL/SO、热插拔支持
- **模块版本** (`module_version.h/c`) - 版本号解析和比较、依赖版本检查

#### 调度器模块 (scheduler)
**文件位置**: `modules/core/scheduler/`

提供核心调度功能：
- **协程** (`coroutine.h/c`) - 轻量级协程调度、多优先级协程、时间片轮转调度、协程挂起和恢复、性能统计
- **消息总线** (`msg_bus.h/c`) - 模块间通信、多优先级消息队列、零拷贝消息传输、消息广播、批量消息处理
- **上下文** (`context.h/c`) - 协程上下文管理、上下文切换

#### 微内核核心 (micro-kernel)
**文件位置**: `modules/core/micro-kernel/`

系统最核心的功能：
- **微内核主接口** (`micro_kernel.h/c`)
  - `idcu_kernel_init()` - 初始化微内核
  - `idcu_kernel_start_modules()` - 启动所有模块
  - `idcu_kernel_run()` - 运行主循环
  - `idcu_kernel_stop()` - 停止所有模块
  - `idcu_kernel_hotplug_load()` - 热加载模块
  - `idcu_kernel_hotplug_unload()` - 热卸载模块

功能包括：
- 协程调度器的管理
- 消息总线的初始化和维护
- 沙箱环境的创建
- 模块的加载、初始化和启动
- 健康状态监控

#### 软件开发工具包 (SDK)
**文件位置**: `modules/core/sdk/`

提供外部开发接口，封装底层复杂性：
- **SDK 接口** (`sdk.h/c`) - 模块开发简化接口、消息发送/接收封装、常用工具函数

#### 测试框架 (Test Framework)
**文件位置**: `modules/core/test-framework/`

提供单元测试框架支持：
- **测试框架** (`test_framework.h/c`) - 测试用例注册、测试断言宏、测试运行器、测试结果统计

### 2. 服务模块

#### 监控系统 (monitor)
**文件位置**: `modules/services/monitor/`

提供完整的监控功能：
- **健康检查** (`health_check.h/c`) - 模块健康状态检测
- **指标收集** (`metrics.h/c`) - 性能指标收集和管理
- **告警管理** (`alert_manager.h/c`) - 告警规则和通知
- **通知器** (`notifier.h/c`) - 告警通知发送
- **Prometheus 导出** (`prometheus_exporter.h/c`) - Prometheus 格式指标导出

#### 网络系统 (network)
**文件位置**: `modules/services/network/`

提供网络和分布式功能：
- **网络层** (`network_layer.h/c`) - TCP/UDP Socket 封装
- **连接池** (`connection_pool.h/c`) - 连接复用管理
- **HTTP 服务器** (`http_server.h/c`) - HTTP 服务实现
- **管理 API** (`management_api.h/c`) - RESTful 管理接口
- **分布式节点** (`distributed_node.h/c`) - 多节点通信
- **节点发现** (`node_discovery.h/c`) - 节点自动发现

#### 安全系统 (security)
**文件位置**: `modules/services/security/`

提供安全隔离功能：
- **沙箱基础版** (`sandbox.h/c`) - 基础权限控制和资源限制
- **沙箱增强版** (`sandbox_enhanced.h/c`) - 增强的安全机制

### 3. 现有业务模块示例

#### modules/business/log/src/log_module.c
```c
// 参考现有模块实现
```

---

## 如何添加新模块

这是最重要的部分！跟着下面的步骤，你就能添加自己的模块了。

### 第一步：创建模块目录

假设你要创建一个叫 `hello` 模块：

1. 在 `modules/business/` 目录下创建 `hello/` 目录
2. 使用 `templates/module/` 中的模板创建必要的文件，或者参考现有的业务模块结构

### 第二步：写模块代码

在 `modules/business/hello/src/hello_module.c` 中复制下面的模板，然后修改：

```c
#include <stdio.h>

static int hello_init(void)
{
    printf("[hello] init\n");
    return 0;
}

static int hello_run(void)
{
    return 0;
}

static int hello_stop(void)
{
    printf("[hello] stop\n");
    return 0;
}

// 参考现有模块的注册方式
```

### 第三步：创建模块的 CMakeLists.txt

在 `modules/business/hello/` 目录下创建 `CMakeLists.txt`，参考其他业务模块的配置。

### 第四步：编译测试

按照 [user_guide.md](./user_guide.md) 里的方法编译运行，看看你的模块是不是正常工作了！

---

## 前缀命名空间

为了避免命名冲突，本项目所有公共符号都使用 `idcu_` 前缀：

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

---

## API 参考

### 模块生命周期

| 函数 | 说明 |
|------|------|
| init() | 模块加载时调用一次 |
| run() | 内核主循环中反复调用 |
| stop() | 模块卸载时调用 |

### 更多 API

请参考各个头文件中的详细注释，特别是 `modules/services/*/include/` 下的头文件。

---

## 编译调试技巧

### 调试编译

想调试代码时，可以在 CMakeLists.txt 中添加调试选项。

### 代码格式化

项目带了 `.clang-format` 配置文件，格式化代码：

```bash
clang-format -i 你的文件.c
```

### 常见调试技巧

1. **多打 printf**：在关键位置加 printf 看看执行到哪了
2. **用 GDB**：
   ```bash
   gdb ./out/idcu_agent
   (gdb) break main.c:18  # 在第18行打断点
   (gdb) run                 # 运行
   (gdb) next                # 单步执行
   (gdb) print 变量名       # 查看变量值
   ```

---

## 代码规范

### 命名规范

- 文件名：全小写，用下划线分隔，如 `micro_kernel.c`
- 公共函数名：加 `idcu_` 前缀，全小写，用下划线分隔，如 `idcu_kernel_init()`
- 宏定义：加 `IDCU_` 前缀，全大写，用下划线分隔，如 `IDCU_MODULE_VERSION()`
- 全局变量：加 `g_` 前缀，如 `g_running`
- 静态变量：加 `s_` 前缀（可选）

### 注释规范

- 函数注释：说明函数做什么，参数是什么，返回值是什么
- 复杂逻辑：写清楚为什么这么做
- 不用写废话：代码本身能说明的不用注释

### 示例代码风格

看现有代码的风格，保持一致：

```c
// 大括号换行
if (condition)
{
    do_something();
}

// 指针靠近类型
int* ptr;

// 运算符两边加空格
a = b + c;

// 逗号后面加空格
func(a, b, c);
```

---

## 下一步

1. 先看懂现有的示例模块：
   - `modules/business/log/src/log_module.c`
   - `modules/business/collect/src/collect_module.c`
   - `modules/services/monitor/src/` 下的服务实现

2. 试试修改它们，看看效果

3. 按照上面的步骤创建自己的模块

4. 探索服务模块代码，了解核心功能是如何工作的

---

祝你开发愉快！有问题多看代码，代码是最好的文档！🎉
