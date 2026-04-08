# 任务 3.16: idcu-healthcheck - 健康检查库

## 目标

创建健康检查库，支持：
- 自定义健康检查
- HTTP 健康检查端点
- 数据库健康检查
- 系统资源检查
- 健康状态聚合
- 检查结果缓存

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-healthcheck/include/idcu/healthcheck
mkdir -p libs/idcu-healthcheck/src/idcu/healthcheck
mkdir -p libs/idcu-healthcheck/tests
mkdir -p libs/idcu-healthcheck/examples
```

### 2. 创建健康检查头文件 (healthcheck.h)

创建 `libs/idcu-healthcheck/include/idcu/healthcheck/healthcheck.h`：

```c
#ifndef IDCU_HEALTHCHECK_HEALTHCHECK_H
#define IDCU_HEALTHCHECK_HEALTHCHECK_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/lock.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_HEALTH_STATUS_PASS = 0,
    IDCU_HEALTH_STATUS_WARN,
    IDCU_HEALTH_STATUS_FAIL,
    IDCU_HEALTH_STATUS_UNKNOWN
} idcu_HealthStatus;

typedef enum
{
    IDCU_HEALTH_CHECK_TYPE_CUSTOM = 0,
    IDCU_HEALTH_CHECK_TYPE_HTTP,
    IDCU_HEALTH_CHECK_TYPE_DATABASE,
    IDCU_HEALTH_CHECK_TYPE_DISK,
    IDCU_HEALTH_CHECK_TYPE_MEMORY,
    IDCU_HEALTH_CHECK_TYPE_CPU
} idcu_HealthCheckType;

typedef struct
{
    char name[128];
    idcu_HealthStatus status;
    char message[512];
    char component_id[128];
    char component_type[128];
    uint64_t measured_at;
    double observed_value;
    char observed_unit[32];
    char links[256];
    char output[1024];
} idcu_HealthCheckResult;

typedef int (*idcu_HealthCheckFunc)(void* user_data, idcu_HealthCheckResult* result);

typedef struct idcu_HealthCheck
{
    char name[128];
    idcu_HealthCheckType type;
    idcu_HealthCheckFunc check_func;
    void* user_data;
    uint64_t interval_ms;
    uint64_t timeout_ms;
    uint64_t last_checked_at;
    idcu_HealthCheckResult last_result;
    int enabled;
} idcu_HealthCheck;

typedef struct
{
    idcu_Vector checks;
    idcu_Mutex lock;
    uint64_t overall_checked_at;
    idcu_HealthStatus overall_status;
    int initialized;
} idcu_HealthChecker;

typedef struct
{
    char url[1024];
    int timeout_ms;
    int expected_status_code;
    char expected_content[512];
    char headers[1024];
} idcu_HttpCheckConfig;

typedef struct
{
    char connection_string[1024];
    char query[256];
    int timeout_ms;
} idcu_DatabaseCheckConfig;

typedef struct
{
    char path[1024];
    uint64_t min_free_bytes;
    double min_free_percent;
} idcu_DiskCheckConfig;

typedef struct
{
    uint64_t min_free_bytes;
    double min_free_percent;
} idcu_MemoryCheckConfig;

typedef struct
{
    double max_usage_percent;
} idcu_CpuCheckConfig;

int  idcu_healthchecker_init(idcu_HealthChecker* checker);
void idcu_healthchecker_destroy(idcu_HealthChecker* checker);

int  idcu_healthchecker_add_check(idcu_HealthChecker* checker, const idcu_HealthCheck* check);
int  idcu_healthchecker_remove_check(idcu_HealthChecker* checker, const char* name);
idcu_HealthCheck* idcu_healthchecker_get_check(idcu_HealthChecker* checker, const char* name);
int  idcu_healthchecker_enable_check(idcu_HealthChecker* checker, const char* name);
int  idcu_healthchecker_disable_check(idcu_HealthChecker* checker, const char* name);

int  idcu_healthchecker_check_all(idcu_HealthChecker* checker);
int  idcu_healthchecker_check_one(idcu_HealthChecker* checker, const char* name);
idcu_HealthStatus idcu_healthchecker_get_overall_status(idcu_HealthChecker* checker);

int  idcu_healthchecker_get_results(idcu_HealthChecker* checker, idcu_Vector* results);
int  idcu_healthchecker_to_json(idcu_HealthChecker* checker, char* buffer, size_t buffer_size);

