# 任务 3.15: idcu-metrics - 指标收集库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的指标收集库，支持 Counter、Gauge、Histogram 三种指标类型，支持标签维度，支持指标注册和查询，支持 Prometheus 和 JSON 格式导出，支持并发 1000 个指标操作，指标操作延迟 ≤ 1ms。

### 1.2 不做什么
- 不实现指标持久化存储
- 不实现自动推送到 Prometheus Server
- 不实现指标聚合和告警功能
- 不实现分布式指标收集

### 1.3 输入
- 指标名称（最大 64 字符）
- 指标帮助信息（最大 256 字符）
- 标签键值对（最多 16 个标签）
- Histogram 桶配置

### 1.4 输出
- 指标操作返回码（成功 0，失败非 0）
- Prometheus 格式文本输出
- JSON 格式文本输出

### 1.5 前置依赖
- idcu-common 模块已完成（vector、hash_map、lock）
- idcu-log 模块已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- 语言：C11
- 构建：CMake
- 锁：idcu-common 提供的互斥锁
- 容器：idcu-common 提供的 vector、hash_map

### 2.2 核心逻辑
1. 指标注册中心管理所有指标
2. 每种指标类型独立实现（Counter/Gauge/Histogram）
3. 指标操作使用互斥锁保证线程安全
4. 导出时遍历所有指标并格式化输出

### 2.3 数据结构/接口
```c
typedef enum {
    IDCU_METRIC_TYPE_COUNTER = 0,
    IDCU_METRIC_TYPE_GAUGE,
    IDCU_METRIC_TYPE_HISTOGRAM
} idcu_MetricType;

typedef struct {
    char name[64];
    char value[128];
} idcu_MetricLabel;

typedef struct {
    idcu_MetricLabel labels[16];
    size_t label_count;
} idcu_MetricLabels;

typedef struct idcu_MetricsRegistry {
    idcu_Vector metrics;
    idcu_HashMap metrics_by_name;
    idcu_Mutex lock;
    int initialized;
} idcu_MetricsRegistry;

// Counter API
idcu_MetricCounter* idcu_metrics_counter_create(const char* name, const char* help, const idcu_MetricLabels* labels);
void idcu_metrics_counter_inc(idcu_MetricCounter* counter);
void idcu_metrics_counter_add(idcu_MetricCounter* counter, uint64_t value);

// Gauge API
idcu_MetricGauge* idcu_metrics_gauge_create(const char* name, const char* help, const idcu_MetricLabels* labels);
void idcu_metrics_gauge_set(idcu_MetricGauge* gauge, double value);
void idcu_metrics_gauge_inc(idcu_MetricGauge* gauge);
void idcu_metrics_gauge_dec(idcu_MetricGauge* gauge);

// Histogram API
idcu_MetricHistogram* idcu_metrics_histogram_create(const char* name, const char* help, const idcu_MetricLabels* labels, const double* buckets, size_t bucket_count);
void idcu_metrics_histogram_observe(idcu_MetricHistogram* histogram, double value);

// 导出 API
int idcu_metrics_to_prometheus(const idcu_MetricsRegistry* registry, char* buffer, size_t buffer_size);
int idcu_metrics_to_json(const idcu_MetricsRegistry* registry, char* buffer, size_t buffer_size);
```

### 2.4 跨平台适配
- Windows/Linux 使用相同的 API 接口
- 互斥锁底层由 idcu-common 处理跨平台差异
- 无需额外的平台特定代码

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] Counter 可以正常递增和累加
- [ ] Gauge 可以正常设置、增减
- [ ] Histogram 可以正常观测和统计分布
- [ ] 支持最多 16 个标签
- [ ] 指标可以注册到 Registry 并查询
- [ ] 可以导出为 Prometheus 格式
- [ ] 可以导出为 JSON 格式

### 3.2 性能验收
- 单指标操作延迟 ≤ 1ms
- 支持并发 1000 个指标操作无崩溃
- 100 个指标的 Prometheus 导出耗时 ≤ 10ms
- 单个指标内存占用 ≤ 1KB

### 3.3 异常验收
- 创建重复名称指标返回错误码
- 使用空指针操作返回错误码
- 导出缓冲区不足返回错误码
- 多线程并发操作无数据竞争

---

## 4. 执行计划

### 4.1 工期
2 天/人

### 4.2 里程碑
- D1：完成数据结构定义和头文件
- D2：完成 Counter、Gauge、Histogram 实现和导出功能
- D2：完成单元测试和验证

### 4.3 人力
1 人（技能要求：C语言 + 多线程编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名使用小写+下划线，前缀 idcu_metrics_
- 结构体名前缀 idcu_Metric
- 宏定义全大写，前缀 IDCU_METRIC_

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试覆盖三种指标类型的所有操作
- 测试覆盖标签功能
- 测试覆盖导出功能
- 测试覆盖多线程并发场景

