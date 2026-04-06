# IDCU Agent 模块分离开发指南

> **文档版本**: v1.0  
> **创建日期**: 2026-04-06  
> **状态**: 待审核

---

## 目录

1. [概述](#1-概述)
2. [模块分离原则](#2-模块分离原则)
3. [建议分离模块清单](#3-建议分离模块清单)
4. [分离步骤指南](#4-分离步骤指南)
5. [各模块详细分离计划](#5-各模块详细分离计划)
6. [验收标准](#6-验收标准)
7. [风险与注意事项](#7-风险与注意事项)

---

## 1. 概述

本文档详细说明了 IDCU Agent 项目中剩余需要分离为独立库的模块，以及完整的分离实施指南。

### 1.1 当前已分离模块

项目已成功将 **18 个模块** 分离为独立库（位于 `libs/` 目录）：

| 层级 | 库名 | 说明 |
|------|------|------|
| 基础层 | idcu-common | 基础公共库（原子操作、锁、数据结构） |
| 基础层 | idcu-log | 日志系统 |
| 基础层 | idcu-json | JSON 解析 |
| 基础层 | idcu-config | 配置管理 |
| 基础层 | idcu-memory | 内存池 |
| 基础层 | idcu-permission | 权限管理 |
| 网络层 | idcu-network | 基础网络层 |
| 网络层 | idcu-http-server | HTTP 服务器 |
| 网络层 | idcu-http-client | HTTP 客户端 |
| 网络层 | idcu-conn-pool | 连接池 |
| 网络层 | idcu-discovery | 节点发现 |
| 网络层 | idcu-distributed | 分布式节点 |
| 服务层 | idcu-metrics | 指标收集 |
| 服务层 | idcu-healthcheck | 健康检查 |
| 服务层 | idcu-alert | 告警系统 |
| 服务层 | idcu-sandbox | 沙箱安全 |
| 工具 | idcu-module-build | 模块构建工具 |

### 1.2 目标

本文档指导分离剩余的 **8-9 个核心模块**，使项目总计拥有 **26-27 个独立库**。

---

## 2. 模块分离原则

### 2.1 是否应该分离的判断标准

✅ **适合分离的模块**：
- 功能通用，可在其他项目中独立使用
- 与微内核架构无强耦合
- API 清晰，输入输出明确
- 不依赖 `micro_kernel.h`、`module_system.h` 等项目特定头文件
- 依赖关系简单（主要依赖 idcu-common 等基础库）

❌ **不适合分离的模块**：
- 微内核核心（micro-kernel）
- 集成层模块（integrations/）
- 业务模块（business/）
- 与项目架构强耦合的代码

### 2.2 目录结构规范

每个独立库应遵循以下结构：

```
libs/idcu-xxx/
├── include/
│   └── idcu/
│       └── xxx/
│           ├── xxx.h
│           └── ...
├── src/
│   └── idcu/
│       └── xxx/
│           ├── xxx.c
│           └── ...
├── tests/
│   ├── test_xxx.c
│   └── ...
├── examples/
│   ├── example_xxx.c
│   └── ...
├── CMakeLists.txt
├── module.json
└── README.md
```

### 2.3 命名规范

- 库名：`idcu-xxx`（使用短横线）
- 头文件命名空间：`idcu/xxx/`
- 函数前缀：`idcu_xxx_`
- CMake 目标别名：`idcu::xxx`

---

## 3. 建议分离模块清单

### 3.1 高优先级（P0 - 强烈建议分离）

| 序号 | 建议库名 | 当前位置 | 说明 | 预估工作量 |
|------|----------|----------|------|-----------|
| 1 | idcu-coroutine | modules/core/scheduler/coroutine | 协程调度器 | 0.5 天 |
| 2 | idcu-msgbus | modules/core/scheduler/msg_bus | 消息总线 | 0.5 天 |
| 3 | idcu-storage | modules/services/storage | 存储服务 | 0.5 天 |
| 4 | idcu-cache | modules/services/cache | 缓存服务 | 0.5 天 |
| 5 | idcu-module-system | modules/core/module-system | 模块管理系统 | 1 天 |
| 6 | idcu-testframework | modules/core/test-framework | 测试框架 | 0.5 天 |

**P0 总计**：3.5 天

---

### 3.2 中优先级（P1 - 建议分离）

| 序号 | 建议库名 | 当前位置 | 说明 | 预估工作量 |
|------|----------|----------|------|-----------|
| 7 | idcu-plugin | modules/services/plugin | 插件生态系统 | 0.5 天 |
| 8 | idcu-security-enhanced | modules/services/security | 增强安全组件 | 0.5 天 |

**P1 总计**：1 天

---

### 3.3 低优先级（P2 - 可选分离）

| 序号 | 建议库名 | 当前位置 | 说明 | 预估工作量 |
|------|----------|----------|------|-----------|
| 9 | idcu-sdk | modules/core/sdk | SDK（可选） | 0.5 天 |

---

## 4. 分离步骤指南

### 4.1 标准分离流程

以 `idcu-coroutine` 为例，详细说明分离步骤：

#### 步骤 1: 创建目录结构

```bash
# 在 libs/ 下创建新库目录
mkdir -p libs/idcu-coroutine/include/idcu/coroutine
mkdir -p libs/idcu-coroutine/src/idcu/coroutine
mkdir -p libs/idcu-coroutine/tests
mkdir -p libs/idcu-coroutine/examples
```

#### 步骤 2: 移动头文件

将 `modules/core/scheduler/include/coroutine.h` 移动到：
`libs/idcu-coroutine/include/idcu/coroutine/coroutine.h`

**注意更新头文件 include 路径**：

```c
// 修改前
#include "context.h"
#include "idcu/common/config.h"
#include "idcu/common/lock.h"

// 修改后（保持不变，因为是相对路径或已使用命名空间）
#include "context.h"
#include "idcu/common/config.h"
#include "idcu/common/lock.h"
```

同时移动 `context.h` 到同一目录。

#### 步骤 3: 移动源文件

将 `modules/core/scheduler/src/coroutine.c` 和 `context.c` 移动到：
`libs/idcu-coroutine/src/idcu/coroutine/`

#### 步骤 4: 创建 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)

include("${CMAKE_SOURCE_DIR}/libs/idcu-module-build/cmake/idcu_module_build.cmake")
idcu_build_module()
```

#### 步骤 5: 创建 module.json

```json
{
  "name": "idcu-coroutine",
  "category": "library",
  "version": "1.0.0",
  "description": "IDCU 协程调度器库，提供轻量级协程调度功能",
  "author": "IDCU Team",
  "license": "MIT",
  "dependencies": ["idcu-common"],
  "priority": "high",
  "type": "library",
  "build": {
    "language": "C",
    "standard": "99",
    "type": "static",
    "sources": ["src/idcu/coroutine/*.c"],
    "includes": ["include"],
    "link_libraries": ["idcu-common"],
    "platform_deps": {
      "win32": [],
      "unix": ["pthread"]
    }
  }
}
```

#### 步骤 6: 创建 README.md

参考 `libs/idcu-common/README.md` 的格式，包含：
- 库简介
- 特性
- 快速开始（构建、安装、使用）
- API 文档
- 测试
- 示例
- 许可证

#### 步骤 7: 迁移测试用例

将 `modules/core/scheduler/tests/test_coroutine.c` 移动到：
`libs/idcu-coroutine/tests/test_coroutine.c`

更新测试中的 include 路径：
```c
// 修改前
#include "coroutine.h"

// 修改后
#include <idcu/coroutine/coroutine.h>
```

#### 步骤 8: 创建示例代码

在 `libs/idcu-coroutine/examples/` 下创建至少 2 个示例：
- `example_coroutine_basic.c` - 基础协程使用
- `example_coroutine_priority.c` - 优先级调度

#### 步骤 9: 更新根目录 CMakeLists.txt

在"独立库"部分添加：
```cmake
add_subdirectory(libs/idcu-coroutine)
```

#### 步骤 10: 更新原模块的 CMakeLists.txt

修改 `modules/core/scheduler/CMakeLists.txt`，移除对原代码的引用，改为依赖新库：

```cmake
# 修改前
file(GLOB_RECURSE MODULE_SRCS src/*.c)
add_library(${MODULE_NAME} STATIC ${MODULE_SRCS})

# 修改后
add_library(${MODULE_NAME} INTERFACE)
target_link_libraries(${MODULE_NAME} INTERFACE
    idcu::coroutine
    idcu::msgbus
)
```

#### 步骤 11: 修复所有编译错误

1. 更新所有使用旧头文件的代码
2. 更新 CMakeLists.txt 中的依赖
3. 完整编译项目

#### 步骤 12: 运行测试

确保所有测试通过。

---

## 5. 各模块详细分离计划

### 5.1 idcu-coroutine（协程调度器）

**当前位置**：`modules/core/scheduler/`

**需要移动的文件**：
- `include/coroutine.h` → `include/idcu/coroutine/coroutine.h`
- `include/context.h` → `include/idcu/coroutine/context.h`
- `src/coroutine.c` → `src/idcu/coroutine/coroutine.c`
- `src/context.c` → `src/idcu/coroutine/context.c`
- `tests/test_coroutine.c` → `tests/test_coroutine.c`

**依赖**：idcu-common

**module.json** 关键配置：
```json
{
  "name": "idcu-coroutine",
  "dependencies": ["idcu-common"],
  "build": {
    "link_libraries": ["idcu-common"]
  }
}
```

---

### 5.2 idcu-msgbus（消息总线）

**当前位置**：`modules/core/scheduler/`

**需要移动的文件**：
- `include/msg_bus.h` → `include/idcu/msgbus/msg_bus.h`
- `src/msg_bus.c` → `src/idcu/msgbus/msg_bus.c`
- `tests/test_msg_bus.c` → `tests/test_msg_bus.c`

**依赖**：idcu-common, idcu-coroutine

**module.json** 关键配置：
```json
{
  "name": "idcu-msgbus",
  "dependencies": ["idcu-common", "idcu-coroutine"],
  "build": {
    "link_libraries": ["idcu-common", "idcu-coroutine"]
  }
}
```

---

### 5.3 idcu-storage（存储服务）

**当前位置**：`modules/services/storage/`

**需要移动的文件**：
- `include/storage.h` → `include/idcu/storage/storage.h`
- `src/storage.c` → `src/idcu/storage/storage.c`

**依赖**：idcu-common

**注意**：需要检查是否依赖 SQLite 库，在 module.json 中正确配置。

**module.json** 关键配置：
```json
{
  "name": "idcu-storage",
  "dependencies": ["idcu-common"],
  "build": {
    "link_libraries": ["idcu-common"],
    "platform_deps": {
      "win32": [],
      "unix": ["sqlite3"]
    }
  }
}
```

---

### 5.4 idcu-cache（缓存服务）

**当前位置**：`modules/services/cache/`

**需要移动的文件**：
- `include/cache.h` → `include/idcu/cache/cache.h`
- `src/cache.c` → `src/idcu/cache/cache.c`

**依赖**：idcu-common

---

### 5.5 idcu-module-system（模块管理系统）

**当前位置**：`modules/core/module-system/`

**需要移动的文件**：
- `include/module_def.h` → `include/idcu/module/module_def.h`
- `include/module_registry.h` → `include/idcu/module/module_registry.h`
- `include/module_category.h` → `include/idcu/module/module_category.h`
- `include/module_version.h` → `include/idcu/module/module_version.h`
- `include/dynamic_module.h` → `include/idcu/module/dynamic_module.h`
- `src/*.c` → `src/idcu/module/`
- `tests/*.c` → `tests/`

**依赖**：idcu-common, idcu-log

**注意**：这是一个较大的模块，需要仔细检查是否有对 micro-kernel 的依赖。

---

### 5.6 idcu-testframework（测试框架）

**当前位置**：`modules/core/test-framework/`

**需要移动的文件**：
- `include/test_framework.h` → `include/idcu/testframework/test_framework.h`
- `src/test_framework.c` → `src/idcu/testframework/test_framework.c`

**依赖**：idcu-common

---

### 5.7 idcu-plugin（插件生态系统）

**当前位置**：`modules/services/plugin/`

**需要移动的文件**：
- `include/plugin_ecosystem.h` → `include/idcu/plugin/plugin_ecosystem.h`
- `src/plugin_ecosystem.c` → `src/idcu/plugin/plugin_ecosystem.c`
- `tests/test_plugin_ecosystem.c` → `tests/test_plugin_ecosystem.c`

**依赖**：idcu-common

---

### 5.8 idcu-security-enhanced（增强安全组件）

**当前位置**：`modules/services/security/`

**需要移动的文件**：
- `include/sandbox_enhanced.h` → `include/idcu/security/sandbox_enhanced.h`
- `include/tls.h` → `include/idcu/security/tls.h`
- `include/audit_log.h` → `include/idcu/security/audit_log.h`
- `src/sandbox_enhanced.c` → `src/idcu/security/sandbox_enhanced.c`
- `src/tls.c` → `src/idcu/security/tls.c`
- `src/audit_log.c` → `src/idcu/security/audit_log.c`
- `tests/test_sandbox_enhanced.c` → `tests/test_sandbox_enhanced.c`

**依赖**：idcu-common, idcu-sandbox

**注意**：`sandbox.h` 已在 `libs/idcu-sandbox/` 中，不要重复移动。

---

### 5.9 idcu-sdk（可选）

**当前位置**：`modules/core/sdk/`

**建议**：暂不分离，因为与微内核架构强耦合。如果未来需要分离，需要进行大量重构。

---

## 6. 验收标准

### 6.1 每个独立库的验收标准

对于每个提取的独立库，必须满足：

- [ ] 可以独立构建（不依赖项目其他部分）
- [ ] 支持 `cmake --install` 安装到系统
- [ ] 其他项目可以通过 `find_package()` 找到并使用
- [ ] 有完整的单元测试
- [ ] 有 README.md，包含快速开始指南
- [ ] 至少有 2 个可运行的示例代码
- [ ] 无项目特定的依赖（如 `micro_kernel.h`）
- [ ] 代码风格符合项目规范
- [ ] 头文件使用正确的命名空间（`idcu/xxx/`）

### 6.2 整体项目验收标准

- [ ] 无循环依赖
- [ ] 项目可以完整编译
- [ ] 所有测试通过
- [ ] 功能与分离前一致
- [ ] 文档完整且最新

---

## 7. 风险与注意事项

### 7.1 主要风险

| 风险 | 可能性 | 影响 | 缓解措施 |
|------|--------|------|---------|
| 分离期间引入 bug | 高 | 高 | 1. 每步小改动，频繁提交 2. 充分测试 |
| 进度延期 | 中 | 中 | 1. 优先保证 P0 任务 2. 每周进度检查 |
| 功能回归 | 中 | 高 | 1. 完整的回归测试 2. 保持 API 兼容 |

### 7.2 注意事项

1. **保持 API 兼容**：分离过程中不要改变公共 API
2. **分步实施**：一次只分离一个模块，确认稳定后再进行下一个
3. **充分测试**：每个模块分离后都要运行完整测试套件
4. **提交规范**：使用清晰的 commit message，便于回滚
5. **文档同步**：及时更新相关文档

---

## 附录

### A. 参考资料

- [现有独立库示例](../libs/idcu-common/)
- [重构计划文档](./refactoring_plan.md)
- [架构文档](./architecture.md)

### B. 快速检查表

分离一个模块前，检查：

- [ ] 阅读本文档
- [ ] 查看现有独立库的结构
- [ ] 确认模块没有项目特定依赖
- [ ] 准备好测试用例
- [ ] 与团队沟通计划

---

**文档结束**
