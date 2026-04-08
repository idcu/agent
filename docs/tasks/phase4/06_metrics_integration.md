# 任务 4.6: metrics-integration - 指标集成模块

## 目标

创建指标集成模块，支持：
- 统一的指标接口
- 指标收集集成
- 指标导出集成
- 指标聚合
- 指标告警
- 指标持久化
- 指标查询
- Prometheus 格式支持
- 自定义指标注册

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/metrics-integration/include/idcu/metrics_integration
mkdir -p modules/metrics-integration/src/idcu/metrics_integration
mkdir -p modules/metrics-integration/tests
mkdir -p modules/metrics-integration/examples
```

### 2. 创建指标集成头文件 (metrics_integration.h)

创建 `modules/metrics-integration/include/idcu/metrics_integration/metrics_integration.h`：

```c
#ifndef IDCU_METRICS_INTEGRATION_METRICS_INTEGRATION_H
#define IDCU_METRICS_INTEGRATION_METRICS_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/metrics/metrics.h"
#include "idcu/alert/alert.h"
#include "idcu/storage/storage.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_MetricsIntegrationId;

typedef enum
{
    IDCU_METRICS_FORMAT_PROMETHEUS = 0,
    IDCU_METRICS_FORMAT_JSON,
    IDCU_METRICS_FORMAT_TEXT
} idcu_MetricsFormat;

typedef struct
{
    char name[128];
    char metric_name[128];
    double threshold;
    int operator_type;
    char alert_topic[256];
    int enabled;
} idcu_MetricsAlertRule;

typedef struct
{
    idcu_MetricsIntegrationId id;
    char name[128];
    
    idcu_MetricsRegistry registry;
    idcu_Vector alert_rules;
    idcu_Mutex lock;
    
    idcu_AlertManager* alert_manager;
    idcu_KVStore* storage;
    
    int enable_persistence;
    uint64_t persistence_interval_ms;
    
    int enable_alerting;
    uint64_t alert_check_interval_ms;
    
    idcu_Thread persistence_thread;
    idcu_Thread alert_thread;
    int running;
    int initialized;
} idcu_MetricsIntegration;

typedef struct
{
    idcu_MetricsRegistryConfig registry_config;
    idcu_AlertManagerConfig alert_config;
    idcu_KVStoreConfig storage_config;
    
    int enable_persistence;
    int enable_alerting;
    uint64_t persistence_interval_ms;
    uint64_t alert_check_interval_ms;
} idcu_MetricsIntegrationConfig;

int  idcu_metrics_integration_config_init(idcu_MetricsIntegrationConfig* config);

int  idcu_metrics_integration_init(idcu_MetricsIntegration* mi, const idcu_MetricsIntegrationConfig* config);
void idcu_metrics_integration_destroy(idcu_MetricsIntegration* mi);
int  idcu_metrics_integration_start(idcu_MetricsIntegration* mi);
void idcu_metrics_integration_stop(idcu_MetricsIntegration* mi);

int  idcu_metrics_integration_register(idcu_MetricsIntegration* mi, idcu_Metric* metric);
int  idcu_metrics_integration_unregister(idcu_MetricsIntegration* mi, idcu_Metric* metric);
idcu_Metric* idcu_metrics_integration_get(idcu_MetricsIntegration* mi, const char* name);

int  idcu_metrics_integration_counter_inc(idcu_MetricsIntegration* mi, const char* name);
int  idcu_metrics_integration_counter_add(idcu_MetricsIntegration* mi, const char* name, int64_t value);
int  idcu_metrics_integration_gauge_set(idcu_MetricsIntegration* mi, const char* name, double value);
int  idcu_metrics_integration_gauge_inc(idcu_MetricsIntegration* mi, const char* name);
int  idcu_metrics_integration_gauge_dec(idcu_MetricsIntegration* mi, const char* name);
int  idcu_metrics_integration_gauge_add(idcu_MetricsIntegration* mi, const char* name, double value);
int  idcu_metrics_integration_gauge_sub(idcu_MetricsIntegration* mi, const char* name, double value);
int  idcu_metrics_integration_histogram_observe(idcu_MetricsIntegration* mi, const char* name, double value);
int  idcu_metrics_integration_summary_observe(idcu_MetricsIntegration* mi, const char* name, double value);

