# 任务 3.16: idcu-healthcheck - 健康检查库

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的健康检查库，支持自定义健康检查、HTTP 健康检查端点、数据库健康检查、系统资源检查（磁盘、内存、CPU）、健康状态聚合、检查结果缓存，并遵循 RFC draft-inadarei-api-health-check 标准输出 JSON 格式，满足单次健康检查响应时间 ≤ 100ms 的性能要求。

### 1.2 不做什么
- 不实现 HTTPS 健康检查（后续可扩展）
- 不实现分布式健康检查
- 不实现健康检查历史记录持久化

### 1.3 输入
- 健康检查配置：检查类型、URL、超时时间、阈值等
- 自定义检查函数指针
- 健康检查器操作命令（添加、移除、启用、禁用检查）

### 1.4 输出
- 健康状态（PASS/WARN/FAIL/UNKNOWN）
- 完整的健康检查结果（JSON 格式）
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用
- idcu-http-client 库已可用
- idcu-json 库已可用
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **健康状态**: 4 级枚举（PASS &lt; WARN &lt; FAIL &lt; UNKNOWN）
- **检查类型**: 自定义、HTTP、数据库、磁盘、内存、CPU
- **结果缓存**: 基于检查间隔的缓存机制
- **JSON 输出**: 遵循 RFC draft-inadarei-api-health-check 标准
- **线程安全**: 使用互斥锁保护健康检查器状态

### 2.2 核心逻辑
```
1. 初始化健康检查器
   a. 初始化检查列表和互斥锁
   b. 设置整体状态为 UNKNOWN

2. 管理健康检查
   a. 添加检查项到列表
   b. 移除、启用、禁用检查项
   c. 设置检查间隔和超时时间

3. 执行健康检查
   a. 检查所有或单个检查项
   b. 对于每个检查，调用对应类型的检查函数
   c. 缓存检查结果，记录检查时间
   d. 聚合所有检查结果，计算整体状态

4. 输出结果
   a. 聚合所有检查结果
   b. 转换为标准 JSON 格式
   c. 返回整体状态

5. 销毁健康检查器
   a. 清理所有检查项
   b. 销毁互斥锁
```

### 2.3 数据结构/接口
```c
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

// 核心 API
int  idcu_healthchecker_init(idcu_HealthChecker* checker);
void idcu_healthchecker_destroy(idcu_HealthChecker* checker);
int  idcu_healthchecker_add_check(idcu_HealthChecker* checker, const idcu_HealthCheck* check);
int  idcu_healthchecker_remove_check(idcu_HealthChecker* checker, const char* name);
int  idcu_healthchecker_check_all(idcu_HealthChecker* checker);
idcu_HealthStatus idcu_healthchecker_get_overall_status(idcu_HealthChecker* checker);
int  idcu_healthchecker_to_json(idcu_HealthChecker* checker, char* buffer, size_t buffer_size);

// 检查配置和设置
int  idcu_healthcheck_init(idcu_HealthCheck* check, const char* name, idcu_HealthCheckType type);
int  idcu_healthcheck_set_http(idcu_HealthCheck* check, const idcu_HttpCheckConfig* config);
int  idcu_healthcheck_set_database(idcu_HealthCheck* check, const idcu_DatabaseCheckConfig* config);
int  idcu_healthcheck_set_disk(idcu_HealthCheck* check, const idcu_DiskCheckConfig* config);
int  idcu_healthcheck_set_memory(idcu_HealthCheck* check, const idcu_MemoryCheckConfig* config);
int  idcu_healthcheck_set_cpu(idcu_HealthCheck* check, const idcu_CpuCheckConfig* config);
```

### 2.4 跨平台适配
- **磁盘检查**: Windows 使用 `GetDiskFreeSpaceExW`，Linux 使用 `statvfs`
- **内存检查**: Windows 使用 `GlobalMemoryStatusEx`，Linux 使用 `/proc/meminfo`
- **CPU 检查**: Windows 使用 `GetSystemTimes`，Linux 使用 `/proc/stat`
- **文件路径**: Windows 使用反斜杠 `\`，Linux 使用正斜杠 `/`

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以添加自定义健康检查
- [ ] 可以添加 HTTP 健康检查
- [ ] 可以添加数据库健康检查
- [ ] 可以添加磁盘空间检查
- [ ] 可以添加内存使用检查
- [ ] 可以添加 CPU 使用检查
- [ ] 可以启用和禁用检查项
- [ ] 健康状态聚合正常工作（FAIL &gt; WARN &gt; PASS）
- [ ] 检查结果缓存机制正常工作
- [ ] 可以输出符合 RFC 标准的 JSON 格式结果

### 3.2 性能验收
- 单个健康检查响应时间 ≤ 100ms
- 同时执行 10 个健康检查的总时间 ≤ 500ms
- 健康检查器初始化时间 ≤ 10ms
- JSON 输出生成时间 ≤ 5ms
- 内存占用 ≤ 256KB（10 个检查项）

### 3.3 异常验收
- [ ] 初始化失败返回明确错误码
- [ ] 传入 NULL 参数时安全处理
- [ ] 检查超时（HTTP、数据库）返回 WARN/FAIL 状态
- [ ] 网络不可达时 HTTP 检查返回 FAIL 状态
- [ ] 多次调用 destroy 不崩溃

---

## 4. 执行计划

### 4.1 工期
3 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义（45 分钟）
- D1-45: 完成核心实现（健康检查器管理和聚合）（1 小时）
- D1-105: 完成各类检查实现（HTTP、数据库、磁盘、内存、CPU）（45 分钟）
- D1-150: 完成单元测试（30 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 网络编程 + 跨平台开发）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，结构体前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释

### 5.2 测试要求
- 单元测试覆盖率 ≥ 85%
- 测试用例覆盖：初始化、添加各类检查、执行检查、状态聚合、JSON 输出、异常场景
- 性能测试用例验证响应时间指标

### 5.3 部署指引
- 编译命令：`cmake -B build &amp;&amp; cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::healthcheck)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：HTTP 检查超时导致整体检查时间过长  
应对：为每个检查设置独立的超时时间，并在超时后返回 WARN/FAIL 状态，不阻塞其他检查

