# IDCU Agent 架构设计文档

## 概述

IDCU Agent 采用微内核架构设计，核心只负责基本的调度和通信功能，所有业务逻辑通过模块实现。

## 系统架构图

```
┌─────────────────────────────────────────────────────────┐
│                    应用层 (Application)                  │
├─────────────────────────────────────────────────────────┤
│                    模块层 (Modules)                      │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐            │
│  │ base_log │  │biz_collect│  │   ...    │            │
│  └──────────┘  └──────────┘  └──────────┘            │
├─────────────────────────────────────────────────────────┤
│                 微内核层 (Micro Kernel)                   │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐            │
│  │ 协程调度 │  │ 消息总线 │  │ 沙箱安全 │            │
│  └──────────┘  └──────────┘  └──────────┘            │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐            │
│  │ 模块管理 │  │ 监控检测 │  │ 网络通信 │            │
│  └──────────┘  └──────────┘  └──────────┘            │
└─────────────────────────────────────────────────────────┘
```

## 核心模块说明

### 1. 微内核 (Micro Kernel)
**文件位置**: `include/kernel/micro_kernel.h` / `src/kernel/micro_kernel.c`

微内核是整个系统的核心，负责：
- 协程调度器的管理
- 消息总线的初始化和维护
- 沙箱环境的创建
- 模块的加载、初始化和启动

### 2. 协程调度器 (Coroutine Scheduler)
**文件位置**: `include/scheduler/coroutine.h` / `src/scheduler/coroutine.c`

提供轻量级的协程调度功能，支持：
- 多优先级协程
- 时间片轮转调度
- 协程挂起和恢复
- 性能统计

### 3. 消息总线 (Message Bus)
**文件位置**: `include/scheduler/msg_bus.h` / `src/scheduler/msg_bus.c`

模块间通信的核心组件，特性包括：
- 多优先级消息队列
- 零拷贝消息传输
- 消息广播
- 批量消息处理

### 4. 沙箱安全 (Sandbox)
**文件位置**:
- `include/security/sandbox.h` / `src/security/sandbox.c`
- `include/security/sandbox_enhanced.h` / `src/security/sandbox_enhanced.c`

提供模块隔离和安全机制：
- 权限控制 (发送、接收、运行、硬件访问等)
- 内存区域保护
- 系统调用白名单
- 资源限制 (CPU、内存、文件描述符)

### 5. 模块管理 (Module Management)
**文件位置**:
- `include/module/module_def.h` - 模块接口定义
- `include/module/module_registry.h` / `src/module/module_registry.c` - 模块注册表
- `include/module/dynamic_module.h` / `src/module/dynamic_loader.c` - 动态模块加载

支持模块的生命周期管理：
- 静态模块注册
- 动态模块加载 (Windows/Linux)
- 模块初始化、运行、停止
- 模块依赖管理

### 6. 监控检测 (Monitoring)
**文件位置**:
- `include/monitor/health_check.h` / `src/monitor/health_check.c` - 健康检查
- `include/monitor/metrics.h` / `src/monitor/metrics.c` - 指标收集

提供系统监控功能：
- 模块心跳检测
- 健康状态管理
- 性能指标收集
- Prometheus 格式导出

### 7. 网络通信 (Network)
**文件位置**:
- `include/network/network_layer.h` / `src/network/network_layer.c` - 网络层
- `include/network/distributed_node.h` / `src/network/distributed_node.c` - 分布式节点
- `include/network/node_discovery.h` / `src/network/node_discovery.c` - 节点发现

支持网络通信和分布式功能：
- TCP/UDP Socket 封装
- 分布式节点管理
- 节点自动发现
- 节点间消息传递

## 目录结构

```
idcu-agent/
├── include/              # 公共头文件
│   ├── common/           # 通用工具 (atomic, lock, error_code, config)
│   ├── utils/            # 工具模块 (log, memory_pool, json_parser, config_manager)
│   ├── scheduler/        # 调度相关 (coroutine, msg_bus, context)
│   ├── module/           # 模块系统 (module_def, module_registry, dynamic_module)
│   ├── monitor/          # 监控系统 (health_check, metrics)
│   ├── network/          # 网络相关 (network_layer, distributed_node, node_discovery)
│   ├── security/         # 安全相关 (sandbox, sandbox_enhanced)
│   ├── plugin/           # 插件系统 (plugin_ecosystem)
│   ├── test/             # 测试框架 (test_framework)
│   └── kernel/           # 内核核心 (micro_kernel)
├── src/                  # 源代码实现
│   └── (与 include 对应)
├── modules/              # 业务模块
│   ├── base/             # 基础模块
│   └── biz/              # 业务模块
├── config/               # 配置文件
├── tests/                # 测试用例
├── docs/                 # 文档目录
└── CMakeLists.txt
```

## 数据流

1. **模块启动流程**:
   ```
   main() 
     → idcu_kernel_init() 
       → 初始化协程调度器
       → 初始化消息总线
       → 初始化沙箱
     → idcu_kernel_start_modules()
       → 加载并初始化所有模块
     → idcu_kernel_run()
       → 主循环：调度协程 + 处理消息
   ```

2. **模块间通信**:
   ```
   模块A 
     → idcu_msg_send() 
       → 消息总线队列
     → 模块B 
       → idcu_msg_recv() 
         → 处理消息
   ```

3. **协程调度**:
   ```
   协程创建 
     → 加入就绪队列
   调度器循环 
     → 选择最高优先级协程
     → 运行时间片
     → 切换到下一个协程
   ```

## 前缀命名空间

为了避免命名冲突，本项目所有公共符号都使用 `idcu_` 前缀：

| 符号类型 | 规则 | 示例 |
|---------|------|------|
| 类型定义 | `idcu_TypeName` | `idcu_Mutex`, `idcu_ErrorCode` |
| 函数名 | `idcu_function_name()` | `idcu_kernel_init()` |
| 宏定义 | `IDCU_MACRO_NAME` | `IDCU_ERR_OK`, `IDCU_CONFIG_MAX_MODULES` |
| 枚举值 | `IDCU_ENUM_VALUE` | `IDCU_MOD_STATE_INITED` |

## 扩展指南

### 添加新功能模块

1. 在 `modules/` 目录下创建模块文件
2. 实现 `idcu_ModuleInterface` 接口
3. 使用 `IDCU_REGISTER_MODULE` 宏注册模块
4. 在微内核中添加模块引用

### 添加新的核心组件

1. 在 `include/` 对应子目录创建头文件
2. 在 `src/` 对应子目录创建实现文件
3. 更新 `CMakeLists.txt`
4. 在微内核中集成新组件

## 设计原则

1. **最小化核心**: 微内核只包含最基本的功能
2. **模块化**: 所有功能通过模块实现
3. **松耦合**: 模块间通过消息总线通信
4. **安全性**: 沙箱隔离，权限控制
5. **可扩展**: 易于添加新功能和模块