int  idcu_metrics_integration_export(idcu_MetricsIntegration* mi, char* buffer, size_t buffer_size, idcu_MetricsFormat format);
int  idcu_metrics_integration_export_prometheus(idcu_MetricsIntegration* mi, char* buffer, size_t buffer_size);
int  idcu_metrics_integration_export_json(idcu_MetricsIntegration* mi, char* buffer, size_t buffer_size);

idcu_MetricsIntegrationId idcu_metrics_integration_add_alert_rule(idcu_MetricsIntegration* mi, const idcu_MetricsAlertRule* rule);
int  idcu_metrics_integration_remove_alert_rule(idcu_MetricsIntegration* mi, idcu_MetricsIntegrationId id);
idcu_MetricsAlertRule* idcu_metrics_integration_get_alert_rule(idcu_MetricsIntegration* mi, idcu_MetricsIntegrationId id);
int  idcu_metrics_integration_enable_alert_rule(idcu_MetricsIntegration* mi, idcu_MetricsIntegrationId id);
int  idcu_metrics_integration_disable_alert_rule(idcu_MetricsIntegration* mi, idcu_MetricsIntegrationId id);

int  idcu_metrics_integration_set_alert_manager(idcu_MetricsIntegration* mi, idcu_AlertManager* alert_manager);
int  idcu_metrics_integration_set_storage(idcu_MetricsIntegration* mi, idcu_KVStore* storage);

int  idcu_metrics_integration_enable_persistence(idcu_MetricsIntegration* mi);
int  idcu_metrics_integration_disable_persistence(idcu_MetricsIntegration* mi);
int  idcu_metrics_integration_save_metrics(idcu_MetricsIntegration* mi);
int  idcu_metrics_integration_load_metrics(idcu_MetricsIntegration* mi);

int  idcu_metrics_integration_enable_alerting(idcu_MetricsIntegration* mi);
int  idcu_metrics_integration_disable_alerting(idcu_MetricsIntegration* mi);

int  idcu_metrics_integration_query(idcu_MetricsIntegration* mi, const char* name, idcu_Vector* values, uint64_t start_time, uint64_t end_time);
int  idcu_metrics_integration_query_range(idcu_MetricsIntegration* mi, const char* name, idcu_Vector* values, 
                                            uint64_t start_time, uint64_t end_time, uint64_t step_ms);

int  idcu_metrics_integration_aggregate_sum(idcu_MetricsIntegration* mi, const char* name, uint64_t start_time, uint64_t end_time, double* result);
int  idcu_metrics_integration_aggregate_avg(idcu_MetricsIntegration* mi, const char* name, uint64_t start_time, uint64_t end_time, double* result);
int  idcu_metrics_integration_aggregate_min(idcu_MetricsIntegration* mi, const char* name, uint64_t start_time, uint64_t end_time, double* result);
int  idcu_metrics_integration_aggregate_max(idcu_MetricsIntegration* mi, const char* name, uint64_t start_time, uint64_t end_time, double* result);

int  idcu_metrics_integration_reset(idcu_MetricsIntegration* mi);
int  idcu_metrics_integration_reset_metric(idcu_MetricsIntegration* mi, const char* name);

size_t idcu_metrics_integration_count(idcu_MetricsIntegration* mi);
int  idcu_metrics_integration_get_all(idcu_MetricsIntegration* mi, idcu_Vector* metrics);

int  idcu_metrics_alert_rule_init(idcu_MetricsAlertRule* rule);
void idcu_metrics_alert_rule_destroy(idcu_MetricsAlertRule* rule);
int  idcu_metrics_alert_rule_set_name(idcu_MetricsAlertRule* rule, const char* name);
int  idcu_metrics_alert_rule_set_metric(idcu_MetricsAlertRule* rule, const char* metric_name);
int  idcu_metrics_alert_rule_set_threshold(idcu_MetricsAlertRule* rule, double threshold, int operator_type);
int  idcu_metrics_alert_rule_set_topic(idcu_MetricsAlertRule* rule, const char* topic);

int  idcu_metrics_integration_get_info(idcu_MetricsIntegration* mi, char* buffer, size_t buffer_size);
int  idcu_metrics_integration_get_info_json(idcu_MetricsIntegration* mi, char* buffer, size_t buffer_size);

int  idcu_metrics_integration_create_counter(idcu_MetricsIntegration* mi, const char* name, const char* help,
                                              const char** labels, size_t label_count);
int  idcu_metrics_integration_create_gauge(idcu_MetricsIntegration* mi, const char* name, const char* help,
                                            const char** labels, size_t label_count);
