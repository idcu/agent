# 模块化积木式架构设计指南

## 概述

IDCU Agent 采用**配置驱动的模块化积木式架构**，核心思想是：

1. **服务核心** - 服务模块提供基础功能
2. **积木式组装** - 所有业务功能通过业务模块实现，像搭积木一样组合
3. **分类分级** - 模块按功能类别和优先级分级管理
4. **配置驱动** - 通过配置文件启用/禁用模块，无需修改代码
5. **易于扩展** - 新增模块只需添加文件和配置，不影响现有代码

---

## 架构分层

```
┌─────────────────────────────────────────────────────────┐
│  Level 3: 业务层 (Business)        核心业务功能        │
├─────────────────────────────────────────────────────────┤
│  Level 2: 服务层 (Service)         监控/网络/安全      │
├─────────────────────────────────────────────────────────┤
│  Level 1: 核心基础设施层 (Core)    微内核/调度/模块管理 │
└─────────────────────────────────────────────────────────┘
```

### 模块分类说明

| 层级 | 分类 | 说明 | 示例 |
|------|------|------|------|
| Level 1 | core | 核心基础设施，提供微内核架构基础 | common, utils, module-system, scheduler, micro-kernel, sdk, test-framework |
| Level 2 | service | 服务层模块，提供特定服务 | monitor, network, security, plugin |
| Level 3 | business | 业务功能模块，实现核心业务 | log, metrics, alert, collect |

---

## 配置文件详解

配置文件位于 `config/agent.cfg`，采用 INI 格式。

### 1. 模块启用/禁用

通过 `enable_<模块名>` 配置项控制模块是否启用：

```ini
[modules]
# 启用核心业务模块
enable_core_module = true
enable_log_module = true

# 可选业务模块
enable_collect_module = true
enable_heartbeat_module = true
enable_alert_module = false  # 禁用告警模块
```

### 2. 模块特定配置

每个模块可以有自己的配置 section，格式为 `[module.<模块名>]`：

```ini
[module.heartbeat_module]
interval_ms = 5000
timeout_ms = 15000

[module.metrics_module]
collection_interval_ms = 5000
export_enabled = true
```

---

## 开发新模块指南

### 步骤 1: 创建模块文件

在 `modules/` 目录下对应的分类文件夹中创建模块目录：

```
modules/
├── core/                     # 核心基础设施组
│   ├── common/               # 通用基础组件
│   ├── utils/                # 通用工具组件
│   ├── module-system/        # 模块管理系统
│   ├── scheduler/            # 调度器模块
│   ├── micro-kernel/         # 微内核核心
│   ├── sdk/                  # 软件开发工具包
│   └── test-framework/       # 测试框架
├── services/                 # 服务模块组
│   ├── monitor/              # 监控系统
│   ├── network/              # 网络相关
│   ├── security/             # 安全相关
│   └── plugin/               # 插件系统
└── business/                 # 业务模块组
    ├── core-module/          # 核心基础模块
    ├── alert/                # 告警模块
    ├── collect/              # 采集模块
    ├── config/               # 配置模块
    ├── data-collector/       # 数据采集模块
    ├── healthcheck/          # 健康检查模块
    ├── heartbeat/            # 心跳模块
    ├── http-management/      # HTTP管理模块
    ├── log/                  # 日志模块
    ├── metrics/              # 指标模块
    ├── examples/             # 示例模块
    └── dynamic/              # 动态加载模块示例
```

### 步骤 2: 实现模块接口

模块需要实现基本的生命周期函数：

```c
#include <stdio.h>

// 模块内部状态
static int g_my_counter = 0;

// 初始化函数
static int my_module_init()
{
    printf("[my_module] initialized\n");
    return 0;
}

// 运行函数（主循环中调用）
static int my_module_run()
{
    g_my_counter++;
    if (g_my_counter % 1000000 == 0) {
        printf("[my_module] running (counter: %d)\n", g_my_counter);
    }
    return 0;
}

// 停止函数
static int my_module_stop()
{
    printf("[my_module] stopped (counter: %d)\n", g_my_counter);
    return 0;
}
```

### 步骤 3: 创建模块的 CMakeLists.txt

在新模块目录下创建 `CMakeLists.txt`，参考其他模块的配置。

### 步骤 4: 在配置文件中添加模块

在 `config/agent.cfg` 中：

1. 配置启用状态
2. （可选）添加模块特定配置

```ini
[modules]
enable_my_module = true

[module.my_module]
my_config = 200
```

---

## 核心组件说明

### 1. 监控服务 (`modules/services/monitor/`)

负责：
- 健康检查
- 指标收集
- 告警管理
- Prometheus 导出

### 2. 网络服务 (`modules/services/network/`)

负责：
- 网络层封装
- 连接池管理
- HTTP 服务器
- 分布式节点管理
- 节点发现

### 3. 安全服务 (`modules/services/security/`)

负责：
- 沙箱安全机制
- 权限控制
- 资源限制

### 4. 插件服务 (`modules/services/plugin/`)

负责：
- 插件生态系统管理
- 第三方插件集成

---

## 最佳实践

### 1. 模块设计原则

- **单一职责** - 每个模块只负责一个功能
- **松耦合** - 模块间通过消息总线通信，不直接依赖
- **可配置** - 关键参数通过配置文件设置
- **无状态** - 尽量避免全局状态，或通过配置管理

### 2. 配置管理

- 为模块提供合理的默认值
- 使用 `module.<模块名>` 作为配置前缀
- 在模块初始化时读取配置
- 重要配置变更需要重启模块

### 3. 错误处理

- 初始化失败返回错误码
- 运行时错误尽量恢复，不要轻易崩溃
- 使用日志记录错误信息

### 4. 依赖管理

- 尽量减少模块间依赖
- 服务模块被业务模块依赖
- 业务模块之间尽量减少直接依赖

---

## 常见问题

### Q: 新增模块需要重新编译吗？

A: 是的，静态模块需要重新编译。如果需要动态加载，可以使用动态模块功能。

### Q: 如何在运行时启用/禁用模块？

A: 当前版本需要修改配置文件并重启程序。

### Q: 模块间如何通信？

A: 通过消息总线发送和接收消息。

### Q: 如何调试模块？

A: 查看日志文件，或使用调试输出。

---

## 总结

新的模块化积木式架构提供了：

✅ **配置驱动** - 无需修改代码即可调整模块组合  
✅ **分类分级** - 清晰的模块层级和职责划分  
✅ **易于扩展** - 新增模块只需添加文件和配置  
✅ **灵活组合** - 像搭积木一样组装功能  

通过这种架构，你可以：
- 快速原型开发 - 只启用需要的模块
- 生产环境部署 - 按需组合功能模块
- 功能定制 - 通过配置调整系统行为
- 团队协作 - 不同模块可独立开发和测试
