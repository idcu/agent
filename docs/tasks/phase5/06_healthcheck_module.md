# 任务 5.6: healthcheck-module - 健康检查业务模块

## 目标

创建健康检查业务模块，支持：
- 健康检查管理
- 自定义健康检查
- 内置健康检查（HTTP、数据库、磁盘、内存、CPU）
- 健康状态聚合
- 健康检查历史
- 健康检查通知
- 健康检查调度
- 健康检查导出
- 健康检查可视化支持

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/healthcheck-module/include/idcu/healthcheck_module
mkdir -p modules/healthcheck-module/src/idcu/healthcheck_module
mkdir -p modules/healthcheck-module/tests
mkdir -p modules/healthcheck-module/examples
```

### 2. 创建健康检查业务模块头文件 (healthcheck_module.h)

创建 `modules/healthcheck-module/include/idcu/healthcheck_module/healthcheck_module.h`：

```c
#ifndef IDCU_HEALTHCHECK_MODULE_HEALTHCHECK_MODULE_H
#define IDCU_HEALTHCHECK_MODULE_HEALTHCHECK_MODULE_H

#include "idcu/common/error_code.h"
#include "idcu/healthcheck/healthcheck.h"
#include "idcu/http/client.h"
#include "idcu/storage/storage.h"
#include "idcu/msgbus/msg_bus.h"
#include "idcu/metrics/metrics.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_HealthCheckModuleId;

typedef enum
{
    IDCU_HEALTHCHECK_TYPE_CUSTOM = 0,
    IDCU_HEALTHCHECK_TYPE_HTTP,
    IDCU_HEALTHCHECK_TYPE_DATABASE,
    IDCU_HEALTHCHECK_TYPE_DISK,
    IDCU_HEALTHCHECK_TYPE_MEMORY,
    IDCU_HEALTHCHECK_TYPE_CPU,
    IDCU_HEALTHCHECK_TYPE_TCP,
    IDCU_HEALTHCHECK_TYPE_DNS
} idcu_HealthCheckType;

typedef struct
{
    idcu_HealthCheckModuleId id;
    char name[128];
    char description[512];
    idcu_HealthCheckType type;
    idcu_HealthCheckFunc func;
    void* user_data;
    
    idcu_HealthStatus last_status;
    char last_message[1024];
    uint64_t last_check_at;
    uint64_t check_interval_ms;
    uint64_t timeout_ms;
    int enabled;
    
    idcu_Vector history;
    size_t max_history_size;
    
    int consecutive_failures;
    int max_consecutive_failures;
    
    union
    {
        struct
        {
            char url[512];
            int timeout_ms;
            int expected_status;
            char expected_body[1024];
            char headers[2048];
        } http;
        
        struct
        {
            char connection_string[512];
            char query[256];
            int timeout_ms;
        } database;
        
        struct
        {
            char path[512];
            uint64_t warning_threshold_bytes;
            uint64_t critical_threshold_bytes;
        } disk;
        
        struct
        {
            double warning_threshold_percent;
            double critical_threshold_percent;
        } memory;
        
        struct
        {
            double warning_threshold_percent;
            double critical_threshold_percent;
            int sample_duration_ms;
        } cpu;
        
        struct
        {
            char host[256];
            uint16_t port;
            int timeout_ms;
        } tcp;
        
        struct
        {
            char host[256];
            char expected_ip[64];
            int timeout_ms;
        } dns;
    } config;
} idcu_HealthCheckModuleCheck;

typedef void (*idcu_HealthCheckModuleCallback)(const idcu_HealthCheckModuleCheck* check, 
                                                 idcu_HealthStatus old_status, 
                                                 idcu_HealthStatus new_status,
                                                 void* user_data);