### 6.2 风险2
描述：跨平台系统资源检查实现差异大  
应对：分别实现 Windows 和 Linux 版本，使用条件编译区分，确保在两个平台上功能一致

---

## 7. 详细实现步骤

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
#include &lt;stddef.h&gt;
#include &lt;stdint.h&gt;

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
    $&lt;BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include&gt;
    $&lt;INSTALL_INTERFACE:include&gt;
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
idcu_healthchecker_init(&amp;checker);
```

### 添加 HTTP 检查

```c
idcu_HealthCheck http_check;
idcu_healthcheck_init(&amp;http_check, "api", IDCU_HEALTH_CHECK_TYPE_HTTP);

idcu_HttpCheckConfig http_config;
strncpy(http_config.url, "http://localhost:8080/health", sizeof(http_config.url));
http_config.timeout_ms = 5000;
http_config.expected_status_code = 200;

idcu_healthcheck_set_http(&amp;http_check, &amp;http_config);
idcu_healthcheck_set_interval(&amp;http_check, 30000);
idcu_healthcheck_set_timeout(&amp;http_check, 5000);

idcu_healthchecker_add_check(&amp;checker, &amp;http_check);
```

### 添加数据库检查

```c
idcu_HealthCheck db_check;
idcu_healthcheck_init(&amp;db_check, "database", IDCU_HEALTH_CHECK_TYPE_DATABASE);

idcu_DatabaseCheckConfig db_config;
strncpy(db_config.connection_string, "sqlite://./data.db", sizeof(db_config.connection_string));
strncpy(db_config.query, "SELECT 1", sizeof(db_config.query));
db_config.timeout_ms = 5000;

idcu_healthcheck_set_database(&amp;db_check, &amp;db_config);
idcu_healthchecker_add_check(&amp;checker, &amp;db_check);
```

### 添加磁盘检查

```c
idcu_HealthCheck disk_check;
idcu_healthcheck_init(&amp;disk_check, "disk", IDCU_HEALTH_CHECK_TYPE_DISK);

idcu_DiskCheckConfig disk_config;
strncpy(disk_config.path, "/", sizeof(disk_config.path));
disk_config.min_free_percent = 10.0;

idcu_healthcheck_set_disk(&amp;disk_check, &amp;disk_config);
idcu_healthchecker_add_check(&amp;checker, &amp;disk_check);
```

### 添加内存检查

```c
idcu_HealthCheck memory_check;
idcu_healthcheck_init(&amp;memory_check, "memory", IDCU_HEALTH_CHECK_TYPE_MEMORY);

idcu_MemoryCheckConfig memory_config;
memory_config.min_free_percent = 5.0;

idcu_healthcheck_set_memory(&amp;memory_check, &amp;memory_config);
idcu_healthchecker_add_check(&amp;checker, &amp;memory_check);
```

### 执行所有检查

```c
idcu_healthchecker_check_all(&amp;checker);

idcu_HealthStatus overall = idcu_healthchecker_get_overall_status(&amp;checker);
printf("Overall status: %s\n", idcu_health_status_to_string(overall));
```

### 输出 JSON 格式

```c
char json_buffer[8192];
idcu_healthchecker_to_json(&amp;checker, json_buffer, sizeof(json_buffer));
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
idcu_healthcheck_init(&amp;custom, "custom", IDCU_HEALTH_CHECK_TYPE_CUSTOM);
idcu_healthcheck_set_custom(&amp;custom, custom_check, NULL);
idcu_healthchecker_add_check(&amp;checker, &amp;custom);
```

### 销毁健康检查器

```c
idcu_healthchecker_destroy(&amp;checker);
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

---

## 8. 验证检查清单

- [ ] 健康检查头文件已创建
- [ ] 健康检查实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以添加和执行健康检查
- [ ] 可以输出 JSON 格式结果
- [ ] 健康状态聚合正常工作
- [ ] 单元测试通过率 100%
- [ ] 性能测试达标（单次检查 ≤ 100ms）
- [ ] 跨平台测试通过（Windows + Linux）
- [ ] 已提交 Git

---

## 9. Git 提交

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
- Add JSON format output (RFC draft-inadarei-api-health-check)
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 检查超时 | 超时时间太短 | 增加 timeout_ms 值 |
| HTTP 检查失败 | URL 不可达 | 检查 URL 和网络连接 |
| 状态聚合错误 | 检查未正确执行 | 确保所有检查都已执行 |
| JSON 输出不完整 | 缓冲区太小 | 增大 buffer_size 值 |
| 磁盘检查在 Windows 上失败 | 路径格式错误 | 使用 Windows 风格的路径（如 C:\\） |