### 5.3 部署指引
- 编译命令：`cmake --build build --target idcu-metrics`
- 头文件部署路径：`include/idcu/metrics/`
- 库文件部署路径：`lib/`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：多线程并发操作导致数据竞争  
应对：每个指标独立使用互斥锁，Registry 操作也加锁保护

### 6.2 风险2
描述：导出时缓冲区不足导致截断  
应对：返回需要的缓冲区大小，调用方可以重新分配足够的缓冲区

---

## 7. 详细实现步骤

### 7.1 创建目录结构

```bash
mkdir -p libs/idcu-metrics/include/idcu/metrics
mkdir -p libs/idcu-metrics/src/idcu/metrics
mkdir -p libs/idcu-metrics/tests
mkdir -p libs/idcu-metrics/examples
```

### 7.2 创建指标头文件 (metrics.h)

创建 `libs/idcu-metrics/include/idcu/metrics/metrics.h`：

```c
#ifndef IDCU_METRICS_METRICS_H
#define IDCU_METRICS_METRICS_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_METRIC_TYPE_COUNTER = 0,
    IDCU_METRIC_TYPE_GAUGE,
    IDCU_METRIC_TYPE_HISTOGRAM
} idcu_MetricType;

typedef struct
{
    char name[64];
    char value[128];
} idcu_MetricLabel;

typedef struct
{
    idcu_MetricLabel labels[16];
    size_t label_count;
} idcu_MetricLabels;

typedef struct idcu_MetricCounter
{
    char name[64];
    char help[256];
    idcu_MetricLabels labels;
    uint64_t value;
    idcu_Mutex lock;
} idcu_MetricCounter;

typedef struct idcu_MetricGauge
{
    char name[64];
    char help[256];
    idcu_MetricLabels labels;
    double value;
    idcu_Mutex lock;
} idcu_MetricGauge;

typedef struct idcu_MetricHistogram
{
    char name[64];
    char help[256];
    idcu_MetricLabels labels;
    double* buckets;
    size_t bucket_count;
    uint64_t* bucket_counts;
    uint64_t sample_count;
    double sample_sum;
    idcu_Mutex lock;
} idcu_MetricHistogram;

typedef struct idcu_Metric
{
    idcu_MetricType type;
    union
    {
        idcu_MetricCounter* counter;
        idcu_MetricGauge* gauge;
        idcu_MetricHistogram* histogram;
    } data;
} idcu_Metric;

typedef struct
{
    idcu_Vector metrics;
    idcu_HashMap metrics_by_name;
    idcu_Mutex lock;
    int initialized;
} idcu_MetricsRegistry;

int  idcu_metrics_registry_init(idcu_MetricsRegistry* registry);
void idcu_metrics_registry_destroy(idcu_MetricsRegistry* registry);

idcu_MetricCounter* idcu_metrics_counter_create(const char* name, const char* help, const idcu_MetricLabels* labels);
void idcu_metrics_counter_destroy(idcu_MetricCounter* counter);
void idcu_metrics_counter_inc(idcu_MetricCounter* counter);
void idcu_metrics_counter_add(idcu_MetricCounter* counter, uint64_t value);
uint64_t idcu_metrics_counter_get(idcu_MetricCounter* counter);

idcu_MetricGauge* idcu_metrics_gauge_create(const char* name, const char* help, const idcu_MetricLabels* labels);
void idcu_metrics_gauge_destroy(idcu_MetricGauge* gauge);
void idcu_metrics_gauge_set(idcu_MetricGauge* gauge, double value);
void idcu_metrics_gauge_inc(idcu_MetricGauge* gauge);
void idcu_metrics_gauge_dec(idcu_MetricGauge* gauge);
void idcu_metrics_gauge_add(idcu_MetricGauge* gauge, double value);
void idcu_metrics_gauge_sub(idcu_MetricGauge* gauge, double value);
double idcu_metrics_gauge_get(idcu_MetricGauge* gauge);

idcu_MetricHistogram* idcu_metrics_histogram_create(const char* name, const char* help, const idcu_MetricLabels* labels,
                                                      const double* buckets, size_t bucket_count);
void idcu_metrics_histogram_destroy(idcu_MetricHistogram* histogram);
void idcu_metrics_histogram_observe(idcu_MetricHistogram* histogram, double value);
void idcu_metrics_histogram_get_counts(idcu_MetricHistogram* histogram, uint64_t* sample_count, double* sample_sum);

int idcu_metrics_registry_register_counter(idcu_MetricsRegistry* registry, idcu_MetricCounter* counter);
int idcu_metrics_registry_register_gauge(idcu_MetricsRegistry* registry, idcu_MetricGauge* gauge);
int idcu_metrics_registry_register_histogram(idcu_MetricsRegistry* registry, idcu_MetricHistogram* histogram);

int idcu_metrics_registry_unregister(idcu_MetricsRegistry* registry, const char* name);

idcu_MetricCounter* idcu_metrics_registry_get_counter(idcu_MetricsRegistry* registry, const char* name);
idcu_MetricGauge* idcu_metrics_registry_get_gauge(idcu_MetricsRegistry* registry, const char* name);
idcu_MetricHistogram* idcu_metrics_registry_get_histogram(idcu_MetricsRegistry* registry, const char* name);

int idcu_metrics_to_prometheus(const idcu_MetricsRegistry* registry, char* buffer, size_t buffer_size);
int idcu_metrics_to_json(const idcu_MetricsRegistry* registry, char* buffer, size_t buffer_size);

int idcu_metrics_labels_init(idcu_MetricLabels* labels);
int idcu_metrics_labels_add(idcu_MetricLabels* labels, const char* name, const char* value);
void idcu_metrics_labels_clear(idcu_MetricLabels* labels);

#ifdef __cplusplus
}
#endif

#endif
```