typedef struct
{
    idcu_Vector checks;
    idcu_HashMap checks_by_id;
    idcu_HashMap checks_by_name;
    idcu_Mutex lock;
    
    idcu_HealthStatus overall_status;
    uint64_t last_aggregate_at;
    
    idcu_Vector callbacks;
    
    idcu_Thread check_thread;
    int running;
    
    idcu_AlertManager* alert_manager;
    idcu_MsgBus* msg_bus;
    char msgbus_topic[256];
    
    idcu_Counter* checks_total_counter;
    idcu_Counter* checks_passed_counter;
    idcu_Counter* checks_failed_counter;
    idcu_Gauge* healthy_checks_gauge;
    
    uint64_t default_interval_ms;
    uint64_t default_timeout_ms;
    size_t default_history_size;
    int default_max_consecutive_failures;
    
    int initialized;
} idcu_HealthCheckModule;

typedef struct
{
    uint64_t default_interval_ms;
    uint64_t default_timeout_ms;
    size_t default_history_size;
    int default_max_consecutive_failures;
    int enable_alerting;
    int enable_msgbus;
    char msgbus_topic[256];
    int enable_metrics;
} idcu_HealthCheckModuleConfig;

int  idcu_healthcheck_module_config_init(idcu_HealthCheckModuleConfig* config);

int  idcu_healthcheck_module_init(idcu_HealthCheckModule* hcm, const idcu_HealthCheckModuleConfig* config);
void idcu_healthcheck_module_destroy(idcu_HealthCheckModule* hcm);
int  idcu_healthcheck_module_start(idcu_HealthCheckModule* hcm);
void idcu_healthcheck_module_stop(idcu_HealthCheckModule* hcm);

idcu_HealthCheckModuleId idcu_healthcheck_module_add_check(idcu_HealthCheckModule* hcm, 
                                                              const idcu_HealthCheckModuleCheck* check);
int  idcu_healthcheck_module_remove_check(idcu_HealthCheckModule* hcm, idcu_HealthCheckModuleId id);
idcu_HealthCheckModuleCheck* idcu_healthcheck_module_get_check(idcu_HealthCheckModule* hcm, idcu_HealthCheckModuleId id);
idcu_HealthCheckModuleCheck* idcu_healthcheck_module_get_check_by_name(idcu_HealthCheckModule* hcm, const char* name);
int  idcu_healthcheck_module_enable_check(idcu_HealthCheckModule* hcm, idcu_HealthCheckModuleId id);
int  idcu_healthcheck_module_disable_check(idcu_HealthCheckModule* hcm, idcu_HealthCheckModuleId id);

idcu_HealthCheckModuleId idcu_healthcheck_module_add_http_check(idcu_HealthCheckModule* hcm, const char* name,
                                                                   const char* url, int timeout_ms, int expected_status);
idcu_HealthCheckModuleId idcu_healthcheck_module_add_database_check(idcu_HealthCheckModule* hcm, const char* name,
                                                                       const char* connection_string, const char* query);
idcu_HealthCheckModuleId idcu_healthcheck_module_add_disk_check(idcu_HealthCheckModule* hcm, const char* name,
                                                                   const char* path, uint64_t warning_bytes, uint64_t critical_bytes);
idcu_HealthCheckModuleId idcu_healthcheck_module_add_memory_check(idcu_HealthCheckModule* hcm, const char* name,
                                                                     double warning_percent, double critical_percent);
idcu_HealthCheckModuleId idcu_healthcheck_module_add_cpu_check(idcu_HealthCheckModule* hcm, const char* name,
                                                                  double warning_percent, double critical_percent);
idcu_HealthCheckModuleId idcu_healthcheck_module_add_tcp_check(idcu_HealthCheckModule* hcm, const char* name,
                                                                  const char* host, uint16_t port, int timeout_ms);
idcu_HealthCheckModuleId idcu_healthcheck_module_add_dns_check(idcu_HealthCheckModule* hcm, const char* name,
                                                                  const char* host, const char* expected_ip);
idcu_HealthCheckModuleId idcu_healthcheck_module_add_custom_check(idcu_HealthCheckModule* hcm, const char* name,
                                                                     idcu_HealthCheckFunc func, void* user_data);