int  idcu_healthcheck_init(idcu_HealthCheck* check, const char* name, idcu_HealthCheckType type);
void idcu_healthcheck_destroy(idcu_HealthCheck* check);
int  idcu_healthcheck_set_custom(idcu_HealthCheck* check, idcu_HealthCheckFunc func, void* user_data);
int  idcu_healthcheck_set_http(idcu_HealthCheck* check, const idcu_HttpCheckConfig* config);
int  idcu_healthcheck_set_database(idcu_HealthCheck* check, const idcu_DatabaseCheckConfig* config);
int  idcu_healthcheck_set_disk(idcu_HealthCheck* check, const idcu_DiskCheckConfig* config);
int  idcu_healthcheck_set_memory(idcu_HealthCheck* check, const idcu_MemoryCheckConfig* config);
int  idcu_healthcheck_set_cpu(idcu_HealthCheck* check, const idcu_CpuCheckConfig* config);
int  idcu_healthcheck_set_interval(idcu_HealthCheck* check, uint64_t interval_ms);
int  idcu_healthcheck_set_timeout(idcu_HealthCheck* check, uint64_t timeout_ms);

int  idcu_healthcheck_result_init(idcu_HealthCheckResult* result);
void idcu_healthcheck_result_destroy(idcu_HealthCheckResult* result);
int  idcu_healthcheck_result_set_status(idcu_HealthCheckResult* result, idcu_HealthStatus status);
int  idcu_healthcheck_result_set_message(idcu_HealthCheckResult* result, const char* message);
int  idcu_healthcheck_result_set_component(idcu_HealthCheckResult* result, const char* component_id, const char* component_type);
int  idcu_healthcheck_result_set_observed_value(idcu_HealthCheckResult* result, double value, const char* unit);
int  idcu_healthcheck_result_set_links(idcu_HealthCheckResult* result, const char* links);
int  idcu_healthcheck_result_set_output(idcu_HealthCheckResult* result, const char* output);
const char* idcu_health_status_to_string(idcu_HealthStatus status);

int idcu_http_check(const idcu_HttpCheckConfig* config, idcu_HealthCheckResult* result);
int idcu_database_check(const idcu_DatabaseCheckConfig* config, idcu_HealthCheckResult* result);
int idcu_disk_check(const idcu_DiskCheckConfig* config, idcu_HealthCheckResult* result);
int idcu_memory_check(const idcu_MemoryCheckConfig* config, idcu_HealthCheckResult* result);
int idcu_cpu_check(const idcu_CpuCheckConfig* config, idcu_HealthCheckResult* result);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-healthcheck/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-healthcheck VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-healthcheck STATIC
    src/idcu/healthcheck/healthcheck.c
)

target_include_directories(idcu-healthcheck PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-healthcheck PRIVATE
    idcu::common
    idcu::http-client
    idcu::json
)

add_library(idcu::healthcheck ALIAS idcu-healthcheck)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-healthcheck/module.yaml`：

```yaml
name: idcu-healthcheck
version: 1.0.0
description: Health check library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-http-client
  - idcu-json

build:
  type: cmake
  targets:
    - idcu-healthcheck

headers:
  - idcu/healthcheck/healthcheck.h

features:
  - custom: Custom health checks
  - http: HTTP health checks
  - database: Database health checks
  - disk: Disk space checks
  - memory: Memory usage checks
  - cpu: CPU usage checks
  - aggregate: Health status aggregation
  - cache: Check result caching
  - json: JSON format output (RFC draft-inadarei-api-health-check)

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-healthcheck/README.md`：

```markdown
# idcu-healthcheck

IDCU Agent 的健康检查库。

## 功能特性

- **自定义检查**: 自定义健康检查
- **HTTP 检查**: HTTP 健康检查
- **数据库检查**: 数据库健康检查
- **磁盘检查**: 磁盘空间检查
- **内存检查**: 内存使用检查
- **CPU 检查**: CPU 使用检查
- **状态聚合**: 健康状态聚合
- **结果缓存**: 检查结果缓存
- **JSON 输出**: JSON 格式输出（RFC 标准）

## 快速开始

### 初始化健康检查器

```c
#include "idcu/healthcheck/healthcheck.h"

idcu_HealthChecker checker;
idcu_healthchecker_init(&checker);
```

### 添加 HTTP 检查

