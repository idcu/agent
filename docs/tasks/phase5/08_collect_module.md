# 任务 5.8: collect-module - 数据采集业务模块

## 目标

创建数据采集业务模块，支持：
- 多数据源采集
- 数据过滤和转换
- 数据聚合
- 数据缓存
- 数据导出
- 定时采集
- 告警触发

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/business/collect-module/include/idcu/collect_module
mkdir -p modules/business/collect-module/src/idcu/collect_module
mkdir -p modules/business/collect-module/tests
mkdir -p modules/business/collect-module/examples
```

### 2. 创建采集模块头文件 (collect_module.h)

创建 `modules/business/collect-module/include/idcu/collect_module/collect_module.h`：

```c
#ifndef IDCU_COLLECT_MODULE_COLLECT_MODULE_H
#define IDCU_COLLECT_MODULE_COLLECT_MODULE_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/sdk/sdk.h"
#include "idcu/device_collector/device_collector.h"
#include "idcu/alert/alert.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_DataSourceId;
typedef uint64_t idcu_CollectTaskId;

typedef enum
{
    IDCU_DATA_SOURCE_TYPE_SERVER = 0,
    IDCU_DATA_SOURCE_TYPE_NETWORK,
    IDCU_DATA_SOURCE_TYPE_IOT,
    IDCU_DATA_SOURCE_TYPE_DATABASE,
    IDCU_DATA_SOURCE_TYPE_API,
    IDCU_DATA_SOURCE_TYPE_FILE,
    IDCU_DATA_SOURCE_TYPE_CUSTOM
} idcu_DataSourceType;

typedef enum
{
    IDCU_COLLECT_STATUS_IDLE = 0,
    IDCU_COLLECT_STATUS_RUNNING,
    IDCU_COLLECT_STATUS_SUCCESS,
    IDCU_COLLECT_STATUS_FAILED,
    IDCU_COLLECT_STATUS_PARTIAL
} idcu_CollectStatus;

typedef struct
{
    char name[128];
    char description[512];
    idcu_DataSourceType type;
    char connection_string[1024];
    char credentials[1024];
    uint64_t collect_interval_ms;
    int enabled;
    uint64_t last_collect_time;
    idcu_CollectStatus last_status;
    char last_error[512];
} idcu_DataSource;

typedef struct
{
    char metric_name[128];
    char source_name[128];
    char tags[512];
    double value;
    uint64_t timestamp;
    char raw_data[2048];
} idcu_CollectedMetric;

typedef struct
{
    char name[128];
    char expression[512];
    char description[512];
    int enabled;
} idcu_AlertRule;

typedef struct
{
    idcu_CollectTaskId task_id;
    idcu_DataSourceId source_id;
    uint64_t start_time;
    uint64_t end_time;
    uint64_t duration_ms;
    idcu_CollectStatus status;
    size_t metrics_collected;
    char error_message[512];
} idcu_CollectTask;

typedef struct
{
    char config_path[1024];
    uint64_t default_collect_interval_ms;
    uint64_t data_retention_seconds;
    size_t max_cached_metrics;
    int enable_alerts;
    int enable_data_export;
    char export_path[1024];
} idcu_CollectModuleConfig;

typedef struct
{
    idcu_SdkContext* sdk;
    idcu_CollectModuleConfig config;
    
    idcu_Vector data_sources;
    idcu_HashMap source_map;
    idcu_Mutex sources_lock;
    
    idcu_Vector collected_metrics;
    idcu_Mutex metrics_lock;
    
    idcu_Vector alert_rules;
    idcu_Mutex alerts_lock;
    
    idcu_Vector collect_tasks;
    idcu_Mutex tasks_lock;
    
    int running;
    uint64_t last_cleanup_time;
} idcu_CollectModule;

int  idcu_collect_module_config_init(idcu_CollectModuleConfig* config);

int  idcu_collect_module_init(idcu_CollectModule* module, idcu_SdkContext* sdk,
                                 const idcu_CollectModuleConfig* config);
void idcu_collect_module_destroy(idcu_CollectModule* module);

int  idcu_collect_module_start(idcu_CollectModule* module);
int  idcu_collect_module_stop(idcu_CollectModule* module);

int  idcu_collect_module_load_config(idcu_CollectModule* module, const char* path);
int  idcu_collect_module_save_config(idcu_CollectModule* module, const char* path);

int  idcu_collect_module_add_data_source(idcu_CollectModule* module, 
                                           const idcu_DataSource* source,
                                           idcu_DataSourceId* out_id);
int  idcu_collect_module_remove_data_source(idcu_CollectModule* module, 
                                              idcu_DataSourceId source_id);
idcu_DataSource* idcu_collect_module_get_data_source(idcu_CollectModule* module,
                                                        idcu_DataSourceId source_id);
idcu_DataSource* idcu_collect_module_get_data_source_by_name(idcu_CollectModule* module,
                                                               const char* name);

size_t idcu_collect_module_get_data_source_count(idcu_CollectModule* module);
int  idcu_collect_module_get_all_data_sources(idcu_CollectModule* module, idcu_Vector* sources);

int  idcu_collect_module_enable_data_source(idcu_CollectModule* module, 
                                              idcu_DataSourceId source_id);
int  idcu_collect_module_disable_data_source(idcu_CollectModule* module, 
                                               idcu_DataSourceId source_id);

int  idcu_collect_module_collect_now(idcu_CollectModule* module, 
                                       idcu_DataSourceId source_id,
                                       idcu_CollectTaskId* out_task_id);
int  idcu_collect_module_collect_all_now(idcu_CollectModule* module);

