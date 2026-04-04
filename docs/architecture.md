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
│                 服务层 (Services)                        │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐            │
│  │ 监控系统 │  │ 网络系统 │  │ 安全系统 │            │
│  └──────────┘  └──────────┘  └──────────┘            │
│  ┌──────────┐                                          │
│  │ 插件系统 │                                          │
│  └──────────┘                                          │
└─────────────────────────────────────────────────────────┘
```

## 核心模块说明

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

### 4. 插件系统 (Plugin)
**文件位置**: `modules/services/plugin/`

提供插件生态系统支持：

- **插件加载和卸载**
- **插件依赖管理**
- **插件生命周期管理**

## 目录结构

```
idcu-agent/
├── app/                          # 应用程序入口
├── modules/                      # 模块化结构
│   ├── services/                 # 服务模块组
│   │   ├── monitor/              # 监控系统
│   │   │   ├── include/          # 头文件
│   │   │   ├── src/              # 源代码
│   │   │   ├── tests/            # 测试
│   │   │   ├── CMakeLists.txt
│   │   │   ├── README.md
│   │   │   └── module.json
│   │   ├── network/              # 网络相关
│   │   │   ├── include/          # 头文件
│   │   │   ├── src/              # 源代码
│   │   │   ├── tests/            # 测试
│   │   │   ├── CMakeLists.txt
│   │   │   ├── README.md
│   │   │   └── module.json
│   │   ├── security/             # 安全相关
│   │   │   ├── include/          # 头文件
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
│       └── dynamic/              # 动态加载模块示例
├── module-repo/                  # 模块仓库
├── tests/                        # 测试用例
├── examples/                     # 示例程序
├── config/                       # 配置文件
├── docs/                         # 文档目录
└── CMakeLists.txt
```

## 数据流

1. **模块启动流程**:
   ```
   main() 
     → 初始化服务模块
       → 初始化监控系统
       → 初始化网络系统
       → 初始化安全系统
     → 加载业务模块
       → 初始化业务模块
     → 主循环
       → 调度模块运行
       → 处理消息
   ```

2. **模块间通信**:
   ```
   模块A 
     → 发送消息
       → 消息总线队列
     → 模块B 
       → 接收消息
         → 处理消息
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

### 添加新业务模块

1. 在 `modules/business/` 目录下创建模块目录
2. 实现模块功能
3. 创建 CMakeLists.txt
4. 编译测试

### 添加新的服务组件

1. 在 `modules/services/` 对应子目录创建头文件和实现文件
2. 更新 CMakeLists.txt
3. 编写测试
4. 集成到系统中

## 设计原则

1. **最小化核心**: 服务模块提供基础功能，业务逻辑在业务模块中实现
2. **模块化**: 所有功能通过模块实现
3. **松耦合**: 模块间通过消息总线通信
4. **安全性**: 沙箱隔离，权限控制
5. **可扩展**: 易于添加新功能和模块