int  idcu_healthcheck_module_check(idcu_HealthCheckModule* hcm, idcu_HealthCheckModuleId id);
int  idcu_healthcheck_module_check_all(idcu_HealthCheckModule* hcm);

idcu_HealthStatus idcu_healthcheck_module_get_overall_status(idcu_HealthCheckModule* hcm);
idcu_HealthStatus idcu_healthcheck_module_get_check_status(idcu_HealthCheckModule* hcm, idcu_HealthCheckModuleId id);

int  idcu_healthcheck_module_add_callback(idcu_HealthCheckModule* hcm, idcu_HealthCheckModuleCallback callback, void* user_data);
int  idcu_healthcheck_module_remove_callback(idcu_HealthCheckModule* hcm, idcu_HealthCheckModuleCallback callback);

int  idcu_healthcheck_module_set_alert_manager(idcu_HealthCheckModule* hcm, idcu_AlertManager* alert_manager);
int  idcu_healthcheck_module_set_msgbus(idcu_HealthCheckModule* hcm, idcu_MsgBus* msg_bus, const char* topic);
int  idcu_healthcheck_module_set_metrics(idcu_HealthCheckModule* hcm, idcu_Counter* total,
                                            idcu_Counter* passed, idcu_Counter* failed,
                                            idcu_Gauge* healthy);

int  idcu_healthcheck_module_get_check_history(idcu_HealthCheckModule* hcm, idcu_HealthCheckModuleId id, idcu_Vector* history);
int  idcu_healthcheck_module_clear_history(idcu_HealthCheckModule* hcm, idcu_HealthCheckModuleId id);
int  idcu_healthcheck_module_clear_all_history(idcu_HealthCheckModule* hcm);

int  idcu_healthcheck_module_get_all_checks(idcu_HealthCheckModule* hcm, idcu_Vector* checks);
size_t idcu_healthcheck_module_check_count(idcu_HealthCheckModule* hcm);
size_t idcu_healthcheck_module_healthy_count(idcu_HealthCheckModule* hcm);
size_t idcu_healthcheck_module_unhealthy_count(idcu_HealthCheckModule* hcm);

int  idcu_healthcheck_module_check_init(idcu_HealthCheckModuleCheck* check);
void idcu_healthcheck_module_check_destroy(idcu_HealthCheckModuleCheck* check);
int  idcu_healthcheck_module_check_set_name(idcu_HealthCheckModuleCheck* check, const char* name);
int  idcu_healthcheck_module_check_set_description(idcu_HealthCheckModuleCheck* check, const char* description);
int  idcu_healthcheck_module_check_set_type(idcu_HealthCheckModuleCheck* check, idcu_HealthCheckType type);
int  idcu_healthcheck_module_check_set_interval(idcu_HealthCheckModuleCheck* check, uint64_t interval_ms);
int  idcu_healthcheck_module_check_set_timeout(idcu_HealthCheckModuleCheck* check, uint64_t timeout_ms);
int  idcu_healthcheck_module_check_set_history_size(idcu_HealthCheckModuleCheck* check, size_t size);
int  idcu_healthcheck_module_check_set_max_failures(idcu_HealthCheckModuleCheck* check, int max_failures);

int  idcu_healthcheck_module_check_http_config(idcu_HealthCheckModuleCheck* check, const char* url,
                                                 int timeout_ms, int expected_status,
                                                 const char* expected_body, const char* headers);
int  idcu_healthcheck_module_check_database_config(idcu_HealthCheckModuleCheck* check, const char* connection_string,
                                                      const char* query, int timeout_ms);
int  idcu_healthcheck_module_check_disk_config(idcu_HealthCheckModuleCheck* check, const char* path,
                                                  uint64_t warning_bytes, uint64_t critical_bytes);
int  idcu_healthcheck_module_check_memory_config(idcu_HealthCheckModuleCheck* check, double warning_percent,
                                                    double critical_percent);