```c
idcu_HealthCheck http_check;
idcu_healthcheck_init(&http_check, "api", IDCU_HEALTH_CHECK_TYPE_HTTP);

idcu_HttpCheckConfig http_config;
strncpy(http_config.url, "http://localhost:8080/health", sizeof(http_config.url));
http_config.timeout_ms = 5000;
http_config.expected_status_code = 200;

idcu_healthcheck_set_http(&http_check, &http_config);
idcu_healthcheck_set_interval(&http_check, 30000);
idcu_healthcheck_set_timeout(&http_check, 5000);

idcu_healthchecker_add_check(&checker, &http_check);
```

### 添加数据库检查

```c
idcu_HealthCheck db_check;
idcu_healthcheck_init(&db_check, "database", IDCU_HEALTH_CHECK_TYPE_DATABASE);

idcu_DatabaseCheckConfig db_config;
strncpy(db_config.connection_string, "sqlite://./data.db", sizeof(db_config.connection_string));
strncpy(db_config.query, "SELECT 1", sizeof(db_config.query));
db_config.timeout_ms = 5000;

idcu_healthcheck_set_database(&db_check, &db_config);
idcu_healthchecker_add_check(&checker, &db_check);
```

### 添加磁盘检查

```c
idcu_HealthCheck disk_check;
idcu_healthcheck_init(&disk_check, "disk", IDCU_HEALTH_CHECK_TYPE_DISK);

idcu_DiskCheckConfig disk_config;
strncpy(disk_config.path, "/", sizeof(disk_config.path));
disk_config.min_free_percent = 10.0;

idcu_healthcheck_set_disk(&disk_check, &disk_config);
idcu_healthchecker_add_check(&checker, &disk_check);
```

### 添加内存检查

```c
idcu_HealthCheck memory_check;
idcu_healthcheck_init(&memory_check, "memory", IDCU_HEALTH_CHECK_TYPE_MEMORY);

idcu_MemoryCheckConfig memory_config;
memory_config.min_free_percent = 5.0;

idcu_healthcheck_set_memory(&memory_check, &memory_config);
idcu_healthchecker_add_check(&checker, &memory_check);
```

### 执行所有检查

```c
idcu_healthchecker_check_all(&checker);

idcu_HealthStatus overall = idcu_healthchecker_get_overall_status(&checker);
printf("Overall status: %s\n", idcu_health_status_to_string(overall));
```

### 输出 JSON 格式

```c
char json_buffer[8192];
idcu_healthchecker_to_json(&checker, json_buffer, sizeof(json_buffer));
printf("%s\n", json_buffer);
```

### 自定义检查

```c
int custom_check(void* user_data, idcu_HealthCheckResult* result)
{
    int is_healthy = 1;
    
    if (is_healthy) {
        idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_PASS);
        idcu_healthcheck_result_set_message(result, "Custom check passed");
    } else {
        idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_FAIL);
        idcu_healthcheck_result_set_message(result, "Custom check failed");
    }
    
    return IDCU_ERR_OK;
}

idcu_HealthCheck custom;
idcu_healthcheck_init(&custom, "custom", IDCU_HEALTH_CHECK_TYPE_CUSTOM);
idcu_healthcheck_set_custom(&custom, custom_check, NULL);
idcu_healthchecker_add_check(&checker, &custom);
```

### 销毁健康检查器

```c
idcu_healthchecker_destroy(&checker);
```

## 健康状态

| 状态 | 说明 |
|-----|------|
| PASS | 通过 |
| WARN | 警告 |
| FAIL | 失败 |
| UNKNOWN | 未知 |

## API 文档

详见 [include/idcu/healthcheck/healthcheck.h](include/idcu/healthcheck/healthcheck.h)
```

## 验证检查清单

- [ ] 健康检查头文件已创建
- [ ] 健康检查实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以添加和执行健康检查
- [ ] 可以输出 JSON 格式结果
- [ ] 健康状态聚合正常工作

## Git 提交

```bash
git add libs/idcu-healthcheck/
git commit -m "feat: add idcu-healthcheck library

- Add custom health checks
- Add HTTP health checks
- Add database health checks
- Add disk space checks
- Add memory usage checks
- Add CPU usage checks
- Add health status aggregation
- Add check result caching
- Add JSON format output
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 检查超时 | 超时时间太短 | 增加 timeout_ms 值 |
| HTTP 检查失败 | URL 不可达 | 检查 URL 和网络连接 |
| 状态聚合错误 | 检查未正确执行 | 确保所有检查都已执行 |
