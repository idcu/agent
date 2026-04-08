# 任务 4.6: metrics-integration - 指标集成模块

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建 metrics-integration 集成模块，将 idcu-metrics 集成到微内核架构中，支持：
- 统一的指标接口
- 指标收集、导出、聚合
- 指标告警和持久化
- Prometheus 格式支持
- 指标更新延迟 ≤ 10ms，支持 10000 指标/秒

### 1.2 不做什么
- 不修改 idcu-metrics 独立库的核心代码
- 不实现 Grafana 集成
- 不实现指标可视化

### 1.3 输入
- idcu-metrics 独立库
- idcu-alert、idcu-storage 独立库
- SDK 基础
- YAML 配置文件

### 1.4 输出
- metrics-integration 集成模块
- 可以注册、更新、查询指标
- 支持 Prometheus 格式导出

### 1.5 前置依赖
- phase3 15_idcu_metrics.md 任务已完成
- phase2 SDK 基础已完成
- 微内核初始化完成

---

## 2. 技术实现方案

### 2.1 核心选型
- 指标库：idcu-metrics
- 告警：idcu-alert
- 存储：idcu-storage
- 导出格式：Prometheus
- 构建系统：idcu-module-build

### 2.2 核心逻辑
1. 创建 metrics-integration 目录结构
2. 封装 idcu-metrics 接口
3. 实现指标注册和更新
4. 实现指标告警规则
5. 实现指标持久化
6. 实现 Prometheus 格式导出

### 2.3 数据结构/接口
```c
#ifndef IDCU_INTEGRATIONS_METRICS_INTEGRATION_H
#define IDCU_INTEGRATIONS_METRICS_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/metrics/metrics.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int                   initialized;
        idcu_MetricsCollector collector;
    } idcu_MetricsIntegration;

    int  idcu_metrics_integration_init(idcu_MetricsIntegration* integration);
    void idcu_metrics_integration_destroy(idcu_MetricsIntegration* integration);

    idcu_MetricsCollector* idcu_metrics_integration_get_collector(
        idcu_MetricsIntegration* integration);

#ifdef __cplusplus
}
#endif

#endif  // IDCU_INTEGRATIONS_METRICS_INTEGRATION_H
```

### 2.4 跨平台适配
- 无特殊跨平台差异
- 使用 idcu-common 提供的跨平台功能

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以创建和更新指标
- [ ] Prometheus 格式导出正常工作
- [ ] 模块可以正常初始化、启动、停止、销毁
- [ ] YAML 配置可以正确加载
- [ ] 支持多种指标类型（Counter、Gauge、Histogram）
- [ ] 支持指标标签

### 3.2 性能验收
- [ ] 指标更新延迟 ≤ 10ms
- [ ] 支持 10000 指标/秒
- [ ] 模块初始化时间 ≤ 50ms
- [ ] 内存占用 ≤ 512KB

### 3.3 异常验收
- [ ] 配置文件不存在时使用默认值，模块正常启动
- [ ] 指标名称重复时返回错误
- [ ] 模块初始化失败返回错误码，核心无崩溃

---

## 4. 执行计划

### 4.1 工期
0.5-1 天/人

### 4.2 里程碑
- D1：完成目录结构和集成代码
- D1：完成 CMakeLists.txt 和配置文件
- D1：完成测试和验证

### 4.3 人力
1 人（技能要求：C 语言 + 熟悉 idcu-metrics）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目的 .clang-format 规范
- 函数名小写+下划线，结构体前缀 idcu_
- 所有头文件使用 include guard

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试 3 种异常场景
- 至少 3 个测试用例

### 5.3 部署指引
- 编译命令：`cmake --build build --target metrics_integration`
- 部署路径：`modules/integrations/metrics-integration/`
- 配置使用 YAML 格式

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：指标收集开销过大影响性能  
应对：使用轻量级指标收集，支持可配置的采样率

### 6.2 风险2
描述：Prometheus 导出格式复杂  
应对：参考 Prometheus 官方文档，使用标准格式

### 6.3 风险3
描述：性能不满足预期要求  
应对：进行性能基准测试，优化关键路径代码

### 6.4 风险4
描述：跨平台兼容性问题  
应对：使用跨平台 API，充分测试不同平台

---

## 7. 详细实现步骤

### 7.1 创建目录结构

```bash
mkdir -p modules/integrations/metrics-integration/src
mkdir -p modules/integrations/metrics-integration/include
mkdir -p modules/integrations/metrics-integration/tests
```