int  idcu_metrics_integration_create_histogram(idcu_MetricsIntegration* mi, const char* name, const char* help,
                                                const double* buckets, size_t bucket_count,
                                                const char** labels, size_t label_count);
int  idcu_metrics_integration_create_summary(idcu_MetricsIntegration* mi, const char* name, const char* help,
                                              const double* quantiles, size_t quantile_count,
                                              const char** labels, size_t label_count);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/metrics-integration/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(metrics-integration VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(metrics-integration STATIC
    src/idcu/metrics_integration/metrics_integration.c
    src/idcu/metrics_integration/metrics_alert.c
    src/idcu/metrics_integration/metrics_persistence.c
)

target_include_directories(metrics-integration PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(metrics-integration PRIVATE
    idcu::common
    idcu::metrics
    idcu::alert
    idcu::storage
    idcu::json
    idcu::utils
    idcu::log
)

add_library(idcu::metrics-integration ALIAS metrics-integration)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/metrics-integration/module.yaml`：

```yaml
name: metrics-integration
version: 1.0.0
description: Metrics integration module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-metrics
  - idcu-alert
  - idcu-storage
  - idcu-json
  - idcu-utils
  - idcu-log

build:
  type: cmake
  targets:
    - metrics-integration

headers:
  - idcu/metrics_integration/metrics_integration.h

features:
  - unified: Unified metrics interface
  - collection: Metrics collection integration
  - export: Metrics export integration
  - aggregation: Metrics aggregation
  - alert: Metrics alert
  - persistence: Metrics persistence
  - query: Metrics query
  - prometheus: Prometheus format support
  - custom: Custom metrics registration

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `modules/metrics-integration/README.md`：

```markdown
# metrics-integration

IDCU Agent 的指标集成模块。

## 功能特性

- **统一接口**: 统一的指标接口
- **收集集成**: 指标收集集成
- **导出集成**: 指标导出集成
- **指标聚合**: 指标聚合
- **指标告警**: 指标告警
- **指标持久化**: 指标持久化
- **指标查询**: 指标查询
- **Prometheus**: Prometheus 格式支持
- **自定义注册**: 自定义指标注册

## 快速开始

### 初始化指标集成

```c
#include "idcu/metrics_integration/metrics_integration.h"

idcu_MetricsIntegrationConfig config;
idcu_metrics_integration_config_init(&config);

config.enable_persistence = 1;
config.enable_alerting = 1;
config.persistence_interval_ms = 60000;
config.alert_check_interval_ms = 30000;

idcu_MetricsIntegration mi;
idcu_metrics_integration_init(&mi, &config);
```

### 创建指标

```c
const char* labels[] = {"method", "status"};
idcu_metrics_integration_create_counter(&mi, "http_requests_total", "Total HTTP requests",
                                          labels, 2);

idcu_metrics_integration_create_gauge(&mi, "active_connections", "Active connections",
                                        NULL, 0);

const double buckets[] = {0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1, 2.5, 5, 10};
idcu_metrics_integration_create_histogram(&mi, "http_request_duration_seconds",
                                            "HTTP request duration", buckets, 11, NULL, 0);
```

### 更新指标

```c
idcu_metrics_integration_counter_inc(&mi, "http_requests_total");
idcu_metrics_integration_counter_add(&mi, "http_requests_total", 5);

idcu_metrics_integration_gauge_set(&mi, "active_connections", 100.0);
idcu_metrics_integration_gauge_inc(&mi, "active_connections");
idcu_metrics_integration_gauge_dec(&mi, "active_connections");
idcu_metrics_integration_gauge_add(&mi, "active_connections", 5.0);
idcu_metrics_integration_gauge_sub(&mi, "active_connections", 3.0);

idcu_metrics_integration_histogram_observe(&mi, "http_request_duration_seconds", 0.125);
```

### 导出指标

```c
char buffer[8192];

idcu_metrics_integration_export_prometheus(&mi, buffer, sizeof(buffer));
printf("Prometheus:\n%s\n", buffer);

idcu_metrics_integration_export_json(&mi, buffer, sizeof(buffer));
printf("JSON:\n%s\n", buffer);

idcu_metrics_integration_export(&mi, buffer, sizeof(buffer), IDCU_METRICS_FORMAT_PROMETHEUS);
```

### 添加告警规则

```c
idcu_MetricsAlertRule rule;
idcu_metrics_alert_rule_init(&rule);
idcu_metrics_alert_rule_set_name(&rule, "high_error_rate");
idcu_metrics_alert_rule_set_metric(&rule, "http_errors_total");
idcu_metrics_alert_rule_set_threshold(&rule, 100.0, 1);
idcu_metrics_alert_rule_set_topic(&rule, "alerts");
rule.enabled = 1;

idcu_MetricsIntegrationId rule_id = idcu_metrics_integration_add_alert_rule(&mi, &rule);
```

### 设置告警管理器

```c
idcu_metrics_integration_set_alert_manager(&mi, alert_manager);
```

### 设置存储

```c
idcu_metrics_integration_set_storage(&mi, kvstore);
```

### 启用持久化

```c
idcu_metrics_integration_enable_persistence(&mi);
idcu_metrics_integration_save_metrics(&mi);
idcu_metrics_integration_load_metrics(&mi);
```

### 启用告警

```c
idcu_metrics_integration_enable_alerting(&mi);
```

### 启动指标集成

```c
idcu_metrics_integration_start(&mi);
```

### 查询指标

```c
idcu_Vector values;
idcu_vector_init(&values, sizeof(double));

uint64_t end_time = idcu_time_now_ms();
uint64_t start_time = end_time - 3600000;

idcu_metrics_integration_query(&mi, "http_requests_total", &values, start_time, end_time);

idcu_metrics_integration_query_range(&mi, "http_requests_total", &values,
                                       start_time, end_time, 60000);

idcu_vector_destroy(&values);
```

### 聚合指标

```c
double sum, avg, min, max;

uint64_t end_time = idcu_time_now_ms();
uint64_t start_time = end_time - 3600000;

idcu_metrics_integration_aggregate_sum(&mi, "http_requests_total", start_time, end_time, &sum);
idcu_metrics_integration_aggregate_avg(&mi, "http_request_duration_seconds", start_time, end_time, &avg);
idcu_metrics_integration_aggregate_min(&mi, "active_connections", start_time, end_time, &min);
idcu_metrics_integration_aggregate_max(&mi, "active_connections", start_time, end_time, &max);

printf("Sum: %f\n", sum);
printf("Avg: %f\n", avg);
printf("Min: %f\n", min);
printf("Max: %f\n", max);
```

### 重置指标

```c
idcu_metrics_integration_reset_metric(&mi, "http_requests_total");
idcu_metrics_integration_reset(&mi);
```

### 获取指标信息

```c
size_t count = idcu_metrics_integration_count(&mi);
printf("Metrics count: %zu\n", count);

idcu_Vector metrics;
idcu_vector_init(&metrics, sizeof(idcu_Metric*));

idcu_metrics_integration_get_all(&mi, &metrics);

idcu_vector_destroy(&metrics);
```

### 获取信息

```c
char info_buffer[2048];
idcu_metrics_integration_get_info(&mi, info_buffer, sizeof(info_buffer));
printf("%s\n", info_buffer);

char json_buffer[4096];
idcu_metrics_integration_get_info_json(&mi, json_buffer, sizeof(json_buffer));
printf("%s\n", json_buffer);
```

### 停止指标集成

```c
idcu_metrics_integration_stop(&mi);
idcu_metrics_integration_destroy(&mi);
```

## 指标格式

| 格式 | 说明 |
|-----|------|
| PROMETHEUS | Prometheus 格式 |
| JSON | JSON 格式 |
| TEXT | 文本格式 |

## 告警操作符

| 操作符 | 说明 |
|-------|------|
| 0 | < |
| 1 | <= |
| 2 | == |
| 3 | != |
| 4 | >= |
| 5 | > |

## API 文档

详见 [include/idcu/metrics_integration/metrics_integration.h](include/idcu/metrics_integration/metrics_integration.h)
```

## 验证检查清单

- [ ] 指标集成头文件已创建
- [ ] 指标集成实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以创建和更新指标
- [ ] Prometheus 格式导出正常工作
- [ ] 指标告警正常工作

## Git 提交

```bash
git add modules/metrics-integration/
git commit -m "feat: add metrics-integration module

- Add unified metrics interface
- Add metrics collection integration
- Add metrics export integration
- Add metrics aggregation
- Add metrics alert
- Add metrics persistence
- Add metrics query
- Add Prometheus format support
- Add custom metrics registration
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 指标未更新 | 指标未注册 | 确保指标已注册 |
| 告警未触发 | 规则未启用 | 确保告警规则已启用 |
| 持久化失败 | 存储未设置 | 确保已设置存储 |