int  idcu_healthcheck_module_check_cpu_config(idcu_HealthCheckModuleCheck* check, double warning_percent,
                                                 double critical_percent, int sample_duration_ms);
int  idcu_healthcheck_module_check_tcp_config(idcu_HealthCheckModuleCheck* check, const char* host,
                                                 uint16_t port, int timeout_ms);
int  idcu_healthcheck_module_check_dns_config(idcu_HealthCheckModuleCheck* check, const char* host,
                                                 const char* expected_ip, int timeout_ms);
int  idcu_healthcheck_module_check_custom_config(idcu_HealthCheckModuleCheck* check, idcu_HealthCheckFunc func,
                                                   void* user_data);

const char* idcu_healthcheck_type_to_string(idcu_HealthCheckType type);

int  idcu_healthcheck_module_export_json(idcu_HealthCheckModule* hcm, char* buffer, size_t buffer_size);
int  idcu_healthcheck_module_export_prometheus(idcu_HealthCheckModule* hcm, char* buffer, size_t buffer_size);
int  idcu_healthcheck_module_get_info(idcu_HealthCheckModule* hcm, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/healthcheck-module/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(healthcheck-module VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(healthcheck-module STATIC
    src/idcu/healthcheck_module/healthcheck_module.c
    src/idcu/healthcheck_module/healthcheck_module_check.c
    src/idcu/healthcheck_module/builtin_checks.c
)

target_include_directories(healthcheck-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(healthcheck-module PRIVATE
    idcu::common
    idcu::healthcheck
    idcu::http-client
    idcu::storage
    idcu::msgbus
    idcu::metrics
    idcu::alert
    idcu::utils
    idcu::log
)

add_library(idcu::healthcheck-module ALIAS healthcheck-module)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/healthcheck-module/module.yaml`：

```yaml
name: healthcheck-module
version: 1.0.0
description: Health check business module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-healthcheck
  - idcu-http-client
  - idcu-storage
  - idcu-msgbus
  - idcu-metrics
  - idcu-alert
  - idcu-utils
  - idcu-log

build:
  type: cmake
  targets:
    - healthcheck-module

headers:
  - idcu/healthcheck_module/healthcheck_module.h

features:
  - management: Health check management
  - custom: Custom health checks
  - builtin: Built-in health checks (HTTP, database, disk, memory, CPU)
  - aggregate: Health status aggregation
  - history: Health check history
  - notification: Health check notification
  - scheduling: Health check scheduling
  - export: Health check export
  - visualization: Health check visualization support

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `modules/healthcheck-module/README.md`：

```markdown
# healthcheck-module

IDCU Agent 的健康检查业务模块。

## 功能特性

- **管理**: 健康检查管理
- **自定义**: 自定义健康检查
- **内置**: 内置健康检查（HTTP、数据库、磁盘、内存、CPU）
- **聚合**: 健康状态聚合
- **历史**: 健康检查历史
- **通知**: 健康检查通知
- **调度**: 健康检查调度
- **导出**: 健康检查导出
- **可视化**: 健康检查可视化支持

## 快速开始

### 初始化健康检查模块

```c
#include "idcu/healthcheck_module/healthcheck_module.h"

idcu_HealthCheckModuleConfig config;
idcu_healthcheck_module_config_init(&config);

config.default_interval_ms = 30000;
config.default_timeout_ms = 10000;
config.default_history_size = 100;
config.default_max_consecutive_failures = 3;
config.enable_alerting = 1;
config.enable_msgbus = 1;
strncpy(config.msgbus_topic, "healthcheck", sizeof(config.msgbus_topic));
config.enable_metrics = 1;

idcu_HealthCheckModule hcm;
idcu_healthcheck_module_init(&hcm, &config);
```

### 启动健康检查模块

```c
idcu_healthcheck_module_start(&hcm);
```

### 添加 HTTP 健康检查

```c
idcu_HealthCheckModuleId http_id = idcu_healthcheck_module_add_http_check(
    &hcm, "api-server", "https://api.example.com/health", 5000, 200);
```

### 添加数据库健康检查

```c
idcu_HealthCheckModuleId db_id = idcu_healthcheck_module_add_database_check(
    &hcm, "database", "sqlite://./data.db", "SELECT 1");
```

### 添加磁盘健康检查

```c
idcu_HealthCheckModuleId disk_id = idcu_healthcheck_module_add_disk_check(
    &hcm, "disk", "/", 10 * 1024 * 1024 * 1024, 5 * 1024 * 1024 * 1024);
```

### 添加内存健康检查

```c
idcu_HealthCheckModuleId memory_id = idcu_healthcheck_module_add_memory_check(
    &hcm, "memory", 80.0, 95.0);
```

### 添加 CPU 健康检查

```c
idcu_HealthCheckModuleId cpu_id = idcu_healthcheck_module_add_cpu_check(
    &hcm, "cpu", 80.0, 95.0);
```

### 添加 TCP 健康检查

```c
idcu_HealthCheckModuleId tcp_id = idcu_healthcheck_module_add_tcp_check(
    &hcm, "redis", "localhost", 6379, 5000);
```

### 添加 DNS 健康检查

```c
idcu_HealthCheckModuleId dns_id = idcu_healthcheck_module_add_dns_check(
    &hcm, "dns", "api.example.com", "192.168.1.100");
```

### 添加自定义健康检查

```c
int custom_check(void* user_data)
{
    // Custom health check logic
    return IDCU_ERR_OK;
}

idcu_HealthCheckModuleId custom_id = idcu_healthcheck_module_add_custom_check(
    &hcm, "custom", custom_check, NULL);
```

### 创建完整检查配置

```c
idcu_HealthCheckModuleCheck check;
idcu_healthcheck_module_check_init(&check);

idcu_healthcheck_module_check_set_name(&check, "My Check");
idcu_healthcheck_module_check_set_description(&check, "My custom health check");
idcu_healthcheck_module_check_set_type(&check, IDCU_HEALTHCHECK_TYPE_HTTP);
idcu_healthcheck_module_check_set_interval(&check, 60000);
idcu_healthcheck_module_check_set_timeout(&check, 10000);
idcu_healthcheck_module_check_set_history_size(&check, 50);
idcu_healthcheck_module_check_set_max_failures(&check, 5);
idcu_healthcheck_module_check_http_config(&check, "https://api.example.com/health", 5000, 200, NULL, NULL);

idcu_HealthCheckModuleId check_id = idcu_healthcheck_module_add_check(&hcm, &check);
idcu_healthcheck_module_check_destroy(&check);
```

### 添加状态变更回调

```c
void health_changed(const idcu_HealthCheckModuleCheck* check,
                     idcu_HealthStatus old_status,
                     idcu_HealthStatus new_status,
                     void* user_data)
{
    printf("Check %s status changed: %s -> %s\n",
           check->name,
           idcu_health_status_to_string(old_status),
           idcu_health_status_to_string(new_status));
}

idcu_healthcheck_module_add_callback(&hcm, health_changed, NULL);
```

### 设置告警管理器

```c
idcu_healthcheck_module_set_alert_manager(&hcm, alert_manager);
```

### 设置消息总线

```c
idcu_healthcheck_module_set_msgbus(&hcm, msg_bus, "healthcheck");
```

### 设置指标

```c
idcu_healthcheck_module_set_metrics(&hcm, total_counter, passed_counter,
                                       failed_counter, healthy_gauge);
```

### 手动检查

```c
idcu_healthcheck_module_check(&hcm, check_id);
idcu_healthcheck_module_check_all(&hcm);
```

### 获取整体健康状态

```c
idcu_HealthStatus overall = idcu_healthcheck_module_get_overall_status(&hcm);
printf("Overall health: %s\n", idcu_health_status_to_string(overall));
```

### 获取检查状态

```c
idcu_HealthStatus status = idcu_healthcheck_module_get_check_status(&hcm, check_id);
printf("Check status: %s\n", idcu_health_status_to_string(status));
```

### 获取检查

```c
idcu_HealthCheckModuleCheck* check = idcu_healthcheck_module_get_check(&hcm, check_id);
if (check) {
    printf("Check name: %s\n", check->name);
    printf("Last status: %s\n", idcu_health_status_to_string(check->last_status));
    printf("Last message: %s\n", check->last_message);
}
```

### 启用/禁用检查

```c
idcu_healthcheck_module_enable_check(&hcm, check_id);
idcu_healthcheck_module_disable_check(&hcm, check_id);
```

### 获取检查历史

```c
idcu_Vector history;
idcu_vector_init(&history, sizeof(idcu_HealthStatus));

idcu_healthcheck_module_get_check_history(&hcm, check_id, &history);

idcu_vector_destroy(&history);
```

### 清空历史

```c
idcu_healthcheck_module_clear_history(&hcm, check_id);
idcu_healthcheck_module_clear_all_history(&hcm);
```

### 获取所有检查

```c
idcu_Vector checks;
idcu_vector_init(&checks, sizeof(idcu_HealthCheckModuleId));

idcu_healthcheck_module_get_all_checks(&hcm, &checks);

printf("Total checks: %zu\n", idcu_healthcheck_module_check_count(&hcm));
printf("Healthy checks: %zu\n", idcu_healthcheck_module_healthy_count(&hcm));
printf("Unhealthy checks: %zu\n", idcu_healthcheck_module_unhealthy_count(&hcm));

idcu_vector_destroy(&checks);
```

### 移除检查

```c
idcu_healthcheck_module_remove_check(&hcm, check_id);
```

### 移除回调

```c
idcu_healthcheck_module_remove_callback(&hcm, health_changed);
```

### 导出为 JSON

```c
char json_buffer[8192];
idcu_healthcheck_module_export_json(&hcm, json_buffer, sizeof(json_buffer));
printf("%s\n", json_buffer);
```

### 导出为 Prometheus 格式

```c
char prom_buffer[8192];
idcu_healthcheck_module_export_prometheus(&hcm, prom_buffer, sizeof(prom_buffer));
printf("%s\n", prom_buffer);
```

### 获取信息

```c
char info_buffer[4096];
idcu_healthcheck_module_get_info(&hcm, info_buffer, sizeof(info_buffer));
printf("%s\n", info_buffer);
```

### 停止健康检查模块

```c
idcu_healthcheck_module_stop(&hcm);
idcu_healthcheck_module_destroy(&hcm);
```

## 健康检查类型

| 类型 | 说明 |
|-----|------|
| CUSTOM | 自定义 |
| HTTP | HTTP |
| DATABASE | 数据库 |
| DISK | 磁盘 |
| MEMORY | 内存 |
| CPU | CPU |
| TCP | TCP |
| DNS | DNS |

## 健康状态

| 状态 | 说明 |
|-----|------|
| PASSING | 通过 |
| WARNING | 警告 |
| FAILING | 失败 |
| UNKNOWN | 未知 |

## API 文档

详见 [include/idcu/healthcheck_module/healthcheck_module.h](include/idcu/healthcheck_module/healthcheck_module.h)
```

## 验证检查清单

- [ ] 健康检查业务模块头文件已创建
- [ ] 健康检查业务模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以添加和执行健康检查
- [ ] 内置健康检查正常工作
- [ ] 健康状态聚合正常工作

## Git 提交

```bash
git add modules/healthcheck-module/
git commit -m "feat: add healthcheck-module module

- Add health check management
- Add custom health checks
- Add built-in health checks (HTTP, database, disk, memory, CPU)
- Add health status aggregation
- Add health check history
- Add health check notification
- Add health check scheduling
- Add health check export
- Add health check visualization support
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 检查未执行 | 检查未启用 | 确保检查已启用 |
| HTTP 检查失败 | URL 不可访问 | 检查网络连接和 URL |
| 数据库检查失败 | 连接错误 | 检查连接字符串 |
