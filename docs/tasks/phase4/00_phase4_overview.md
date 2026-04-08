# 任务 4.0: 第四阶段 - 模块系统完善

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
完成 phase4 后，实现完整的模块系统集成层，支持：
- 通过集成层使用所有 phase3 开发的基础库
- 使用 SDK 快速开发新模块
- 模块之间可以通过消息总线通信
- 模块初始化耗时 ≤ 100ms，支持 100 个并发模块加载

### 1.2 不做什么
- 不修改 phase1-3 已完成的基础库代码
- 不开发新的业务模块（留到 phase5）
- 不实现跨版本模块回滚

### 1.3 输入
- phase3 已完成的所有独立库（idcu-log、idcu-config、idcu-json 等）
- phase2 完成的 SDK 基础
- 微内核和消息总线

### 1.4 输出
- 10 个集成模块（log-integration、config-integration 等）
- 完整的 SDK 实现
- 统一的模块开发规范

### 1.5 前置依赖
- phase1 项目基础构建完成
- phase2 核心基础设施（微内核、SDK 基础）完成
- phase3 所有独立库开发完成

---

## 2. 技术实现方案

### 2.1 核心选型
- 集成模块架构：基于 SDK 的统一封装层
- 配置格式：YAML 为默认格式
- 构建系统：idcu-module-build
- 跨平台：支持 Windows/Linux

### 2.2 核心逻辑
1. 为每个 phase3 独立库创建对应的集成模块
2. 将独立库接口封装为 SDK 可用的接口
3. 实现模块生命周期管理（init/start/stop/destroy）
4. 实现与消息总线的集成
5. 完善 SDK 提供完整的模块开发功能

### 2.3 数据结构/接口
```c
// SDK 模块定义宏
#define IDCU_SDK_MODULE_DEFINE(name, ver, desc, init_fn, start_fn, stop_fn, destroy_fn) \
    /* 模块定义实现 */

// 集成模块标准接口
typedef int (*idcu_SdkInitFunc)(idcu_SdkContext* ctx);
typedef int (*idcu_SdkStartFunc)(idcu_SdkContext* ctx);
typedef void (*idcu_SdkStopFunc)(idcu_SdkContext* ctx);
typedef void (*idcu_SdkDestroyFunc)(idcu_SdkContext* ctx);
```

### 2.4 跨平台适配
- Windows：使用 LoadLibrary 加载模块
- Linux：使用 dlopen 加载模块
- 路径分隔符：Windows 用 `\`，Linux 用 `/`
- 动态库扩展名：Windows 用 `.dll`，Linux 用 `.so`

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 所有 phase3 独立库都有对应的集成模块（共 9 个）
- [ ] SDK 可以正常使用，支持模块快速开发
- [ ] 模块可以通过 SDK 便捷地开发
- [ ] 集成层与微内核架构无缝对接
- [ ] 模块之间可以通过消息总线通信
- [ ] 支持 YAML 配置格式（默认）

### 3.2 性能验收
- [ ] 单个集成模块初始化时间 ≤ 50ms
- [ ] 支持并发加载 100 个模块无崩溃
- [ ] 模块间消息延迟 ≤ 1ms
- [ ] 内存占用 ≤ 512KB/集成模块

### 3.3 异常验收
- [ ] 加载非法模块返回错误码，核心无崩溃
- [ ] 配置加载失败时模块优雅降级
- [ ] 网络异常时模块不影响其他模块运行
- [ ] 日志输出完整的错误码和原因

---

## 4. 执行计划

### 4.1 工期
5 天/人

### 4.2 里程碑
- D1：完成 00_phase4_overview 和 01_log_integration
- D2：完成 02-05 文档（config、json、yaml、network）
- D3：完成 06-08 文档（metrics、basic-libs、sdk-complete）
- D4：补充 REST API 文档，代码质量检查
- D5：验证与验收，Git 提交

### 4.3 人力
1 人（技能要求：C 语言 + Linux/Windows 开发 + 熟悉微内核架构）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目的 .clang-format 规范
- 函数名小写+下划线，结构体前缀 Idcu_
- 所有头文件使用 include guard

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 集成测试覆盖 5 种异常场景
- 每个集成模块至少 3 个测试用例

### 5.3 部署指引
- 编译命令：`cmake --build build --target all`
- 部署路径：`modules/integrations/`
- 模块配置使用 YAML 格式

---

## 6. 风险与应对

### 6.1 风险1
描述：集成模块与独立库接口不兼容  
应对：先定义统一的接口规范，再开发集成模块

### 6.2 风险2
描述：跨平台性能差异大  
应对：分别制定 Windows/Linux 性能基线，针对性优化

---

## 7. 详细实现步骤

### 7.1 任务列表

| 序号 | 组件 | 文档 | 状态 | 预计时间 | 依赖 |
|-----|------|------|------|---------|------|
| 4.1 | log-integration | [01_log_integration.md](./01_log_integration.md) | ⏳ 待开始 | 2小时 | 3.1 + SDK |
| 4.2 | config-integration | [02_config_integration.md](./02_config_integration.md) | ⏳ 待开始 | 2小时 | 3.6 + SDK |
| 4.3 | json-integration | [03_json_integration.md](./03_json_integration.md) | ⏳ 待开始 | 2小时 | 3.2 + SDK |
| 4.4 | yaml-integration | [04_yaml_integration.md](./04_yaml_integration.md) | ⏳ 待开始 | 2小时 | 3.3 + SDK |
| 4.5 | network-integration | [05_network_integration.md](./05_network_integration.md) | ⏳ 待开始 | 2小时 | 3.9 + SDK |
| 4.6 | metrics-integration | [06_metrics_integration.md](./06_metrics_integration.md) | ⏳ 待开始 | 2小时 | 3.15 + SDK |
| 4.7 | basic-libs | [07_basic_libs.md](./07_basic_libs.md) | ⏳ 待开始 | 3小时 | 多个基础库 + SDK |
| 4.8 | SDK 完善 | [08_sdk_complete.md](./08_sdk_complete.md) | ⏳ 待开始 | 4小时 | 微内核 |
| - | REST API | [01_rest_api.md](./01_rest_api.md) | ⏳ 待开始 | 3小时 | 多个库 + SDK |

### 7.2 创建集成层示例

集成层的作用是将独立库与微内核架构连接起来。

创建 `modules/integrations/log-integration/src/log_integration.c`：

```c
#include "sdk.h"
#include "idcu/log/log.h"
#include &lt;stdio.h&gt;

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

### 7.3 创建 SDK 示例

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

---

## 8. 验证检查清单

- [ ] phase4 所有 10 个文档已创建
- [ ] 每个集成模块文档包含任务边界、技术方案、验收标准
- [ ] YAML 配置示例已添加到相关文档
- [ ] 代码质量工具使用要求已明确
- [ ] 安全最佳实践已强调
- [ ] 所有文档无乱码问题

---

## 9. Git 提交

```bash
git add docs/tasks/phase4/
git commit -m "docs: complete phase4 documentation

- Add phase4 overview with standardized template
- Add 10 integration module task documents
- Add YAML config examples (default format)
- Add code quality and security considerations
- Align all docs with reference/task_template.md"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 集成模块无法加载 | SDK 未初始化 | 确保先初始化 SDK |
| 配置读取失败 | 配置路径错误 | 检查 YAML 配置文件路径 |
| 消息总线通信失败 | 端点未注册 | 确保消息总线已启动 |
