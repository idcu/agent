# IDCU Agent 完美开发指南

> **文档版本**: v4.0 - 终极完美版
> **最后更新**: 2026-04-08
> **适用人群**: 开发者、架构师
> **目标**: 基于经验，从零开始开发一个更优秀的 IDCU Agent

---

## 目录

1. [前言：为什么这份指南更完美](#1-前言为什么这份指南更完美)
2. [开发前准备检查清单](#2-开发前准备检查清单)
3. [如何参考现有代码](#3-如何参考现有代码)
4. [项目概览](#4-项目概览)
5. [开发约定与规范（必读）](#5-开发约定与规范必读)
6. [开发路线图](#6-开发路线图)
7. [第一阶段：项目初始化和基础构建](#7-第一阶段项目初始化和基础构建)
8. [第二阶段：核心基础设施构建](#8-第二阶段核心基础设施构建)
9. [第三阶段：独立库开发与完善](#9-第三阶段独立库开发与完善)
10. [第四阶段：模块系统完善](#10-第四阶段模块系统完善)
11. [第五阶段：业务模块开发](#11-第五阶段业务模块开发)
12. [调试技巧](#12-调试技巧)
13. [最佳实践与经验教训](#13-最佳实践与经验教训)
14. [任务完成标准流程](#14-任务完成标准流程)
15. [常见问题 FAQ](#15-常见问题-faq)
16. [技术决策记录](#16-技术决策记录)
17. [整体进度追踪表](#17-整体进度追踪表)

---

## 1. 前言：为什么这份指南更完美？

### 1.1 我们的优势

这份指南不是凭空写的——它是基于**第一次完整开发的经验教训**编写的。我们知道：

✅ **哪些架构决策是正确的**  
✅ **哪些坑需要避免**  
✅ **哪些模块应该优先开发**  
✅ **如何让构建系统更统一**  
✅ **如何让测试更完善**  

### 1.2 核心改进理念

| 方面 | 第一次开发 | 这次开发（更完美） |
|-----|-----------|------------------|
| **构建系统** | 后期才统一 | **从第一天就用 idcu-module-build** |
| **测试** | 很多模块测试滞后 | **TDD，先写测试** |
| **依赖关系** | 有些混乱 | **更严格的分层架构** |
| **YAML 支持** | 没有规划 | **从一开始就加入** |
| **API 设计** | 经过多次迭代 | **一次设计到位** |

### 1.3 推荐策略：混合开发

不是所有代码都要重写！建议采用混合策略：

- **核心模块**（微内核、模块系统）→ 重新设计，基于经验优化
- **成熟库**（idcu-log、idcu-json）→ 直接参考或复用
- **业务模块** → 逐步实现，业务逻辑稳定

---

## 2. 开发前准备检查清单

在开始任何代码之前，请确认你已经完成了以下准备工作：

- [ ] **开发环境已搭建**
  - [ ] CMake 3.14+ 已安装
  - [ ] 编译器已安装（GCC 7.0+ / Clang 6.0+ / MSVC 2017+）
  - [ ] Git 已安装并配置好
  - [ ] Clang-Format 和 Clang-Tidy 已安装
- [ ] **已阅读相关文档**
  - [ ] 阅读了 [coding_standards.md](./coding_standards.md)（编码规范）
  - [ ] 阅读了 [development_workflow.md](./development_workflow.md)（开发工作流）
  - [ ] 了解了微内核架构概念
- [ ] **已有项目参考准备**
  - [ ] 克隆了现有项目作为参考
  - [ ] 浏览了现有代码结构
  - [ ] 明确了重开发的目标（完全重写 vs 混合策略）
- [ ] **Git 仓库准备**
  - [ ] 创建了新的开发分支
  - [ ] 配置了 .gitignore
  - [ ] 设置了 Git 用户信息

---

## 3. 如何参考现有代码

因为你已经有一个完整的项目，这里告诉你如何高效地参考：

### 3.1 可以直接参考/复用的模块

| 模块 | 建议 | 理由 |
|-----|------|------|
| **idcu-log** | ✅ 直接复用 | 已经很成熟，无需重写 |
| **idcu-json** | ✅ 直接复用 | 已经很成熟，无需重写 |
| **idcu-network** | ✅ 参考实现 | 网络层已验证可靠 |
| **idcu-http-server** | ✅ 参考实现 | HTTP 服务器已稳定 |
| **idcu-metrics** | ✅ 参考实现 | 指标收集逻辑清晰 |
| **idcu-healthcheck** | ✅ 参考实现 | 健康检查逻辑简单 |

### 3.2 建议重新设计的模块

| 模块 | 建议 | 理由 |
|-----|------|------|
| **idcu-common** | 🔄 重新设计 | 可以更精简，只保留真正需要的 |
| **模块系统** | 🔄 重新设计 | 可以更灵活，依赖解析更智能 |
| **微内核** | 🔄 重新设计 | 整合经验，一次设计到位 |
| **SDK** | 🔄 重新设计 | 从一开始就设计好接口 |

### 3.3 在新旧代码之间切换的技巧

```bash
# 1. 在现有项目目录中，创建新分支
git checkout -b feature/redevelopment-v2

# 2. 或者，克隆到新目录，同时保留旧代码作为参考
cd ..
git clone idcu-agent idcu-agent-v2
cd idcu-agent-v2

# 3. 查看旧代码作为参考
# 在另一个终端窗口中打开旧代码
# 或使用 IDE 的分屏功能
```

---

## 4. 项目概览

### 4.1 什么是 IDCU Agent？

IDCU Agent 是一个基于**微内核架构**的实时代理程序，使用纯 C 语言编写。它的设计理念是：**核心很小，功能通过添加模块来实现**。

### 4.2 技术栈

- **编程语言**: C99
- **构建工具**: CMake 3.14+
- **编译器**: GCC 7.0+ / Clang 6.0+ / MSVC 2017+
- **代码质量**: Clang-Format, Clang-Tidy
- **测试框架**: 自研测试框架

### 4.3 目标项目架构（更清晰的分层）

```
┌─────────────────────────────────────────────────┐
│           业务层 (Business Layer)              │
├─────────────────────────────────────────────────┤
│          集成层 (Integration Layer)            │
├─────────────────────────────────────────────────┤
│            SDK层 (SDK Layer)                   │
├─────────────────────────────────────────────────┤
│          核心层 (Core Layer)                   │
│  ┌─────────────────────────────────────────┐  │
│  │  微内核 (Micro Kernel)                  │  │
│  ├─────────────────────────────────────────┤  │
│  │  模块系统 (Module System)               │  │
│  ├─────────────────────────────────────────┤  │
│  │  调度器 (Scheduler)                     │  │
│  └─────────────────────────────────────────┘  │
├─────────────────────────────────────────────────┤
│        基础库层 (Library Layer)               │
└─────────────────────────────────────────────────┘
```

### 4.4 严格的依赖规则（必须遵守！）

1. **上层可以依赖下层**
2. **下层绝对不能依赖上层**
3. **同层之间尽量减少依赖**
4. **所有跨层依赖必须通过明确的接口**

---

## 5. 开发约定与规范（必读）

在开始任何开发工作之前，请务必遵守以下约定。

### 5.1 编码规范

详细的编码规范请参考 [coding_standards.md](./coding_standards.md)，核心要点：

1. **语言标准**: C99
2. **代码风格**: 通过 `.clang-format` 自动格式化（Google 风格变体）
3. **命名规范**:
   - 函数: `idcu_prefix_action_object()`
   - 类型: `idcu_TypeName_t`
   - 宏: `IDCU_MACRO_NAME`
   - 变量: `snake_case`
4. **错误处理**: 使用统一的 `idcu_ErrorCode`
5. **内存管理**: 优先使用内存池

### 5.2 Git 提交规范

#### 5.2.1 分支策略

采用 Git Flow 风格的分支策略：

- **main**: 稳定的生产版本
- **develop**: 开发集成分支
- **feature/xxx**: 功能分支（从 develop 创建，合并回 develop）
- **hotfix/xxx**: 热修复分支（从 main 创建，合并回 main 和 develop）
- **release/vx.y.z**: 发布分支

#### 5.2.2 Commit Message 格式

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Type 类型**:
- `feat`: 新功能
- `fix`: 修复 bug
- `docs`: 文档更新
- `style`: 代码格式调整（不影响代码运行）
- `refactor`: 重构
- `perf`: 性能优化
- `test`: 测试相关
- `chore`: 构建/工具链相关

**示例**:
```
feat(libs/idcu-log): add file rotation support

- Add log file rotation based on size
- Add log file rotation based on time
- Update documentation

Closes #123
```

#### 5.2.3 提交前检查清单

每次提交前必须完成：

- [ ] 代码已格式化（运行 `scripts/format.bat` 或 `scripts/format.sh`）
- [ ] 代码可以正常编译通过
- [ ] 所有相关测试通过
- [ ] 没有新增的编译警告
- [ ] 静态分析（clang-tidy）没有发现严重问题
- [ ] 更新了相关文档（如需要）
- [ ] 添加了单元测试（如需要）

### 5.3 测试策略（从经验中学到的）

**核心原则**：
- 核心模块 **100%** 覆盖
- 基础库 **> 90%** 覆盖
- 业务模块 **> 70%** 覆盖

**测试工具**：
- 自研测试框架
- Coverage 统计
- Fuzzing 测试（关键模块）

**开发方式**：测试驱动开发（TDD）- 先写测试，再写代码！

### 5.4 开发工作流

详细的开发工作流请参考 [development_workflow.md](./development_workflow.md)。

---

## 6. 开发路线图

我们将采用**迭代式开发**方法，分 5 个阶段逐步构建项目，每个阶段再细分为具体的模块开发步骤。

### 6.1 关键决策：idcu-module-build 和 idcu-yaml

#### idcu-module-build - 必须前期完成！⭐⭐⭐⭐⭐

**为什么？**
1. 可以显著简化后续模块的构建过程
2. 配置驱动的构建方式可以减少重复的 CMake 代码
3. 支持自动适配，有利于跨平台开发
4. 可以在开发过程中持续使用，提高开发效率
5. **第一次开发后期才统一，这次我们从第一天就用！**

**计划**：阶段 1 就完成 idcu-module-build，所有后续模块都使用它。

#### idcu-yaml - 必须规划进去！⭐⭐⭐⭐

**为什么？**
1. YAML 是现代配置文件的事实标准，比 JSON 更易读易写
2. 可以与 idcu-json 配合使用
3. 许多现代工具（如 Kubernetes、Ansible）都使用 YAML
4. 为未来的配置管理提供更多选择
5. **第一次开发没有规划，这次我们补上！**

**计划**：阶段 3 中添加 idcu-yaml 模块，放在 idcu-json 之后。

### 6.2 阶段概览

| 阶段 | 目标 | 预计工作量 | 关键产出 | 里程碑演示 |
|-----|------|-----------|---------|-----------|
| **阶段 1** | 项目初始化和基础构建 | 2-3 天 | 可运行的 Hello World + CMake + **idcu-module-build（优先！）** + 测试框架 | Hello World + 构建系统工作 |
| **阶段 2** | 核心基础设施构建 | 5-7 天 | 微内核 + 模块系统 + 调度器 + SDK | 可以加载和运行简单模块 |
| **阶段 3** | 独立库开发与完善 | 7-10 天 | 20+ 个可独立使用的库（含 **idcu-yaml**） | 可以使用日志、配置等基础库 |
| **阶段 4** | 模块系统完善 | 3-5 天 | 集成层 + 完整的 SDK | 可以通过 SDK 开发模块 |
| **阶段 5** | 业务模块开发 | 7-10 天 | 完整的业务功能模块 | 完整的业务功能 |

---

### 6.3 详细开发顺序（按依赖关系）

#### **阶段 1：项目初始化和基础构建**

| 步骤 | 任务 | 依赖 | 说明 | 经验提示 | 风险提示 |
|-----|------|------|------|---------|---------|
| 1.1 | 创建项目目录结构 | 无 | 创建标准的项目目录 | 参考现有结构，但更精简 | 注意目录命名一致性 |
| 1.2 | 编写主程序入口 | 无 | app/main.c - Hello World | 保持简单 | 无 |
| 1.3 | 配置 CMake 构建系统 | 无 | 根目录 CMakeLists.txt | 为 idcu-module-build 预留接口 | CMake 跨平台配置可能有坑 |
| 1.4 | 配置代码质量工具 | 无 | .clang-format, .clang-tidy | 从第一天就启用！ | 不同版本的 clang-format 行为可能不同 |
| 1.5 | **搭建测试框架** | 无 | 自研测试框架 | **TDD 从这里开始！** | 测试框架设计要简单易用 |
| 1.6 | **idcu-module-build 初始化** | 1.3 | 创建模块构建工具基础 | **优先完成！** | 这是最重要的一步，多花时间设计 |
| 1.7 | 验证项目可以编译 | 1.1-1.6 | 确保构建系统正常工作 | | 在多个平台上测试 |

#### **阶段 2：核心基础设施构建**

| 步骤 | 模块/组件 | 依赖 | 说明 | 经验提示 | 风险提示 |
|-----|----------|------|------|---------|---------|
| 2.1 | idcu-common | 无 | 最基础的通用组件（错误码、数据结构、锁、原子操作） | 更精简，只保留真正需要的 | 不要过度设计，够用就好 |
| 2.2 | **完善 idcu-module-build** | 2.1 | 集成 idcu-common，完善构建功能 | 所有后续模块都用它 | 确保 API 稳定，避免后续重构 |
| 2.3 | 模块系统 (module-system) | 2.1, 2.2 | 模块注册、依赖管理、生命周期管理 | 更灵活的依赖解析 | 循环依赖检测要做好 |
| 2.4 | 协程调度器 (scheduler/coroutine) | 2.1 | 协程上下文切换、调度器 | 性能优先 | 协程切换的性能可能是瓶颈 |
| 2.5 | 消息总线 (scheduler/msg_bus) | 2.1 | 消息收发、优先级队列 | 更可靠的消息传递 | 消息丢失是大问题 |
| 2.6 | 微内核核心 (micro-kernel) | 2.3 + 2.4 + 2.5 | 整合所有核心组件 | 一次设计到位 | 整合时要注意组件之间的耦合 |
| 2.7 | SDK 基础 (sdk) | 2.6 + 2.1 | 模块开发基础接口 | **从一开始就设计好！** | SDK API 要稳定，避免后期改动 |

#### **阶段 3：独立库开发与完善**

##### **基础工具层**

| 步骤 | 库名 | 依赖 | 功能 | 经验提示 | 风险提示 |
|-----|------|------|------|---------|---------|
| 3.1 | idcu-log | 2.1 | 日志系统（多级别、文件输出） | 参考现有实现，已很成熟 | 直接复用现有代码 |
| 3.2 | idcu-json | 2.1 | JSON 解析与序列化 | 参考现有实现，已很成熟 | 直接复用现有代码 |
| 3.3 | **idcu-yaml** | 2.1, 3.2 | YAML 解析与序列化 | **新增！先实现简化版** | 先做简化版，够用就好 |
| 3.4 | idcu-memory | 2.1 | 内存池管理 | 性能优化 | 内存泄漏检测要做好 |
| 3.5 | idcu-testframework | 2.1 | 单元测试框架 | 与阶段 1.5 配合 | 保持简单，不要过度设计 |

##### **配置与存储层**

| 步骤 | 库名 | 依赖 | 功能 | 经验提示 |
|-----|------|------|------|---------|
| 3.6 | idcu-config | 2.1 + 3.2 + 3.3 | 配置管理（热重载、多环境，**支持 JSON 和 YAML**） | 支持双格式！ |
| 3.7 | idcu-storage | 2.1 | 持久化存储接口 | |
| 3.8 | idcu-cache | 2.1 | 内存缓存 | |

##### **网络与通信层**

| 步骤 | 库名 | 依赖 | 功能 | 经验提示 |
|-----|------|------|------|---------|
| 3.9 | idcu-network | 2.1 + 3.1 | 网络层（TCP/UDP、Socket 封装） | 参考现有实现 |
| 3.10 | idcu-conn-pool | 2.1 + 3.9 | 连接池管理 | |
| 3.11 | idcu-http-server | 2.1 + 3.1 + 3.9 | HTTP 服务器 | 参考现有实现 |
| 3.12 | idcu-http-client | 2.1 + 3.1 + 3.9 | HTTP 客户端 | 参考现有实现 |
| 3.13 | idcu-msgbus | 2.1 | 独立消息总线库 | |
| 3.14 | idcu-coroutine | 2.1 | 独立协程库 | |

##### **监控与服务层**

| 步骤 | 库名 | 依赖 | 功能 | 经验提示 |
|-----|------|------|------|---------|
| 3.15 | idcu-metrics | 2.1 + 3.1 | 指标收集（Counter、Gauge、Histogram） | 参考现有实现 |
| 3.16 | idcu-healthcheck | 2.1 + 3.1 | 健康检查 | 参考现有实现 |
| 3.17 | idcu-alert | 2.1 + 3.1 | 告警管理（规则、通知） | |
| 3.18 | idcu-watchdog | 2.1 | 看门狗定时器 | |
| 3.19 | idcu-discovery | 2.1 + 3.9 | 节点发现 | |

##### **安全与插件层**

| 步骤 | 库名 | 依赖 | 功能 |
|-----|------|------|------|
| 3.20 | idcu-sandbox | 2.1 + 3.1 | 沙箱安全隔离 |
| 3.21 | idcu-permission | 2.1 | 权限管理 |
| 3.22 | idcu-plugin | 2.1 + 3.1 | 插件加载系统 |
| 3.23 | idcu-management | 2.1 + 3.11 | 管理 CLI 和 API |

##### **高级功能层**

| 步骤 | 库名 | 依赖 | 功能 |
|-----|------|------|------|
| 3.24 | idcu-distributed | 2.1 + 3.9 + 3.19 | 分布式节点支持 |
| 3.25 | idcu-scheduler | 2.1 + 3.14 | 任务调度器 |
| 3.26 | idcu-device-collector | 2.1 + 3.9 | 设备数据采集 |
| 3.27 | idcu-server-monitor | 2.1 + 3.15 | 服务器监控 |
| 3.28 | idcu-module-isolation | 2.1 + 3.20 | 模块隔离 |
| 3.29 | idcu-module-verifier | 2.1 | 模块验证 |

#### **阶段 4：模块系统完善**

| 步骤 | 组件 | 依赖 | 功能 |
|-----|------|------|------|
| 4.1 | log-integration | 3.1 + SDK | 日志系统集成模块 |
| 4.2 | config-integration | 3.6 + SDK | 配置系统集成模块 |
| 4.3 | json-integration | 3.2 + SDK | JSON 解析集成模块 |
| 4.4 | **yaml-integration** | 3.3 + SDK | YAML 解析集成模块 |
| 4.5 | network-integration | 3.9 + SDK | 网络层集成模块 |
| 4.6 | metrics-integration | 3.15 + SDK | 指标收集集成模块 |
| 4.7 | basic-libs | 多个基础库 + SDK | 基础库统一集成 |
| 4.8 | SDK 完善 | 微内核 | 完整的 SDK 接口封装 |

#### **阶段 5：业务模块开发**

| 步骤 | 业务模块 | 依赖 | 功能 |
|-----|---------|------|------|
| 5.1 | core-module | SDK + 基础集成 | 核心基础模块 |
| 5.2 | log-module | SDK + 3.1 | 日志业务模块 |
| 5.3 | config-module | SDK + 3.6 | 配置业务模块 |
| 5.4 | heartbeat | SDK | 心跳模块 |
| 5.5 | metrics-module | SDK + 3.15 | 指标业务模块 |
| 5.6 | healthcheck-module | SDK + 3.16 | 健康检查业务模块 |
| 5.7 | alert-module | SDK + 3.17 | 告警业务模块 |
| 5.8 | collect-module | SDK + 3.26 | 数据采集业务模块 |
| 5.9 | cache-module | SDK + 3.8 | 缓存业务模块 |
| 5.10 | storage-module | SDK + 3.7 | 存储业务模块 |
| 5.11 | security-module | SDK + 3.20 + 3.21 | 安全业务模块 |
| 5.12 | http-client-module | SDK + 3.12 | HTTP 客户端业务模块 |
| 5.13 | http-management | SDK + 3.23 | HTTP 管理业务模块 |

---

### 6.4 依赖关系图

```
项目初始化
    ↓
idcu-module-build (优先！)
    ↓
idcu-common (基础)
    ↓
    ├→ idcu-log ─┬→ idcu-network ─→ idcu-http-server
    │            │                  ↓
    │            │            idcu-http-client
    │            │
    ├→ idcu-json ─→ idcu-yaml ─→ idcu-config
    │
    ├→ idcu-memory
    │
    ├→ 模块系统 ─┬→ 协程调度器
    │            ├→ 消息总线
    │            ↓
    │         微内核 ─→ SDK
    │
    ├→ idcu-metrics ─→ idcu-healthcheck
    │
    ├→ idcu-alert
    │
    ├→ idcu-sandbox ─→ idcu-permission
    │
    └→ (其他独立库...)
         ↓
      集成层
         ↓
      业务模块
```

---

## 7. 第一阶段：项目初始化和基础构建

### 7.1 阶段里程碑

完成本阶段后，你应该能够：
- ✅ 运行 Hello World 程序
- ✅ 使用 CMake 成功构建项目
- ✅ 使用 idcu-module-build 创建新模块
- ✅ 运行简单的测试

### 7.2 步骤 1: 创建项目结构（详细版）

#### 7.2.1 目标
创建完整的、可扩展的项目目录结构，为后续所有模块开发打下基础。

#### 7.2.2 详细步骤

**1. 创建根目录**
```bash
# 如果还没有项目目录，先创建
mkdir -p idcu-agent
cd idcu-agent

# 初始化 Git 仓库（如果还没有）
git init
```

**2. 创建完整的目录结构**
```bash
# 应用程序目录
mkdir -p app

# 配置文件目录
mkdir -p config
mkdir -p config/default

# 文档目录
mkdir -p docs
mkdir -p docs/api
mkdir -p docs/design

# 脚本目录
mkdir -p scripts
mkdir -p scripts/windows
mkdir -p scripts/linux

# 库目录
mkdir -p libs
mkdir -p libs/idcu-module-build
mkdir -p libs/idcu-module-build/include
mkdir -p libs/idcu-module-build/src
mkdir -p libs/idcu-module-build/tests

# 模块目录
mkdir -p modules
mkdir -p modules/core
mkdir -p modules/integrations
mkdir -p modules/business

# 测试目录
mkdir -p tests
mkdir -p tests/unit
mkdir -p tests/integration
mkdir -p tests/benchmarks
mkdir -p tests/data

# 构建输出目录（会被 .gitignore 忽略）
mkdir -p build
mkdir -p out
```

**3. 创建初始文件**
```bash
# 创建 .gitignore
cat > .gitignore << 'EOF'
# Build directories
build/
out/
bin/
lib/

# IDE files
.idea/
.vscode/
*.swp
*.swo
*~

# OS files
.DS_Store
Thumbs.db

# Temporary files
*.tmp
*.bak
*.old

# Log files
*.log
logs/
EOF

# 创建 README.md（项目总览）
cat > README.md << 'EOF'
# IDCU Agent

基于微内核架构的实时代理程序。

## 快速开始

详见 [docs/step_by_step_development_guide.md](docs/step_by_step_development_guide.md)

## 项目结构

```
idcu-agent/
├── app/                 # 应用程序入口
├── config/              # 配置文件
├── docs/                # 文档
├── libs/                # 独立库
├── modules/             # 模块
├── scripts/             # 构建脚本
└── tests/               # 测试
```
EOF

# 创建空的 CMakeLists.txt（稍后填充）
touch CMakeLists.txt
```

#### 7.2.3 验证步骤
- [ ] 所有目录都已创建
- [ ] .gitignore 已创建
- [ ] README.md 已创建
- [ ] 可以使用 `tree -L 2` 查看目录结构（Linux）或 `dir /s`（Windows）

#### 7.2.4 Git 提交
```bash
git add .
git commit -m "chore: initialize project structure

- Create complete directory structure
- Add .gitignore
- Add initial README.md"
```

---

### 7.3 步骤 2: 编写主程序入口（详细版）

#### 7.3.1 目标
创建简单但可扩展的主程序入口，为后续集成微内核做准备。

#### 7.3.2 详细步骤

**1. 创建主程序文件**
```c
// app/main.c
#include <stdio.h>
#include <stdlib.h>

// 版本信息
#define IDCU_AGENT_VERSION_MAJOR 0
#define IDCU_AGENT_VERSION_MINOR 1
#define IDCU_AGENT_VERSION_PATCH 0

static void print_version(void) {
    printf("IDCU Agent v%d.%d.%d\n",
           IDCU_AGENT_VERSION_MAJOR,
           IDCU_AGENT_VERSION_MINOR,
           IDCU_AGENT_VERSION_PATCH);
}

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --version  Show version information\n");
    printf("\n");
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) {
            print_version();
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Unknown option: %s\n", arg);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    // 正常启动
    printf("========================================\n");
    print_version();
    printf("========================================\n");
    printf("Hello, IDCU Agent!\n");
    printf("System is starting...\n");

    // TODO: 这里将来会集成微内核
    // 目前只是简单的 Hello World

    printf("System started successfully!\n");
    printf("Press Ctrl+C to exit...\n");

    // 简单的等待循环
    while (1) {
        // 什么都不做，等待用户中断
    }

    return EXIT_SUCCESS;
}
```

**2. 创建头文件（版本信息）**
```bash
mkdir -p app/include
```

```c
// app/include/version.h
#ifndef IDCU_APP_VERSION_H
#define IDCU_APP_VERSION_H

#define IDCU_AGENT_VERSION_MAJOR 0
#define IDCU_AGENT_VERSION_MINOR 1
#define IDCU_AGENT_VERSION_PATCH 0

#define IDCU_AGENT_VERSION_STRING "0.1.0"

const char* idcu_agent_get_version(void);

#endif
```

```c
// app/src/version.c
#include "version.h"

const char* idcu_agent_get_version(void) {
    return IDCU_AGENT_VERSION_STRING;
}
```

#### 7.3.3 验证步骤
- [ ] 代码已创建
- [ ] 语法检查通过（可以用编译器检查）
- [ ] 代码已通过 clang-format 格式化

#### 7.3.4 Git 提交（任务完成后）
```bash
git add app/
git commit -m "feat(app): add main program entry

- Add main.c with command line parsing
- Add version information
- Add help message"
```

---

### 7.4 步骤 3: 创建 CMake 构建配置（详细版）

#### 7.4.1 目标
创建灵活、可扩展的 CMake 构建系统，为 idcu-module-build 预留接口。

#### 7.4.2 详细步骤

**1. 创建根目录 CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.14)
project(idcu_agent C)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 编译选项
if(MSVC)
    # MSVC 特定选项
    add_compile_options(/W4 /WX /utf-8)
    add_compile_options(/MP)  # 多核编译
else()
    # GCC/Clang 通用选项
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_options(-Werror)  # 警告当作错误
    
    # 调试构建选项
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
        add_compile_definitions(DEBUG=1)
    endif()
    
    # 发布构建选项
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
        add_compile_definitions(NDEBUG=1)
    endif()
endif()

# 选项：是否构建测试
option(BUILD_TESTS "Build unit tests" ON)

# 选项：是否构建示例
option(BUILD_EXAMPLES "Build example programs" ON)

# 添加 idcu-module-build 模块
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/libs/idcu-module-build/cmake")

# 包含子目录
add_subdirectory(libs)
add_subdirectory(app)

# 测试（如果启用）
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 打印配置摘要
message(STATUS "")
message(STATUS "========================================")
message(STATUS "IDCU Agent Configuration Summary")
message(STATUS "========================================")
message(STATUS "  Version: ${IDCU_AGENT_VERSION_STRING}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "========================================")
message(STATUS "")
```

**2. 创建 app/CMakeLists.txt**
```cmake
# app/CMakeLists.txt

# 主程序
add_executable(idcu_agent
    main.c
    src/version.c
)

target_include_directories(idcu_agent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 安装规则
install(TARGETS idcu_agent
    RUNTIME DESTINATION bin
)
```

**3. 创建 libs/CMakeLists.txt**
```cmake
# libs/CMakeLists.txt

# idcu-module-build 是一个特殊的模块，先添加它
add_subdirectory(idcu-module-build)

# 其他库将在后续阶段添加
# add_subdirectory(idcu-common)
# add_subdirectory(idcu-log)
# ...
```

**4. 创建 tests/CMakeLists.txt**
```cmake
# tests/CMakeLists.txt
# 测试框架将在后续阶段添加
message(STATUS "Tests will be added in later stages")
```

#### 7.4.3 验证步骤
- [ ] CMakeLists.txt 语法正确
- [ ] 可以运行 `cmake -B build` 不报错
- [ ] 可以看到配置摘要输出

#### 7.4.4 Git 提交
```bash
git add CMakeLists.txt
git add app/CMakeLists.txt
git add libs/CMakeLists.txt
git add tests/CMakeLists.txt
git commit -m "chore(build): add CMake build system

- Add root CMakeLists.txt with configuration options
- Add app CMakeLists.txt
- Add libs CMakeLists.txt
- Add tests CMakeLists.txt
- Add MSVC and GCC/Clang compiler options"
```

---

### 7.5 步骤 4: 验证项目可以编译（详细版）

#### 7.5.1 目标
确保 CMake 构建系统可以正常工作，在多个平台上验证。

#### 7.5.2 详细步骤

**1. Windows (MinGW)**
```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ..

# 编译
mingw32-make -j4

# 运行程序
./bin/idcu_agent.exe

# 测试命令行参数
./bin/idcu_agent.exe --help
./bin/idcu_agent.exe --version
```

**2. Windows (MSVC)**
```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake（Visual Studio 2019）
cmake -G "Visual Studio 16 2019" -A x64 ..

# 编译 Debug 版本
cmake --build . --config Debug

# 运行程序
./Debug/bin/idcu_agent.exe
```

**3. Linux**
```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake
cmake -DCMAKE_BUILD_TYPE=Debug ..

# 编译
make -j4

# 运行程序
./bin/idcu_agent

# 测试命令行参数
./bin/idcu_agent --help
./bin/idcu_agent --version
```

**4. 清理并重新构建（验证）**
```bash
# 清理
cd ..
rm -rf build

# 重新配置和编译
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4

# 验证 Release 版本
./build/bin/idcu_agent --version
```

#### 7.5.3 验证检查清单
- [ ] Debug 版本可以正常编译
- [ ] Release 版本可以正常编译
- [ ] 程序可以正常运行
- [ ] `--help` 参数工作正常
- [ ] `--version` 参数工作正常
- [ ] 可以正常退出（Ctrl+C）

#### 7.5.4 常见问题排查
| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| CMake 找不到编译器 | 编译器未安装或不在 PATH 中 | 安装编译器并检查 PATH |
| 链接错误 | 缺少依赖库 | 检查依赖是否正确安装 |
| 中文乱码 | 字符编码问题 | 确保源文件是 UTF-8 编码 |

---

### 7.6 步骤 5: 添加代码质量工具（详细版）

#### 7.6.1 目标
配置代码格式化和静态分析工具，从第一天就保证代码质量。

#### 7.6.2 详细步骤

**1. 创建 .clang-format（更详细的配置）**
```yaml
---
Language: Cpp
BasedOnStyle: Google
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 100

# 指针和引用对齐
PointerAlignment: Left

# 括号风格
BreakBeforeBraces: Attach

# 空行
MaxEmptyLinesToKeep: 2
KeepEmptyLinesAtTheStartOfBlocks: false

# 包含排序
IncludeBlocks: Regroup
IncludeCategories:
  - Regex: '^<.*\.h>'
    Priority: 1
  - Regex: '^".*'
    Priority: 2

# 其他设置
SpaceBeforeParens: ControlStatements
SpaceInEmptyParentheses: false
SpacesInAngles: false
SpacesInCStyleCastParentheses: false
SpacesInContainerLiterals: true
SpacesInParentheses: false
SpacesInSquareBrackets: false
...
```

**2. 创建 .clang-tidy（更详细的配置）**
```yaml
---
Checks: >
  -*,
  clang-analyzer-*,
  performance-*,
  modernize-*,
  readability-*,
  -modernize-use-trailing-return-type,
  -readability-magic-numbers

WarningsAsErrors: ''
HeaderFilterRegex: ''
FormatStyle: file

CheckOptions:
  - key: readability-identifier-naming.FunctionCase
    value: lower_case
  - key: readability-identifier-naming.VariableCase
    value: lower_case
  - key: readability-identifier-naming.ParameterCase
    value: lower_case
  - key: readability-identifier-naming.MemberCase
    value: lower_case
  - key: readability-identifier-naming.GlobalConstantCase
    value: UPPER_CASE
  - key: readability-identifier-naming.EnumConstantCase
    value: UPPER_CASE
  - key: readability-identifier-naming.TypeAliasCase
    value: CamelCase
  - key: readability-identifier-naming.TypedefCase
    value: CamelCase
...
```

**3. 创建格式化脚本**

**Windows (scripts/windows/format.bat)**
```batch
@echo off
echo Formatting C/C++ files...

REM 格式化所有 .c 和 .h 文件
for /r %%f in (*.c *.h) do (
    echo Formatting: %%f
    clang-format -i "%%f"
)

echo Formatting complete!
```

**Linux (scripts/linux/format.sh)**
```bash
#!/bin/bash

echo "Formatting C/C++ files..."

# 格式化所有 .c 和 .h 文件
find . -name "*.c" -o -name "*.h" | while read -r file; do
    echo "Formatting: $file"
    clang-format -i "$file"
done

echo "Formatting complete!"
```

**4. 创建格式检查脚本**

**Windows (scripts/windows/check_format.bat)**
```batch
@echo off
echo Checking code format...

set HAS_ERRORS=0

REM 检查所有 .c 和 .h 文件
for /r %%f in (*.c *.h) do (
    clang-format --dry-run --Werror "%%f" >nul 2>&1
    if errorlevel 1 (
        echo File needs formatting: %%f
        set HAS_ERRORS=1
    )
)

if %HAS_ERRORS% equ 0 (
    echo All files are properly formatted!
) else (
    echo Some files need formatting!
    exit /b 1
)
```

**Linux (scripts/linux/check_format.sh)**
```bash
#!/bin/bash

echo "Checking code format..."

HAS_ERRORS=0

# 检查所有 .c 和 .h 文件
find . -name "*.c" -o -name "*.h" | while read -r file; do
    if ! clang-format --dry-run --Werror "$file" > /dev/null 2>&1; then
        echo "File needs formatting: $file"
        HAS_ERRORS=1
    fi
done

if [ "$HAS_ERRORS" -eq 0 ]; then
    echo "All files are properly formatted!"
else
    echo "Some files need formatting!"
    exit 1
fi
```

**5. 给脚本添加执行权限（Linux）**
```bash
chmod +x scripts/linux/format.sh
chmod +x scripts/linux/check_format.sh
```

**6. 运行格式化**
```bash
# Windows
scripts/windows/format.bat

# Linux
scripts/linux/format.sh
```

#### 7.6.3 验证检查清单
- [ ] .clang-format 已创建
- [ ] .clang-tidy 已创建
- [ ] 格式化脚本已创建
- [ ] 检查脚本已创建
- [ ] 可以运行格式化脚本
- [ ] 可以运行检查脚本
- [ ] 现有代码已格式化

#### 7.6.4 Git 提交
```bash
git add .clang-format
git add .clang-tidy
git add scripts/
git commit -m "chore: add code quality tools

- Add .clang-format configuration
- Add .clang-tidy configuration
- Add format scripts for Windows and Linux
- Add check-format scripts for Windows and Linux"

# 格式化现有代码并提交
scripts/windows/format.bat  # 或 scripts/linux/format.sh
git add -u
git commit -m "style: format existing code with clang-format"
```

---

### 7.7 步骤 6: 搭建测试框架（详细版 - TDD 从这里开始！）

#### 7.7.1 目标
创建轻量级、易用的测试框架，支持 TDD（测试驱动开发）。

#### 7.7.2 详细步骤

**1. 创建测试框架目录结构**
```bash
mkdir -p libs/idcu-testframework/include/idcu/testframework
mkdir -p libs/idcu-testframework/src
mkdir -p libs/idcu-testframework/tests
mkdir -p libs/idcu-testframework/examples
```

**2. 创建测试框架头文件**
```c
// libs/idcu-testframework/include/idcu/testframework/testframework.h
#ifndef IDCU_TESTFRAMEWORK_TESTFRAMEWORK_H
#define IDCU_TESTFRAMEWORK_TESTFRAMEWORK_H

#include <stdio.h>
#include <stdbool.h>

// 测试结果统计
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} idcu_TestStats;

// 测试函数类型
typedef void (*idcu_TestFunc)(void);

// 测试用例注册
void idcu_test_register(const char* suite_name, const char* test_name, idcu_TestFunc func);

// 断言宏
#define IDCU_TEST_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "ASSERT FAILED: %s:%d - %s\n", __FILE__, __LINE__, #cond); \
            idcu_test_fail_current(); \
            return; \
        } \
    } while (0)

#define IDCU_TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            fprintf(stderr, "ASSERT FAILED: %s:%d - %s != %s (expected %d, got %d)\n", \
                    __FILE__, __LINE__, #expected, #actual, (int)(expected), (int)(actual)); \
            idcu_test_fail_current(); \
            return; \
        } \
    } while (0)

#define IDCU_TEST_ASSERT_STRING_EQUAL(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            fprintf(stderr, "ASSERT FAILED: %s:%d - \"%s\" != \"%s\"\n", \
                    __FILE__, __LINE__, (expected), (actual)); \
            idcu_test_fail_current(); \
            return; \
        } \
    } while (0)

// 测试注册宏
#define IDCU_TEST_CASE(suite, name) \
    static void _test_##suite##_##name(void); \
    static void _register_test_##suite##_##name(void) __attribute__((constructor)); \
    static void _register_test_##suite##_##name(void) { \
        idcu_test_register(#suite, #name, _test_##suite##_##name); \
    } \
    static void _test_##suite##_##name(void)

// 内部函数
void idcu_test_fail_current(void);
int idcu_test_run_all(void);
idcu_TestStats idcu_test_get_stats(void);

#endif
```

**3. 创建测试框架实现**
```c
// libs/idcu-testframework/src/testframework.c
#include "idcu/testframework/testframework.h"
#include <stdlib.h>
#include <string.h>

#define MAX_TESTS 1024

typedef struct {
    const char* suite_name;
    const char* test_name;
    idcu_TestFunc func;
    bool failed;
} idcu_TestCase;

static idcu_TestCase g_tests[MAX_TESTS];
static int g_test_count = 0;
static int g_current_test = -1;
static idcu_TestStats g_stats = {0};

void idcu_test_register(const char* suite_name, const char* test_name, idcu_TestFunc func) {
    if (g_test_count >= MAX_TESTS) {
        fprintf(stderr, "Too many tests! Maximum is %d\n", MAX_TESTS);
        return;
    }
    
    g_tests[g_test_count].suite_name = suite_name;
    g_tests[g_test_count].test_name = test_name;
    g_tests[g_test_count].func = func;
    g_tests[g_test_count].failed = false;
    g_test_count++;
}

void idcu_test_fail_current(void) {
    if (g_current_test >= 0 && g_current_test < g_test_count) {
        g_tests[g_current_test].failed = true;
    }
}

int idcu_test_run_all(void) {
    printf("========================================\n");
    printf("Running %d tests...\n", g_test_count);
    printf("========================================\n\n");
    
    g_stats.total_tests = g_test_count;
    g_stats.passed_tests = 0;
    g_stats.failed_tests = 0;
    
    const char* current_suite = NULL;
    
    for (int i = 0; i < g_test_count; i++) {
        // 打印 suite 名称（如果变化）
        if (current_suite == NULL || strcmp(g_tests[i].suite_name, current_suite) != 0) {
            current_suite = g_tests[i].suite_name;
            printf("[Suite: %s]\n", current_suite);
        }
        
        // 运行测试
        printf("  Running: %s... ", g_tests[i].test_name);
        fflush(stdout);
        
        g_current_test = i;
        g_tests[i].failed = false;
        g_tests[i].func();
        
        if (g_tests[i].failed) {
            printf("FAILED\n");
            g_stats.failed_tests++;
        } else {
            printf("PASSED\n");
            g_stats.passed_tests++;
        }
    }
    
    // 打印总结
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total:  %d\n", g_stats.total_tests);
    printf("  Passed: %d\n", g_stats.passed_tests);
    printf("  Failed: %d\n", g_stats.failed_tests);
    printf("========================================\n");
    
    return g_stats.failed_tests;
}

idcu_TestStats idcu_test_get_stats(void) {
    return g_stats;
}
```

**4. 创建测试框架 CMakeLists.txt**
```cmake
# libs/idcu-testframework/CMakeLists.txt

add_library(idcu_testframework STATIC
    src/testframework.c
)

target_include_directories(idcu_testframework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(idcu::testframework ALIAS idcu_testframework)

# 示例
if(BUILD_EXAMPLES)
    add_executable(example_simple_test examples/simple_test.c)
    target_link_libraries(example_simple_test PRIVATE idcu::testframework)
endif()
```

**5. 创建示例测试**
```c
// libs/idcu-testframework/examples/simple_test.c
#include "idcu/testframework/testframework.h"
#include <string.h>

// 示例函数，我们将测试它
static int add(int a, int b) {
    return a + b;
}

static const char* get_greeting(void) {
    return "Hello, World!";
}

// 测试用例 1
IDCU_TEST_CASE(Math, AddPositiveNumbers) {
    IDCU_TEST_ASSERT_EQUAL(5, add(2, 3));
    IDCU_TEST_ASSERT_EQUAL(10, add(5, 5));
}

// 测试用例 2
IDCU_TEST_CASE(Math, AddNegativeNumbers) {
    IDCU_TEST_ASSERT_EQUAL(-1, add(2, -3));
    IDCU_TEST_ASSERT_EQUAL(-5, add(-2, -3));
}

// 测试用例 3
IDCU_TEST_CASE(String, Greeting) {
    IDCU_TEST_ASSERT_STRING_EQUAL("Hello, World!", get_greeting());
}

// 主函数
int main(void) {
    return idcu_test_run_all();
}
```

**6. 创建测试框架的 README**
```markdown
# idcu-testframework

轻量级 C 语言测试框架，支持 TDD。

## 快速开始

```c
#include "idcu/testframework/testframework.h"

IDCU_TEST_CASE(MySuite, MyTest) {
    IDCU_TEST_ASSERT_EQUAL(5, 2 + 3);
}

int main(void) {
    return idcu_test_run_all();
}
```

## API 参考

### 断言

- `IDCU_TEST_ASSERT(cond)` - 断言条件为真
- `IDCU_TEST_ASSERT_EQUAL(expected, actual)` - 断言两个值相等
- `IDCU_TEST_ASSERT_STRING_EQUAL(expected, actual)` - 断言两个字符串相等

### 测试注册

- `IDCU_TEST_CASE(suite, name)` - 定义一个测试用例

### 运行测试

- `idcu_test_run_all()` - 运行所有测试
```

**7. 将测试框架添加到构建中**
编辑 `libs/CMakeLists.txt`，添加：
```cmake
add_subdirectory(idcu-testframework)
```

#### 7.7.3 验证检查清单
- [ ] 测试框架头文件已创建
- [ ] 测试框架实现已创建
- [ ] CMakeLists.txt 已创建
- [ ] 示例测试已创建
- [ ] 可以编译测试框架
- [ ] 可以运行示例测试
- [ ] 示例测试通过

#### 7.7.4 运行测试
```bash
# 构建
cmake -B build -DBUILD_EXAMPLES=ON
cmake --build build

# 运行示例测试
./build/libs/idcu-testframework/example_simple_test
```

#### 7.7.5 Git 提交
```bash
git add libs/idcu-testframework/
git commit -m "feat(libs): add test framework

- Add test framework header
- Add test framework implementation
- Add example tests
- Add CMakeLists.txt
- Add README"

# 更新 libs/CMakeLists.txt
git add libs/CMakeLists.txt
git commit -m "chore: add testframework to build"
```

---

### 7.8 步骤 7: 初始化 idcu-module-build（详细版 - 优先完成！）

#### 7.8.1 目标
创建模块构建工具，这是最重要的一步，后续所有模块都将使用它。

#### 7.8.2 详细步骤

由于这个模块比较复杂，建议直接参考现有项目的实现。这里给出核心结构：

**1. 创建目录结构**
```bash
mkdir -p libs/idcu-module-build/cmake
mkdir -p libs/idcu-module-build/include
mkdir -p libs/idcu-module-build/src
mkdir -p libs/idcu-module-build/tests
mkdir -p libs/idcu-module-build/examples
```

**2. 创建核心 CMake 模块文件**
```cmake
# libs/idcu-module-build/cmake/idcu_module.cmake

# IDCU 模块构建辅助函数
include_guard()

# 创建一个 IDCU 库
function(idcu_add_library name)
    cmake_parse_arguments(IDCU_LIB
        ""
        "VERSION"
        "SOURCES;HEADERS;DEPENDS"
        ${ARGN}
    )
    
    # 创建库
    add_library(${name} STATIC ${IDCU_LIB_SOURCES})
    
    # 设置包含目录
    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    
    # 添加依赖
    if(IDCU_LIB_DEPENDS)
        target_link_libraries(${name} PUBLIC ${IDCU_LIB_DEPENDS})
    endif()
    
    # 创建别名
    add_library(idcu::${name} ALIAS ${name})
    
    message(STATUS "Added library: idcu::${name}")
endfunction()

# 创建一个 IDCU 模块
function(idcu_add_module name)
    cmake_parse_arguments(IDCU_MODULE
        ""
        "VERSION"
        "SOURCES;DEPENDS"
        ${ARGN}
    )
    
    # 创建模块库
    add_library(${name} STATIC ${IDCU_MODULE_SOURCES})
    
    # 设置包含目录
    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    
    # 添加依赖
    if(IDCU_MODULE_DEPENDS)
        target_link_libraries(${name} PUBLIC ${IDCU_MODULE_DEPENDS})
    endif()
    
    message(STATUS "Added module: ${name}")
endfunction()
```

**3. 创建完整的 idcu-module-build**

**重要提示**：由于这个模块的复杂性，**强烈建议直接参考现有项目的完整实现**。现有项目的 `libs/idcu-module-build/` 目录包含：
- 完整的 CMake 模块
- 配置文件支持
- 跨平台支持
- 详细的文档

**4. 验证 idcu-module-build**
```bash
# 确保可以引用这个模块
# 在根 CMakeLists.txt 中已经添加了：
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/libs/idcu-module-build/cmake")
```

#### 7.8.3 验收标准
- [ ] idcu-module-build 目录结构已创建
- [ ] 核心 CMake 模块已创建
- [ ] 可以被其他 CMakeLists.txt 引用
- [ ] 有详细的 README 文档

---

### 7.9 阶段 1 最终验收和交付

#### 7.9.1 完整验收检查清单

在进入阶段 2 之前，请确认：

- [ ] **1.1 - 创建项目结构**
  - [ ] 所有目录已创建
  - [ ] .gitignore 已配置
  - [ ] README.md 已创建
  - [ ] 已提交 Git

- [ ] **1.2 - 编写主程序入口**
  - [ ] main.c 已创建
  - [ ] 支持命令行参数（--help, --version）
  - [ ] 代码已格式化
  - [ ] 已提交 Git

- [ ] **1.3 - 创建 CMake 构建配置**
  - [ ] 根 CMakeLists.txt 已创建
  - [ ] 支持 Debug 和 Release 构建
  - [ ] 有 BUILD_TESTS 和 BUILD_EXAMPLES 选项
  - [ ] 有配置摘要输出
  - [ ] 已提交 Git

- [ ] **1.4 - 验证项目可以编译**
  - [ ] Debug 版本编译成功
  - [ ] Release 版本编译成功
  - [ ] 程序可以正常运行
  - [ ] 命令行参数工作正常
  - [ ] 在至少一个平台上验证过

- [ ] **1.5 - 添加代码质量工具**
  - [ ] .clang-format 已创建
  - [ ] .clang-tidy 已创建
  - [ ] format 脚本已创建
  - [ ] check-format 脚本已创建
  - [ ] 现有代码已格式化
  - [ ] 已提交 Git

- [ ] **1.6 - 搭建测试框架**
  - [ ] 测试框架已创建
  - [ ] 有断言宏
  - [ ] 有测试注册机制
  - [ ] 有示例测试
  - [ ] 示例测试可以运行并通过
  - [ ] 已提交 Git

- [ ] **1.7 - 初始化 idcu-module-build**
  - [ ] 目录结构已创建
  - [ ] 核心 CMake 模块已创建
  - [ ] 有 README 文档
  - [ ] 已提交 Git

#### 7.9.2 阶段 1 完成后的演示

运行以下命令展示成果：

```bash
# 1. 清理并重新构建
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
cmake --build build -j4

# 2. 运行主程序
echo "=== Running main program ==="
./build/bin/idcu_agent --version
./build/bin/idcu_agent --help

# 3. 运行测试框架示例
echo ""
echo "=== Running test framework example ==="
./build/libs/idcu-testframework/example_simple_test

# 4. 检查代码格式
echo ""
echo "=== Checking code format ==="
scripts/windows/check_format.bat  # 或 scripts/linux/check_format.sh

echo ""
echo "========================================"
echo "Phase 1 is COMPLETE! 🎉"
echo "========================================"
```

#### 7.9.3 Git 标签（可选但推荐）

如果想标记阶段 1 完成：

```bash
git tag -a v0.1.0-phase1 -m "Complete Phase 1: Project initialization and basic build"
git push origin v0.1.0-phase1
```

---

**恭喜！你已完成阶段 1！现在可以进入阶段 2 了！** 🚀

---

## 8. 第二阶段：核心基础设施构建

### 8.1 阶段里程碑

完成本阶段后，你应该能够：
- ✅ 加载和注册一个简单模块
- ✅ 通过模块系统初始化和运行模块
- ✅ 使用协程调度器
- ✅ 通过消息总线发送和接收消息
- ✅ 使用 SDK 开发简单模块

### 8.2 步骤 1: 创建通用基础库 (idcu-common)

这是所有其他模块的基础。

#### 8.2.1 创建目录结构

```bash
mkdir -p libs/idcu-common/include/idcu/common
mkdir -p libs/idcu-common/src
mkdir -p libs/idcu-common/tests
```

#### 8.2.2 实现错误码系统

创建 `libs/idcu-common/include/idcu/common/error_code.h`：

```c
#ifndef IDCU_COMMON_ERROR_CODE_H
#define IDCU_COMMON_ERROR_CODE_H

typedef enum {
    IDCU_ERR_SUCCESS     =  0,
    IDCU_ERR_GENERAL     = -1,
    IDCU_ERR_INVALID_PARAM = -2,
    IDCU_ERR_NO_MEMORY   = -3,
    IDCU_ERR_PERM_DENIED = -4,
    IDCU_ERR_NOT_FOUND   = -5,
    IDCU_ERR_ALREADY_EXISTS = -6,
} idcu_ErrorCode;

const char* idcu_error_string(idcu_ErrorCode code);

#endif
```

创建 `libs/idcu-common/src/error_code.c`：

```c
#include "idcu/common/error_code.h"

const char* idcu_error_string(idcu_ErrorCode code) {
    switch (code) {
        case IDCU_ERR_SUCCESS:     return "Success";
        case IDCU_ERR_GENERAL:     return "General error";
        case IDCU_ERR_INVALID_PARAM: return "Invalid parameter";
        case IDCU_ERR_NO_MEMORY:   return "Out of memory";
        case IDCU_ERR_PERM_DENIED: return "Permission denied";
        case IDCU_ERR_NOT_FOUND:   return "Not found";
        case IDCU_ERR_ALREADY_EXISTS: return "Already exists";
        default: return "Unknown error";
    }
}
```

#### 8.2.3 实现基础数据结构

按照相同的模式，实现：
- `atomic.h/c` - 原子操作
- `lock.h/c` - 互斥锁和条件变量
- `vector.h/c` - 动态数组
- `linked_list.h/c` - 双向链表
- `hash_map.h/c` - 哈希表

#### 8.2.4 创建 CMakeLists.txt（使用 idcu-module-build！）

**经验提示**：从这里开始，所有模块都使用 idcu-module-build！

### 8.3 步骤 2: 完善 idcu-module-build

将 idcu-common 集成到 idcu-module-build 中，使其可以用于后续模块的构建。

### 8.4 步骤 3: 创建模块系统

#### 8.4.1 创建目录结构

```bash
mkdir -p modules/core/module-system/include
mkdir -p modules/core/module-system/src
```

#### 8.4.2 定义模块接口

创建 `modules/core/module-system/include/module_def.h`：

```c
#ifndef IDCU_MODULE_DEF_H
#define IDCU_MODULE_DEF_H

#include "idcu/common/error_code.h"

typedef struct {
    const char* name;
    int version_major;
    int version_minor;
    int version_patch;
    int (*init)(void);
    int (*run)(void);
    int (*stop)(void);
} idcu_ModuleInterface;

typedef struct {
    const idcu_ModuleInterface* iface;
    int initialized;
} idcu_RegisteredModule;

#define IDCU_REGISTER_MODULE(name, maj, min, patch, init_fn, run_fn, stop_fn) \
    static const idcu_ModuleInterface _module_##name = { \
        .name = #name, \
        .version_major = maj, \
        .version_minor = min, \
        .version_patch = patch, \
        .init = init_fn, \
        .run = run_fn, \
        .stop = stop_fn, \
    }; \
    __attribute__((constructor)) \
    static void _register_##name(void) { \
        idcu_module_register(&_module_##name); \
    }

void idcu_module_register(const idcu_ModuleInterface* iface);
int idcu_module_init_all(void);
int idcu_module_run_all(void);
int idcu_module_stop_all(void);

#endif
```

### 8.5 阶段 2 验收标准

- [ ] 通用基础库可正常使用
- [ ] **idcu-module-build 已完善并可用于所有后续模块**
- [ ] 模块系统可以注册和管理模块
- [ ] 协程调度器可以正常工作
- [ ] 消息总线可以收发消息
- [ ] 微内核可以启动和运行
- [ ] SDK 基础接口已设计完成

---

## 9. 第三阶段：独立库开发与完善

### 9.1 阶段里程碑

完成本阶段后，你应该能够：
- ✅ 使用日志系统记录日志
- ✅ 解析和序列化 JSON/YAML 配置
- ✅ 使用配置系统管理配置
- ✅ 使用网络层进行通信
- ✅ 收集指标和进行健康检查

### 9.2 提取原则

1. **单一职责**: 每个库只负责一个功能领域
2. **无项目依赖**: 库不能依赖 `micro_kernel.h` 等项目特定的头文件
3. **完整的文档**: 每个库都有 README 和 API 文档
4. **可独立测试**: 每个库都有自己的单元测试

### 9.3 提取示例：idcu-log

#### 9.3.1 创建目录结构

```bash
mkdir -p libs/idcu-log/include/idcu/log
mkdir -p libs/idcu-log/src
mkdir -p libs/idcu-log/tests
mkdir -p libs/idcu-log/examples
```

#### 9.3.2 实现日志功能

创建 `libs/idcu-log/include/idcu/log/log.h`：

```c
#ifndef IDCU_LOG_LOG_H
#define IDCU_LOG_LOG_H

#include "idcu/common/error_code.h"

typedef enum {
    IDCU_LOG_DEBUG = 0,
    IDCU_LOG_INFO = 1,
    IDCU_LOG_WARN = 2,
    IDCU_LOG_ERROR = 3,
    IDCU_LOG_FATAL = 4,
} idcu_LogLevel;

idcu_ErrorCode idcu_log_init(const char* file_path, idcu_LogLevel level);
void idcu_log_set_level(idcu_LogLevel level);
void idcu_log_debug(const char* fmt, ...);
void idcu_log_info(const char* fmt, ...);
void idcu_log_warn(const char* fmt, ...);
void idcu_log_error(const char* fmt, ...);
void idcu_log_shutdown(void);

#endif
```

#### 9.3.3 创建 CMakeLists.txt

```cmake
add_library(idcu_log STATIC
    src/log.c
)

target_include_directories(idcu_log PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(idcu_log PRIVATE
    idcu::common
)

add_library(idcu::log ALIAS idcu_log)

# 示例
add_executable(example_log examples/example_log.c)
target_link_libraries(example_log PRIVATE idcu::log)
```

#### 9.3.4 编写 README.md

```markdown
# idcu-log

一个轻量级的 C 语言日志库。

## 快速开始

```c
#include "idcu/log/log.h"

int main(void) {
    idcu_log_init("app.log", IDCU_LOG_INFO);
    idcu_log_info("Hello, World!");
    idcu_log_shutdown();
    return 0;
}
```

## API 参考

详见 [include/idcu/log/log.h](include/idcu/log/log.h)
```

### 9.4 阶段 3 验收标准

- [ ] 所有计划的独立库都已创建
- [ ] 每个库都有完整的单元测试
- [ ] 每个库都有 README 文档
- [ ] 库之间的依赖关系清晰
- [ ] 可以独立编译和使用每个库

---

## 10. 第四阶段：模块系统完善

### 10.1 阶段里程碑

完成本阶段后，你应该能够：
- ✅ 通过集成层使用所有基础库
- ✅ 使用 SDK 快速开发新模块
- ✅ 模块之间可以通过消息总线通信

### 10.2 创建集成层

集成层的作用是将独立库与微内核架构连接起来。

创建 `modules/integrations/log-integration/src/log_integration.c`：

```c
#include "sdk.h"
#include "idcu/log/log.h"
#include <stdio.h>

static int log_integration_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing log integration");
    return IDCU_ERR_SUCCESS;
}

static int log_integration_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting log integration");
    return IDCU_ERR_SUCCESS;
}

static int log_integration_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping log integration");
    return IDCU_ERR_SUCCESS;
}

static void log_integration_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying log integration");
}

IDCU_SDK_MODULE_DEFINE(
    log_integration,
    "1.0.0",
    "Log system integration module",
    log_integration_init,
    log_integration_start,
    log_integration_stop,
    log_integration_destroy
);
```

### 10.3 创建 SDK

SDK（软件开发工具包）可以简化模块开发。

创建 `modules/core/sdk/include/sdk.h`：

```c
#ifndef IDCU_SDK_H
#define IDCU_SDK_H

#include "idcu/common/error_code.h"
#include "idcu/log/log.h"

typedef struct idcu_SdkContext idcu_SdkContext;

typedef int (*idcu_SdkInitFunc)(idcu_SdkContext* ctx);
typedef int (*idcu_SdkStartFunc)(idcu_SdkContext* ctx);
typedef int (*idcu_SdkStopFunc)(idcu_SdkContext* ctx);
typedef void (*idcu_SdkDestroyFunc)(idcu_SdkContext* ctx);

void idcu_sdk_log_info(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_error(idcu_SdkContext* ctx, const char* fmt, ...);
void* idcu_sdk_get_user_data(idcu_SdkContext* ctx);
void idcu_sdk_set_user_data(idcu_SdkContext* ctx, void* data);

#define IDCU_SDK_MODULE_DEFINE(name, ver, desc, init_fn, start_fn, stop_fn, destroy_fn) \
    /* 模块定义宏实现 */

#endif
```

### 10.4 阶段 4 验收标准

- [ ] 所有独立库都有对应的集成模块
- [ ] SDK 可以正常使用
- [ ] 模块可以通过 SDK 便捷地开发
- [ ] 集成层与微内核架构无缝对接

---

## 11. 第五阶段：业务模块开发

### 11.1 阶段里程碑

完成本阶段后，你应该能够：
- ✅ 运行完整的业务功能
- ✅ 模块之间可以协同工作
- ✅ 系统可以稳定运行

### 11.2 创建第一个业务模块

让我们创建一个简单的心跳模块。

#### 11.2.1 创建目录结构

```bash
mkdir -p modules/business/heartbeat/src
```

#### 11.2.2 编写模块代码

创建 `modules/business/heartbeat/src/heartbeat_module.c`：

```c
#include "sdk.h"
#include <stdio.h>

typedef struct {
    int counter;
} HeartbeatData;

static int heartbeat_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing heartbeat module");
    
    HeartbeatData* data = malloc(sizeof(HeartbeatData));
    if (!data) {
        return IDCU_ERR_NO_MEMORY;
    }
    data->counter = 0;
    idcu_sdk_set_user_data(ctx, data);
    
    return IDCU_ERR_SUCCESS;
}

static int heartbeat_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting heartbeat module");
    return IDCU_ERR_SUCCESS;
}

static int heartbeat_run(idcu_SdkContext* ctx) {
    HeartbeatData* data = idcu_sdk_get_user_data(ctx);
    
    if (data->counter % 1000000 == 0) {
        idcu_sdk_log_info(ctx, "Heartbeat #%d", data->counter / 1000000);
    }
    data->counter++;
    
    return IDCU_ERR_SUCCESS;
}

static int heartbeat_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping heartbeat module");
    return IDCU_ERR_SUCCESS;
}

static void heartbeat_destroy(idcu_SdkContext* ctx) {
    HeartbeatData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
    idcu_sdk_log_info(ctx, "Destroying heartbeat module");
}

IDCU_SDK_MODULE_DEFINE(
    heartbeat,
    "1.0.0",
    "Heartbeat module",
    heartbeat_init,
    heartbeat_start,
    heartbeat_run,
    heartbeat_stop,
    heartbeat_destroy
);
```

#### 11.2.3 创建 CMakeLists.txt

```cmake
add_library(idcu_business_heartbeat STATIC
    src/heartbeat_module.c
)

target_include_directories(idcu_business_heartbeat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modules/core/sdk/include
)

target_link_libraries(idcu_business_heartbeat PRIVATE
    idcu::common
    idcu::log
    idcu_core_sdk
)
```

### 11.3 阶段 5 验收标准

- [ ] 核心业务模块都已实现
- [ ] 模块之间可以通过消息总线通信
- [ ] 系统可以正常运行
- [ ] 所有业务模块都有对应的测试

---

## 12. 调试技巧

### 12.1 使用 GDB/LLDB 调试

#### 启动调试

```bash
# 使用 GDB
gdb ./idcu_agent

# 使用 LLDB
lldb ./idcu_agent
```

#### 常用调试命令

| 命令 | 说明 |
|-----|------|
| `break main` | 在 main 函数设置断点 |
| `run` | 运行程序 |
| `next` / `n` | 单步执行（不进入函数） |
| `step` / `s` | 单步执行（进入函数） |
| `print var` / `p var` | 打印变量值 |
| `backtrace` / `bt` | 查看调用栈 |
| `continue` / `c` | 继续执行 |

### 12.2 添加调试日志

在关键位置添加调试日志：

```c
#include "idcu/log/log.h"

void some_function() {
    idcu_log_debug("Entering some_function");
    
    // ... 代码 ...
    
    idcu_log_debug("Variable value: %d", some_var);
    idcu_log_debug("Exiting some_function");
}
```

### 12.3 使用内存检测工具

#### Valgrind（Linux）

```bash
valgrind --leak-check=full ./idcu_agent
```

#### AddressSanitizer（GCC/Clang）

编译时添加：

```cmake
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
```

运行时会自动检测内存问题。

### 12.4 核心转储分析

启用核心转储：

```bash
# Linux
ulimit -c unlimited

# 分析核心转储
gdb ./idcu_agent core
```

---

## 13. 最佳实践与经验教训

### 13.1 从第一次开发中学到的经验

#### ✅ 做得好的地方（继续保持！）

1. **微内核架构** - 这个选择非常正确
2. **模块化设计** - 模块系统工作得很好
3. **文档重视** - 文档很完善
4. **跨平台支持** - CMake 选择正确

#### ⚠️ 可以改进的地方（这次要避免！）

1. **测试滞后** - 很多模块测试不够完善 → **这次用 TDD，先写测试！**
2. **依赖混乱** - 有些模块依赖关系不够清晰 → **这次有更严格的分层！**
3. **构建不一致** - 不同模块的 CMake 写法不统一 → **这次从第一天就用 idcu-module-build！**
4. **API 迭代** - 有些 API 经过多次修改才稳定 → **这次一次设计到位！**

### 13.2 代码组织原则

1. **分层清晰**: 基础层 → 工具层 → 服务层 → 业务层
2. **依赖单向**: 上层可以依赖下层，下层不能依赖上层
3. **模块化**: 每个模块只负责一个功能领域
4. **单一职责**: 每个模块只做一件事

### 13.3 开发流程（优化版）

1. **先写测试**: 测试驱动开发（TDD）- **这次必须做到！**
2. **小步提交**: 每次提交只做一件事
3. **及时验证**: 每个阶段都要充分测试，不要等最后才集成
4. **代码审查**: 每次合并前都要审查
5. **文档同步**: 代码更新时同步更新文档

### 13.4 重开发时的注意事项

#### 不要过度设计

- 保持简单，够用就好
- 不要为了"优雅"而增加复杂度
- 实用主义优先

#### 保持兼容性（如果可能）

- 如果可能，保持 API 兼容
- 提供迁移指南
- 渐进式改进，而不是革命

---

## 14. 任务完成标准流程

**每个开发任务完成后，必须按照以下流程操作：**

### 14.1 任务完成检查清单

在宣布任务完成前，请确认：

- [ ] 功能实现完整，满足需求
- [ ] **先写了测试**（TDD！）
- [ ] 代码已通过 `clang-format` 格式化
- [ ] 代码可以正常编译，无警告
- [ ] 所有相关单元测试通过
- [ ] 集成测试通过（如适用）
- [ ] `clang-tidy` 静态分析通过
- [ ] 已添加必要的注释和文档
- [ ] 已更新相关的 README 和开发文档
- [ ] 临时文件已清理

### 14.2 标准操作流程

#### 步骤 1: 运行测试和检查

```bash
# 1. 格式化代码
scripts/format.bat          # Windows
scripts/format.sh           # Linux

# 2. 检查格式（可选，用于验证）
scripts/check_format.bat    # Windows
scripts/check_format.sh     # Linux

# 3. 构建项目（启用测试）
cmake -B build -DBUILD_TESTS=ON
cmake --build build --config Debug

# 4. 运行测试
cd build
ctest --output-on-failure

# 5. 运行静态分析（可选但推荐）
cd ..
scripts/run_clang_tidy.bat  # Windows
scripts/run_clang_tidy.sh   # Linux
```

#### 步骤 2: 清理临时文件

```bash
# 删除构建目录
rm -rf build/
rm -rf out/

# 删除备份文件（如适用）
find . -name "*~" -delete
find . -name "*.bak" -delete

# 确保没有提交不必要的文件
# 检查 .gitignore 是否包含了所有应该忽略的文件
```

#### 步骤 3: 更新开发文档

- 更新本文档（`step_by_step_development_guide.md`）中的进度
- 如添加了新模块，更新项目架构图
- 更新相关模块的 README.md
- 如果有 API 变更，更新 API 文档

#### 步骤 4: Git 提交

```bash
# 1. 查看变更
git status
git diff

# 2. 添加变更
git add .

# 3. 提交（使用规范的 commit message）
git commit -m "feat(scope): description of change

- Detailed change 1
- Detailed change 2

Closes #issue-number"

# 4. 推送到远程（如果是功能分支）
git push origin feature/your-feature-name
```

#### 步骤 5: 创建 Pull Request（如适用）

如果是团队协作：
1. 推送到远程功能分支
2. 在 GitHub/GitLab 上创建 Pull Request
3. 填写 PR 模板
4. 等待代码审查
5. 根据审查意见修改
6. 合并到 develop 分支

---

## 15. 常见问题 FAQ

### 15.1 开发相关

**Q: 我可以只重写部分模块吗？**

A: 当然可以！建议采用混合策略：
- 核心模块（微内核、模块系统）→ 重新设计
- 成熟库（idcu-log、idcu-json）→ 直接复用
- 业务模块 → 逐步实现

**Q: 开发中遇到问题怎么办？**

A: 建议按以下顺序排查：
1. 查看本文档的"调试技巧"章节
2. 参考现有项目的实现
3. 查看相关模块的 README
4. 在社区提问

**Q: 如何保证新老 API 兼容？**

A: 如果需要保持兼容：
1. 先标记旧 API 为 deprecated，而不是直接删除
2. 提供迁移指南
3. 保持数据格式兼容
4. 考虑同时支持新旧 API 一段时间

### 15.2 技术相关

**Q: 为什么选择 C99 而不是 C11/C17？**

A: 详见"技术决策记录"章节。主要考虑：
- 更广泛的编译器支持
- 与现有代码库兼容
- 够用就好，不需要 C11 的新特性

**Q: 为什么自研测试框架而不是用 Unity/Check？**

A: 详见"技术决策记录"章节。主要考虑：
- 更轻量，无外部依赖
- 可以完全控制测试输出格式
- 与项目集成更紧密

**Q: 协程调度器性能不够怎么办？**

A: 可以考虑：
1. 使用汇编优化上下文切换
2. 减少协程切换频率
3. 使用工作窃取算法
4. 考虑使用 libco 或 boost.context 等成熟库

### 15.3 流程相关

**Q: 一个人开发也需要用 Git Flow 吗？**

A: 建议使用。即使一个人开发，Git Flow 也可以帮助你：
- 保持分支清晰
- 便于回滚
- 养成良好习惯

**Q: 每次提交都要运行所有测试吗？**

A: 建议至少运行相关模块的测试。完整测试可以在 CI 中运行。

**Q: 文档更新太麻烦，可以不更新吗？**

A: **不可以！** 文档是项目的重要组成部分。代码更新时必须同步更新文档。

---

## 16. 技术决策记录

### 16.1 为什么选择 C99 而不是 C11/C17？

**决策时间**: 2026-04-08
**决策者**: 开发团队
**状态**: 已决定

**原因**:
1. **更广泛的编译器支持** - C99 几乎在所有编译器上都支持
2. **与现有代码库兼容** - 现有项目使用 C99
3. **够用就好** - 我们不需要 C11 的新特性（如 `_Generic`、线程库等）
4. **降低学习成本** - 团队更熟悉 C99

**备选方案**:
- C11: 新特性，但兼容性差
- C++: 功能强大，但复杂度高
- Rust: 内存安全，但学习曲线陡峭

### 16.2 为什么自研测试框架而不是用 Unity/Check？

**决策时间**: 2026-04-08
**决策者**: 开发团队
**状态**: 已决定

**原因**:
1. **更轻量** - 只有几百行代码，无外部依赖
2. **完全控制** - 可以自定义测试输出格式和报告
3. **集成紧密** - 可以直接使用项目的错误码和日志系统
4. **学习成本低** - 自己写的框架，团队更容易理解

**备选方案**:
- Unity: 成熟，但需要额外依赖
- Check: 功能丰富，但复杂度高
- CTest: CMake 自带，但功能有限

### 16.3 为什么用 CMake 而不是 Meson/Bazel？

**决策时间**: 2026-04-08
**决策者**: 开发团队
**状态**: 已决定

**原因**:
1. **业界标准** - CMake 是 C/C++ 项目的事实标准
2. **广泛支持** - 几乎所有 IDE 和工具都支持 CMake
3. **现有经验** - 团队已经熟悉 CMake
4. **与现有项目兼容** - 现有项目使用 CMake

**备选方案**:
- Meson: 更现代，但生态系统小
- Bazel: Google 使用，但学习曲线陡峭
- Autotools: 老旧，不推荐

### 16.4 为什么选择微内核架构？

**决策时间**: 2026-04-08
**决策者**: 开发团队
**状态**: 已决定（第一次开发已验证）

**原因**:
1. **灵活性高** - 可以动态加载/卸载模块
2. **可扩展性好** - 添加新功能不需要修改核心
3. **稳定性好** - 一个模块崩溃不影响整个系统
4. **已验证** - 第一次开发证明这个选择正确

**备选方案**:
- 单体架构: 简单，但扩展性差
- 插件架构: 类似，但微内核更灵活

---

## 17. 整体进度追踪表

使用此表追踪你的开发进度：

| 阶段 | 状态 | 开始日期 | 完成日期 | 备注 |
|-----|------|---------|---------|------|
| **阶段 1**: 项目初始化和基础构建 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;1.1 创建项目目录结构 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;1.2 编写主程序入口 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;1.3 配置 CMake 构建系统 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;1.4 配置代码质量工具 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;1.5 搭建测试框架 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;1.6 idcu-module-build 初始化 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;1.7 验证项目可以编译 | ⏳ 待开始 | | | |
| **阶段 2**: 核心基础设施构建 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;2.1 idcu-common | ⏳ 待开始 | | | |
| &nbsp;&nbsp;2.2 完善 idcu-module-build | ⏳ 待开始 | | | |
| &nbsp;&nbsp;2.3 模块系统 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;2.4 协程调度器 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;2.5 消息总线 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;2.6 微内核核心 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;2.7 SDK 基础 | ⏳ 待开始 | | | |
| **阶段 3**: 独立库开发与完善 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;3.1-3.29 各独立库 | ⏳ 待开始 | | | |
| **阶段 4**: 模块系统完善 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;4.1-4.8 各集成模块和 SDK | ⏳ 待开始 | | | |
| **阶段 5**: 业务模块开发 | ⏳ 待开始 | | | |
| &nbsp;&nbsp;5.1-5.13 各业务模块 | ⏳ 待开始 | | | |

**状态说明**:
- ⏳ 待开始
- 🚧 进行中
- ✅ 已完成

---

## 附录

### A. 参考资料

- [CMake 官方文档](https://cmake.org/documentation/)
- [现代 CMake 最佳实践](https://cliutils.gitlab.io/modern-cmake/)
- [C 语言最佳实践](https://github.com/google/styleguide)
- [编码规范](./coding_standards.md)
- [开发工作流](./development_workflow.md)
- [路线图](./roadmap.md)

### B. 快速参考命令

```bash
# 编译项目
scripts/build.bat          # Windows
scripts/build.sh           # Linux

# 格式化代码
scripts/format.bat         # Windows
scripts/format.sh          # Linux

# 静态分析
scripts/run_clang_tidy.bat  # Windows
scripts/run_clang_tidy.sh   # Linux

# 运行测试
scripts/run_tests.bat      # Windows
scripts/run_tests.sh       # Linux
```

---

## 总结

**重开发不是目的，而是手段**。目标是：

1. 🎯 更清晰的架构
2. 🎯 更可靠的代码
3. 🎯 更完善的测试
4. 🎯 更好的开发体验

**记住**：
- 第一次开发是学习
- 第二次开发才是真正的创造！
- **不要过度设计，保持实用主义**
- **TDD 从第一天开始！**
- **idcu-module-build 从第一天就用！**

祝你开发顺利！🚀

有任何问题，欢迎在社区提问！
