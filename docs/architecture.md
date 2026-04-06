# IDCU Agent 架构设计文档

欢迎阅读 IDCU Agent 的架构设计文档！这篇文档会深入讲解系统的设计理念和实现细节，帮助你更好地理解这个项目。

---

## 目录

1. [概述](#概述)
2. [系统架构图](#系统架构图)
3. [核心基础设施层](#核心基础设施层)
4. [服务模块层](#服务模块层)
5. [业务模块层](#业务模块层)
6. [目录结构](#目录结构)
7. [数据流](#数据流)
8. [设计原则](#设计原则)

---

## 概述

IDCU Agent 采用微内核架构设计，核心只负责基本的调度和通信功能，所有业务逻辑通过模块实现。

### 什么是微内核架构？

微内核架构是一种软件设计模式，它将操作系统或应用程序的核心功能最小化，其他功能都通过"插件"或"模块"来实现。

**类比：**
- **微内核** = 万能插座板
- **模块** = 各种电器（台灯、电视、空调等）

**优势：**
1. **核心代码小** - 容易理解和维护
2. **扩展性好** - 想加新功能，写个模块就行
3. **安全性高** - 模块之间隔离，出问题影响范围小
4. **可测试性好** - 每个模块可以独立测试

---

## 系统架构图

```
┌─────────────────────────────────────────────────────────┐
│                    应用层 (Application)                  │
│                (主程序入口 app/main.c)                   │
├─────────────────────────────────────────────────────────┤
│                    模块层 (Modules)                      │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐            │
│  │ base_log │  │biz_collect│  │  alert   │            │
│  │  log     │  │  config  │  │ heartbeat│            │
│  └──────────┘  └──────────┘  └──────────┘            │
├─────────────────────────────────────────────────────────┤
│                 服务层 (Services)                        │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐            │
│  │ 监控系统 │  │ 网络系统 │  │ 安全系统 │            │
│  │ (monitor)│  │(network) │  │(security)│            │
│  └──────────┘  └──────────┘  └──────────┘            │
│  ┌──────────┐  ┌──────────┐                            │
│  │ 存储服务 │  │ 缓存服务 │                            │
│  │(storage) │  │ (cache)  │                            │
│  └──────────┘  └──────────┘                            │
├─────────────────────────────────────────────────────────┤
│              核心基础设施层 (Core Infrastructure)         │
│  ┌──────────────────────────────────────────────────┐  │
│  │ 微内核 (Micro Kernel) - 核心管理              │  │
│  ├──────────────────────────────────────────────────┤  │
│  │ 调度器 (Scheduler) + 消息总线 (Message Bus)    │  │
│  ├──────────────────────────────────────────────────┤  │
│  │ 模块系统 (Module System) - 注册+加载+版本      │  │
│  ├──────────────────────────────────────────────────┤  │
│  │ 工具组件 (Utils) + 通用基础 (Common)          │  │
│  ├──────────────────────────────────────────────────┤  │
│  │ SDK + 测试框架                                │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

---

## 核心基础设施层

这是整个系统的基石，提供微内核架构的基础功能。

### 1. 通用基础库 (idcu-common)

**文件位置**: `libs/idcu-common/`

提供项目通用的基础工具组件：

- **原子操作** (`atomic.h/c`)
  - 原子变量操作
  - 原子计数器
  - 无锁数据结构支持

- **锁机制** (`lock.h/c`)
  - 互斥锁 (Mutex)
  - 读写锁 (RWLock)
  - 条件变量 (Condition Variable)

- **错误码** (`error_code.h/c`)
  - 统一错误码定义
  - 错误码转字符串
  - 错误处理工具

- **编译时配置** (`config.h`)
  - 编译时配置选项
  - 平台相关定义

- **数据结构**
  - 向量 (`vector.h/c`) - 动态数组
  - 链表 (`linked_list.h/c`) - 双向链表
  - 哈希表 (`hash_map.h/c`) - 哈希表
  - 字符串缓冲区 (`string_buf.h/c`) - 动态字符串
  - 选项类型 (`option.h/c`) - 可选值类型

### 2. 日志系统库 (idcu-log)

**文件位置**: `libs/idcu-log/`

提供灵活高效的日志功能：

- **日志系统** (`log.h/c`)
  - 多级别日志 (DEBUG/INFO/WARN/ERROR/FATAL)
  - 日志格式化
  - 日志输出管理（控制台/文件）

### 3. 配置管理库 (idcu-config)

**文件位置**: `libs/idcu-config/`

提供配置文件管理功能：

- **配置管理** (`config.h/c`)
  - INI 格式配置文件读取
  - 配置项查询
  - 配置更新

### 4. JSON 解析库 (idcu-json)

**文件位置**: `libs/idcu-json/`

提供 JSON 数据处理功能：

- **JSON 解析器** (`json.h/c`)
  - JSON 解析
  - JSON 生成
  - JSON 数据操作

### 5. 网络层库 (idcu-network)

**文件位置**: `libs/idcu-network/`

提供网络通信功能：

- **网络层** (`network_layer.h/c`)
  - TCP/UDP Socket 封装
  - 网络连接管理
  - 数据收发接口

### 6. HTTP 服务器库 (idcu-http-server)

**文件位置**: `libs/idcu-http-server/`

提供轻量级 HTTP 服务：

- **HTTP 服务器** (`http_server.h/c`)
  - 轻量级 HTTP 服务实现
  - 请求路由
  - 响应处理

### 7. HTTP 客户端库 (idcu-http-client)

**文件位置**: `libs/idcu-http-client/`

提供 HTTP 客户端功能：

- **HTTP 客户端** (`http_client.h/c`)
  - HTTP 请求发送
  - 响应接收和解析

### 8. 连接池库 (idcu-conn-pool)

**文件位置**: `libs/idcu-conn-pool/`

提供连接复用管理：

- **连接池** (`conn_pool.h/c`)
  - 连接复用管理
  - 连接生命周期管理
  - 连接健康检查

### 9. 分布式节点库 (idcu-distributed)

**文件位置**: `libs/idcu-distributed/`

提供分布式节点通信：

- **分布式节点** (`distributed.h/c`)
  - 多节点通信
  - 节点状态同步
  - 数据同步机制

### 10. 节点发现库 (idcu-discovery)

**文件位置**: `libs/idcu-discovery/`

提供节点自动发现功能：

- **节点发现** (`discovery.h/c`)
  - 节点自动发现
  - 节点广播
  - 发现协议实现

### 11. 指标收集库 (idcu-metrics)

**文件位置**: `libs/idcu-metrics/`

提供性能指标收集：

- **指标收集** (`metrics.h/c`)
  - 性能指标收集
  - 指标分类管理
  - 指标查询接口
  - Prometheus 格式指标导出 (`prometheus_exporter.h/c`)

### 12. 健康检查库 (idcu-healthcheck)

**文件位置**: `libs/idcu-healthcheck/`

提供健康状态检查：

- **健康检查** (`healthcheck.h/c`)
  - 模块心跳检测
  - 健康状态管理
  - 健康状态查询接口

### 13. 告警系统库 (idcu-alert)

**文件位置**: `libs/idcu-alert/`

提供告警通知功能：

- **告警管理** (`alert.h/c`)
  - 告警规则定义
  - 告警条件判断
  - 告警状态管理
- **通知器** (`notifier.h/c`)
  - 告警通知发送
  - 多渠道通知支持

### 14. 内存池库 (idcu-memory)

**文件位置**: `libs/idcu-memory/`

提供高效内存管理：

- **内存池** (`memory_pool.h/c`)
  - 高效内存分配
  - 内存复用
  - 内存泄漏检测

### 15. 权限管理库 (idcu-permission)

**文件位置**: `libs/idcu-permission/`

提供权限管理功能：

- **权限管理** (`permission.h/c`)
  - 权限定义
  - 权限检查
  - 权限管理

### 16. 沙箱安全库 (idcu-sandbox)

**文件位置**: `libs/idcu-sandbox/`

提供模块隔离和安全机制：

- **沙箱基础版** (`sandbox.h/c`)
  - 权限控制 (发送、接收、运行、硬件访问等)
  - 内存区域保护
  - 资源限制 (CPU、内存、文件描述符)

### 3. 模块管理系统 (Module System)

**文件位置**: `modules/core/module-system/`

这是最核心的部分之一，提供完整的模块管理功能：

- **模块定义** (`module_def.h`)
  - 模块接口定义
  - 模块状态管理
  - 模块注册宏

- **模块注册表** (`module_registry.h/c`)
  - 模块注册和管理
  - 依赖关系图构建
  - 拓扑排序
  - 配置应用

- **模块分类** (`module_category.h/c`)
  - 模块分类和层级管理
  - 模块配置加载

- **动态模块** (`dynamic_module.h/c`)
  - 运行时加载 DLL/SO
  - 热插拔支持

- **模块版本** (`module_version.h/c`)
  - 版本号解析和比较
  - 依赖版本检查

### 4. 调度器模块 (Scheduler)

**文件位置**: `modules/core/scheduler/`

提供核心调度功能：

- **协程** (`coroutine.h/c`)
  - 轻量级协程调度
  - 多优先级协程
  - 时间片轮转调度
  - 协程挂起和恢复
  - 性能统计

- **消息总线** (`msg_bus.h/c`)
  - 模块间通信
  - 多优先级消息队列
  - 零拷贝消息传输
  - 消息广播
  - 批量消息处理

- **上下文** (`context.h/c`)
  - 协程上下文管理
  - 上下文切换

### 5. 微内核核心 (Micro Kernel)

**文件位置**: `modules/core/micro-kernel/`

系统最核心的功能：

```c
typedef struct {
    idcu_CoroScheduler coro;           // 协程调度器
    idcu_MessageBus    msg;            // 消息总线
    idcu_Sandbox       sandbox[16];    // 沙箱环境
    uint32_t           sb_cnt;
    idcu_StackContext  global;         // 全局上下文
    volatile int       should_exit;    // 退出标志
    idcu_TrackedModule tracked_modules[16];  // 跟踪的模块
    uint32_t           tracked_cnt;
    idcu_DynamicLoader dynamic_loader; // 动态加载器
    idcu_HealthMonitor health_monitor; // 健康监控
    // ... 更多字段
} idcu_MicroKernel;
```

**主要接口：**
- `idcu_kernel_init()` - 初始化微内核
- `idcu_kernel_start_modules()` - 启动所有模块
- `idcu_kernel_run()` - 运行主循环
- `idcu_kernel_stop()` - 停止所有模块
- `idcu_kernel_hotplug_load()` - 热加载模块
- `idcu_kernel_hotplug_unload()` - 热卸载模块

**功能包括：**
- 协程调度器的管理
- 消息总线的初始化和维护
- 沙箱环境的创建
- 模块的加载、初始化和启动
- 健康状态监控

### 6. 软件开发工具包 (SDK)

**文件位置**: `modules/core/sdk/`

提供外部开发接口，封装底层复杂性：

- **SDK 接口** (`sdk.h/c`)
  - 模块开发简化接口
  - 消息发送/接收封装
  - 常用工具函数

### 7. 测试框架 (Test Framework)

**文件位置**: `modules/core/test-framework/`

提供单元测试框架支持：

- **测试框架** (`test_framework.h/c`)
  - 测试用例注册
  - 测试断言宏
  - 测试运行器
  - 测试结果统计

---

## 服务模块层

这些是可复用的服务组件，为业务模块提供基础服务。

### 1. 监控系统 (Monitor)

**文件位置**: `modules/services/monitor/`

提供完整的监控和可观测性功能：

- **健康检查** (`health_check.h/c`)
  - 模块心跳检测
  - 健康状态管理
  - 健康状态查询接口

- **指标收集** (`metrics.h/c`)
  - 性能指标收集
  - 指标分类管理
  - 指标查询接口

- **告警管理** (`alert_manager.h/c`)
  - 告警规则定义
  - 告警条件判断
  - 告警状态管理

- **通知器** (`notifier.h/c`)
  - 告警通知发送
  - 多渠道通知支持

- **Prometheus 导出** (`prometheus_exporter.h/c`)
  - Prometheus 格式指标导出
  - HTTP 端点提供

### 2. 网络系统 (Network)

**文件位置**: `modules/services/network/`

提供网络通信和分布式功能：

- **网络层** (`network_layer.h/c`)
  - TCP/UDP Socket 封装
  - 网络连接管理
  - 数据收发接口

- **连接池** (`connection_pool.h/c`)
  - 连接复用管理
  - 连接生命周期管理
  - 连接健康检查

- **HTTP 服务器** (`http_server.h/c`)
  - 轻量级 HTTP 服务实现
  - 请求路由
  - 响应处理

- **管理 API** (`management_api.h/c`)
  - RESTful 管理接口
  - 模块管理 API
  - 状态查询 API

- **分布式节点** (`distributed_node.h/c`)
  - 多节点通信
  - 节点状态同步
  - 数据同步机制

- **节点发现** (`node_discovery.h/c`)
  - 节点自动发现
  - 节点广播
  - 发现协议实现

### 3. 安全系统 (Security)

**文件位置**: `modules/services/security/`

提供模块隔离和安全机制：

- **沙箱基础版** (`sandbox.h/c`)
  - 权限控制 (发送、接收、运行、硬件访问等)
  - 内存区域保护
  - 资源限制 (CPU、内存、文件描述符)

- **沙箱增强版** (`sandbox_enhanced.h/c`)
  - 系统调用白名单
  - 增强的权限检查
  - 更严格的资源限制

---

## 业务模块层

这里是具体的功能模块，也是你最可能添加代码的地方。

### 现有业务模块

- **core-module** - 核心基础模块
- **log** - 日志模块
- **alert** - 告警模块
- **collect** - 采集模块
- **config** - 配置模块
- **data-collector** - 数据采集模块
- **healthcheck** - 健康检查模块
- **heartbeat** - 心跳模块
- **http-client** - HTTP 客户端模块
- **metrics** - 指标模块
- **storage** - 存储模块
- **cache** - 缓存模块
- **security** - 安全模块

### 示例模块

项目还包含一些示例模块，供你学习参考：
- **simple-example** - 简单示例
- **messaging-example** - 消息通信示例
- **advanced-example** - 高级示例

你可以在 `modules/business/examples/` 目录下找到它们。

---

## 目录结构

```
idcu-agent/
├── app/                          # 应用程序入口
│   ├── main.c                    # 主程序入口
│   └── benchmark.c               # 性能基准测试
├── config/                       # 配置文件
│   └── agent.cfg                 # 主配置文件
├── libs/                         # 独立库模块（可复用的基础组件）
│   ├── idcu-module-build/        # 模块构建工具
│   ├── idcu-common/              # 通用基础组件（原子操作、锁、错误码等）
│   ├── idcu-log/                 # 日志系统
│   ├── idcu-config/              # 配置管理
│   ├── idcu-json/                # JSON 解析
│   ├── idcu-network/             # 网络层（TCP/UDP Socket）
│   ├── idcu-http-server/         # HTTP 服务器
│   ├── idcu-http-client/         # HTTP 客户端
│   ├── idcu-conn-pool/           # 连接池
│   ├── idcu-distributed/         # 分布式节点
│   ├── idcu-discovery/           # 节点发现
│   ├── idcu-metrics/             # 指标收集
│   ├── idcu-healthcheck/         # 健康检查
│   ├── idcu-alert/               # 告警系统
│   ├── idcu-memory/              # 内存池
│   ├── idcu-permission/          # 权限管理
│   └── idcu-sandbox/             # 沙箱安全
├── modules/                      # 模块化结构
│   ├── core/                     # 核心基础设施
│   │   ├── micro-kernel/         # 微内核核心
│   │   ├── module-system/        # 模块管理系统
│   │   ├── scheduler/            # 调度器模块
│   │   ├── sdk/                  # 软件开发工具包
│   │   └── test-framework/       # 测试框架
│   ├── services/                 # 服务模块组
│   │   ├── storage/              # 存储服务
│   │   ├── cache/                # 缓存服务
│   │   ├── security/             # 安全系统
│   │   └── plugin/               # 插件系统
│   ├── integrations/             # 集成模块（连接库和模块系统）
│   │   ├── config-integration/
│   │   ├── healthcheck-integration/
│   │   ├── http-client-integration/
│   │   ├── http-server-integration/
│   │   ├── json-integration/
│   │   ├── log-integration/
│   │   ├── metrics-integration/
│   │   ├── network-integration/
│   │   └── network-monitor-integration/
│   └── business/                 # 业务模块组
│       ├── core-module/          # 核心基础模块
│       ├── alert/                # 告警模块
│       ├── collect/              # 采集模块
│       ├── config/               # 配置模块
│       ├── data-collector/       # 数据采集模块
│       ├── healthcheck/          # 健康检查模块
│       ├── heartbeat/            # 心跳模块
│       ├── http-client/          # HTTP客户端模块
│       ├── log/                  # 日志模块
│       ├── metrics/              # 指标模块
│       ├── storage/              # 存储模块
│       ├── cache/                # 缓存模块
│       ├── security/             # 安全模块
│       ├── examples/             # 示例模块
│       │   ├── simple-example/   # 简单示例
│       │   └── messaging-example/# 消息示例
│       └── dynamic/              # 动态加载模块示例
├── module-repo/                  # 模块仓库
│   ├── official/                 # 官方模块
│   │   ├── core/                 # 核心模块
│   │   ├── monitoring/           # 监控模块
│   │   ├── networking/           # 网络模块
│   │   └── security/             # 安全模块
│   └── templates/                # 模块模板
├── tests/                        # 测试用例
│   ├── unit/                     # 单元测试
│   └── integration/              # 集成测试
├── examples/                     # 示例程序
├── docs/                         # 文档目录
│   ├── api/                     # API 文档
│   └── ...                      # 其他文档
├── scripts/                      # 构建脚本
│   ├── build.bat                 # Windows 编译脚本
│   ├── build.sh                  # Linux 编译脚本
│   └── generate_coverage.sh      # 覆盖率生成脚本
├── out/                          # 编译输出目录
├── CMakeLists.txt                # CMake 构建配置
├── CMakeLists_benchmark.txt      # 性能测试配置
├── Doxyfile                      # Doxygen 配置
├── CODE_OF_CONDUCT.md            # 行为准则
├── CONTRIBUTING.md               # 贡献指南
└── README.md                     # 项目说明
```

---

## 数据流

### 1. 模块启动流程

```
main() 
  ↓
初始化日志系统 (idcu_log_init)
  ↓
初始化微内核 (idcu_kernel_init)
  ↓
设置信号处理器 (idcu_kernel_set_signal_handler)
  ↓
加载配置文件 (idcu_kernel_load_config)
  ↓
启动所有模块 (idcu_kernel_start_modules)
  ├─→ 按拓扑排序加载模块
  ├─→ 调用每个模块的 init()
  └─→ 调用每个模块的 run()
  ↓
运行主循环 (idcu_kernel_run)
  ├─→ 协程调度
  ├─→ 消息分发
  ├─→ 健康检查
  └─→ ...
  ↓
(收到 Ctrl+C)
  ↓
停止所有模块 (idcu_kernel_stop)
  ├─→ 调用每个模块的 stop()
  └─→ 清理资源
  ↓
关闭日志系统 (idcu_log_shutdown)
  ↓
退出
```

### 2. 模块间通信流程

```
模块A (发送消息)
  ↓
idcu_msg_bus_send()
  ↓
消息总线 (放入队列)
  ↓
(内核主循环分发)
  ↓
模块B (接收消息)
  ↓
idcu_msg_bus_recv()
  ↓
处理消息
```

### 3. 协程调度流程

```
主协程
  ↓
idcu_coro_spawn() - 创建协程
  ↓
加入调度队列
  ↓
(主循环调度)
  ↓
协程A 运行 → 主动让出 → 协程B 运行 → ...
  ↓
(时间片到或主动挂起)
  ↓
切换上下文
  ↓
继续下一个协程
```

---

## 设计原则

### 1. 最小化核心

服务模块提供基础功能，业务逻辑在业务模块中实现。微内核只负责：
- 模块管理
- 协程调度
- 消息总线
- 基本的健康监控

### 2. 模块化

所有功能通过模块实现，每个模块：
- 有清晰的接口
- 独立的生命周期
- 可以单独启用/禁用

### 3. 松耦合

模块间通过消息总线通信，不直接依赖：
- 降低模块间的耦合度
- 提高系统的可维护性
- 便于测试和替换

### 4. 安全性

- 沙箱隔离：模块运行在沙箱环境中
- 权限控制：细粒度的权限管理
- 资源限制：限制模块的资源使用

### 5. 可扩展

- 易于添加新功能和模块
- 支持动态加载模块
- 清晰的扩展点

---

## 总结

IDCU Agent 的架构设计遵循微内核理念，通过模块化、松耦合的设计，实现了一个灵活、安全、易扩展的实时代理系统。

如果你想深入了解某个部分，可以：
- 查看对应的头文件和源文件
- 阅读其他教程文档
- 参考现有模块的实现

希望这篇架构文档对你有帮助！🎉