int  idcu_collect_module_get_collected_metrics(idcu_CollectModule* module,
                                                  idcu_DataSourceId source_id,
                                                  uint64_t start_time,
                                                  uint64_t end_time,
                                                  idcu_Vector* metrics);
int  idcu_collect_module_get_latest_metrics(idcu_CollectModule* module,
                                              idcu_DataSourceId source_id,
                                              idcu_Vector* metrics);

int  idcu_collect_module_add_alert_rule(idcu_CollectModule* module,
                                          const idcu_AlertRule* rule);
int  idcu_collect_module_remove_alert_rule(idcu_CollectModule* module,
                                             const char* rule_name);

int  idcu_collect_module_export_metrics(idcu_CollectModule* module,
                                          const char* format,
                                          const char* output_path);

int  idcu_collect_module_get_task_status(idcu_CollectModule* module,
                                           idcu_CollectTaskId task_id,
                                           idcu_CollectTask* task);

int  idcu_collect_module_cleanup_old_data(idcu_CollectModule* module);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建模块配置文件 (module.yaml)

创建 `modules/business/collect-module/module.yaml`：

```yaml
name: collect-module
version: 1.0.0
description: Data collection business module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-sdk
  - idcu-device-collector
  - idcu-alert
  - idcu-log

build:
  type: cmake
  targets:
    - collect-module

headers:
  - idcu/collect_module/collect_module.h

features:
  - multi_source: Multiple data source collection
  - filtering: Data filtering and transformation
  - aggregation: Data aggregation
  - caching: Data caching
  - export: Data export
  - scheduled: Scheduled collection
  - alerts: Alert triggering

testing:
  enabled: true
  framework: internal
```

### 4. 创建 README.md

创建 `modules/business/collect-module/README.md`：

```markdown
# collect-module

IDCU Agent 的数据采集业务模块。

## 功能特性

- **多数据源**: 支持多种数据源采集
- **数据处理**: 数据过滤和转换
- **数据聚合**: 数据聚合
- **数据缓存**: 数据缓存
- **数据导出**: 数据导出
- **定时采集**: 定时采集
- **告警触发**: 告警触发

## 快速开始

### 初始化采集模块

```c
#include "idcu/collect_module/collect_module.h"

idcu_CollectModuleConfig config;
idcu_collect_module_config_init(&config);

config.default_collect_interval_ms = 5000;
config.data_retention_seconds = 86400 * 7;
config.max_cached_metrics = 10000;
config.enable_alerts = 1;

idcu_CollectModule module;
idcu_collect_module_init(&module, sdk_context, &config);
```

### 添加数据源

```c
idcu_DataSource source = {
    .name = "server-01",
    .description = "Main application server",
    .type = IDCU_DATA_SOURCE_TYPE_SERVER,
    .connection_string = "192.168.1.100:9100",
    .collect_interval_ms = 10000,
    .enabled = 1
};

idcu_DataSourceId source_id;
idcu_collect_module_add_data_source(&module, &source, &source_id);
```

### 启动采集

```c
idcu_collect_module_start(&module);
```

### 手动触发采集

```c
idcu_CollectTaskId task_id;
idcu_collect_module_collect_now(&module, source_id, &task_id);
```

### 获取采集数据

```c
idcu_Vector metrics;
idcu_vector_init(&metrics, sizeof(idcu_CollectedMetric));

uint64_t now = time(NULL);
idcu_collect_module_get_collected_metrics(&module, source_id, 
                                             now - 3600, now, &metrics);

for (size_t i = 0; i < metrics.size; i++) {
    idcu_CollectedMetric* metric = (idcu_CollectedMetric*)idcu_vector_get(&metrics, i);
    printf("%s: %.2f\n", metric->metric_name, metric->value);
}

idcu_vector_destroy(&metrics);
```

### 添加告警规则

```c
idcu_AlertRule rule = {
    .name = "high_cpu",
    .expression = "cpu_usage > 80",
    .description = "CPU usage is too high",
    .enabled = 1
};

idcu_collect_module_add_alert_rule(&module, &rule);
```

### 导出数据

```c
idcu_collect_module_export_metrics(&module, "json", "./metrics.json");
```

### 停止采集

```c
idcu_collect_module_stop(&module);
idcu_collect_module_destroy(&module);
```

## 数据源类型

| 类型 | 说明 |
|-----|------|
| SERVER | 服务器 |
| NETWORK | 网络设备 |
| IOT | IoT设备 |
| DATABASE | 数据库 |
| API | API接口 |
| FILE | 文件 |
| CUSTOM | 自定义 |

## 采集状态

| 状态 | 说明 |
|-----|------|
| IDLE | 空闲 |
| RUNNING | 采集中 |
| SUCCESS | 成功 |
| FAILED | 失败 |
| PARTIAL | 部分成功 |

## API 文档

详见 [include/idcu/collect_module/collect_module.h](include/idcu/collect_module/collect_module.h)
```

## 验证检查清单

- [ ] 采集模块头文件已创建
- [ ] 采集模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以添加和管理数据源
- [ ] 数据采集功能正常工作
- [ ] 数据导出功能正常

## Git 提交

```bash
git add modules/business/collect-module/
git commit -m "feat: add collect-module business module

- Add multiple data source collection
- Add data filtering and transformation
- Add data aggregation
- Add data caching
- Add data export
- Add scheduled collection
- Add alert triggering
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 数据源连接失败 | 网络问题或认证错误 | 检查网络连接和认证信息 |
| 采集超时 | 数据源响应慢 | 增加超时时间 |
| 数据丢失 | 缓存配置问题 | 检查缓存配置 |