### 7.2 实现集成模块

创建 `modules/integrations/metrics-integration/src/metrics_integration.c`：

```c
#include "sdk.h"
#include "idcu/metrics/metrics.h"
#include "idcu/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
    idcu_MetricsCollector collector;
} MetricsIntegrationData;

static idcu_ErrorCode metrics_integration_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing metrics integration");

    MetricsIntegrationData* data = malloc(sizeof(MetricsIntegrationData));
    if (!data) {
        return IDCU_ERR_NO_MEMORY;
    }
    memset(data, 0, sizeof(MetricsIntegrationData));

    // 初始化指标收集器
    idcu_ErrorCode err = idcu_metrics_collector_init(&data->collector);
    if (err != IDCU_ERR_SUCCESS) {
        free(data);
        return err;
    }

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Metrics integration initialized");
    return IDCU_ERR_SUCCESS;
}

static idcu_ErrorCode metrics_integration_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting metrics integration");
    return IDCU_ERR_SUCCESS;
}

static void metrics_integration_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping metrics integration");
}

static void metrics_integration_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying metrics integration");

    MetricsIntegrationData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        if (data->initialized) {
            idcu_metrics_collector_destroy(&data->collector);
        }
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    metrics_integration,
    "1.0.0",
    "Metrics system integration module",
    metrics_integration_init,
    metrics_integration_start,
    metrics_integration_stop,
    metrics_integration_destroy
);
```

### 7.3 创建 CMakeLists.txt

```cmake
include(idcu_module)

idcu_add_library(metrics_integration
    VERSION 1.0.0
    SOURCES
        src/metrics_integration.c
    HEADERS
        include/metrics_integration.h
    DEPENDS
        idcu::sdk
        idcu::metrics
        idcu::config
)
```

### 7.4 YAML 配置示例（默认格式）

创建配置示例 `config/default/metrics_integration.yaml`：

```yaml
# 指标集成配置
metrics:
  # 启用/禁用指标收集
  enabled: true

  # 收集间隔（毫秒）
  collection_interval: 1000

  # Prometheus 导出配置
  prometheus:
    enabled: true
    port: 9090
    path: "/metrics"

  # 指标持久化配置
  persistence:
    enabled: false
    interval: 60000
    file: "/var/lib/idcu/metrics.dat"
```

### 7.5 创建 README.md

```markdown
# metrics-integration

指标系统集成模块。

## 配置

使用 YAML 配置（默认）：

```yaml
metrics:
  enabled: true
  collection_interval: 1000
  prometheus:
    enabled: true
    port: 9090
```

## API

通过 SDK 使用：

```c
// 创建计数器
idcu_MetricCounter* counter = idcu_metrics_counter_create("requests_total", "Total requests");
idcu_metrics_counter_inc(counter);

// 创建仪表盘
idcu_MetricGauge* gauge = idcu_metrics_gauge_create("active_connections", "Active connections");
idcu_metrics_gauge_set(gauge, 100);
```
```

---

## 8. 验证检查清单

- [ ] 目录结构已创建
- [ ] 集成模块代码已创建
- [ ] CMakeLists.txt 已创建
- [ ] YAML 配置示例已创建
- [ ] README.md 已创建
- [ ] 模块可以正常编译
- [ ] 模块可以正常初始化
- [ ] 指标收集功能正常
- [ ] Prometheus 格式导出正常
- [ ] 配置可以正确加载
- [ ] 代码已格式化（clang-format）
- [ ] 静态分析通过（clang-tidy）
- [ ] 单元测试通过
- [ ] 代码已通过 clang-format 格式化
- [ ] 代码已通过 clang-tidy 静态分析
- [ ] 符合工程化标准要求

---

## 9. Git 提交

```bash
git add modules/integrations/metrics-integration/
git add config/default/metrics_integration.yaml
git commit -m "feat(integrations): add metrics integration

- Add metrics integration module
- Add SDK metrics wrappers
- Add Prometheus format export
- Add YAML config example (default format)
- Add CMakeLists.txt using idcu-module-build
- Add README"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 模块初始化失败 | 配置文件路径错误 | 检查 YAML 配置文件路径 |
| 指标不更新 | 收集间隔设置过长 | 降低收集间隔 |
| Prometheus 无法抓取 | 端口被占用 | 更改 Prometheus 端口配置 |
| 性能下降 | 指标数量过多 | 减少指标数量或启用采样 |
| 内存泄漏 | 未正确释放指标 | 确保在销毁前释放所有指标 |