### 7.3 创建 CMakeLists.txt

创建 `libs/idcu-metrics/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-metrics VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-metrics STATIC
    src/idcu/metrics/metrics.c
)

target_include_directories(idcu-metrics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-metrics PRIVATE
    idcu::common
    idcu::log
)

add_library(idcu::metrics ALIAS idcu-metrics)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 7.4 创建模块配置文件 (module.yaml)

创建 `libs/idcu-metrics/module.yaml`：

```yaml
name: idcu-metrics
version: 1.0.0
description: Metrics collection library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-metrics

headers:
  - idcu/metrics/metrics.h

features:
  - counter: Counter metric type
  - gauge: Gauge metric type
  - histogram: Histogram metric type
  - labels: Label support for metrics
  - registry: Metric registry and management
  - prometheus: Prometheus format export
  - json: JSON format export

testing:
  enabled: true
  framework: internal
```

### 7.5 创建 README.md

创建 `libs/idcu-metrics/README.md`：

```markdown
# idcu-metrics

IDCU Agent 的指标收集库。

## 功能特性

- **Counter**: 单调递增计数器
- **Gauge**: 可增减的仪表盘
- **Histogram**: 直方图统计
- **标签支持**: 为指标添加维度标签
- **指标注册**: 集中管理指标
- **Prometheus 导出**: 支持 Prometheus 格式
- **JSON 导出**: 支持 JSON 格式

## 快速开始

### 使用 Counter

```c
#include "idcu/metrics/metrics.h"

idcu_MetricsRegistry registry;
idcu_metrics_registry_init(&registry);

idcu_MetricLabels labels;
idcu_metrics_labels_init(&labels);
idcu_metrics_labels_add(&labels, "service", "api");

idcu_MetricCounter* requests = idcu_metrics_counter_create(
    "http_requests_total",
    "Total HTTP requests",
    &labels
);

idcu_metrics_registry_register_counter(&registry, requests);

idcu_metrics_counter_inc(requests);
idcu_metrics_counter_add(requests, 5);

uint64_t value = idcu_metrics_counter_get(requests);
printf("Requests: %" PRIu64 "\n", value);

idcu_metrics_counter_destroy(requests);
idcu_metrics_registry_destroy(&registry);
```

### 使用 Gauge

```c
idcu_MetricGauge* connections = idcu_metrics_gauge_create(
    "active_connections",
    "Active connections",
    NULL
);

idcu_metrics_gauge_set(connections, 0);
idcu_metrics_gauge_inc(connections);
idcu_metrics_gauge_add(connections, 10);
idcu_metrics_gauge_dec(connections);

double current = idcu_metrics_gauge_get(connections);
```

### 导出为 Prometheus 格式

```c
char buffer[8192];
idcu_metrics_to_prometheus(&registry, buffer, sizeof(buffer));
printf("%s\n", buffer);
```

## 指标类型

### Counter

用于统计单调递增的值，如请求数、错误数等。

### Gauge

用于统计可以增减的值，如连接数、内存使用量等。

### Histogram

用于统计分布情况，如请求延迟、响应大小等。

## API 文档

详见 [include/idcu/metrics/metrics.h](include/idcu/metrics/metrics.h)
```

---

## 8. 验证检查清单

- [ ] 指标头文件已创建
- [ ] 指标实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] Counter 可以正常增减
- [ ] Gauge 可以正常设置和修改
- [ ] Histogram 可以正常观测
- [ ] 指标可以导出为 Prometheus 格式
- [ ] 指标可以导出为 JSON 格式
- [ ] 单元测试通过
- [ ] 可以正常编译
- [ ] 性能指标达标

---

## 9. Git 提交

```bash
git add libs/idcu-metrics/
git commit -m "feat: add idcu-metrics library

- Add Counter metric type
- Add Gauge metric type
- Add Histogram metric type
- Add label support for metrics
- Add metric registry and management
- Add Prometheus format export
- Add JSON format export
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 指标值不正确 | 未正确使用锁 | 确保在多线程环境中正确使用 |
| 导出失败 | 缓冲区太小 | 使用足够大的缓冲区 |
| 内存泄漏 | 未销毁指标 | 确保创建的指标都被销毁 |
