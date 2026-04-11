# idcu-healthcheck API 文档

用于监控系统健康状态的健康检查库。

## 健康检查器

```c
typedef struct idcu_HealthChecker idcu_HealthChecker;
```

### 健康检查器函数

```c
int idcu_healthchecker_init(idcu_HealthChecker* checker);
void idcu_healthchecker_destroy(idcu_HealthChecker* checker);

int idcu_healthchecker_add_check(idcu_HealthChecker* checker, const idcu_HealthCheck* check);
int idcu_healthchecker_remove_check(idcu_HealthChecker* checker, const char* name);

int idcu_healthchecker_check_all(idcu_HealthChecker* checker);
idcu_HealthStatus idcu_healthchecker_get_overall_status(idcu_HealthChecker* checker);

int idcu_healthchecker_to_json(idcu_HealthChecker* checker, char* buffer, size_t buffer_size);
```

## 健康检查

```c
typedef struct idcu_HealthCheck idcu_HealthCheck;
```

### 健康检查函数

```c
int idcu_healthcheck_init(idcu_HealthCheck* check, const char* name, idcu_HealthCheckType type);
void idcu_healthcheck_destroy(idcu_HealthCheck* check);

int idcu_healthcheck_set_custom(idcu_HealthCheck* check, idcu_HealthCheckFunc func, void* user_data);
int idcu_healthcheck_set_http(idcu_HealthCheck* check, const char* url, int timeout_ms, int expected_status);
int idcu_healthcheck_set_disk(idcu_HealthCheck* check, const char* path, uint64_t min_free_bytes, double min_free_percent);
int idcu_healthcheck_set_memory(idcu_HealthCheck* check, uint64_t min_free_bytes, double min_free_percent);
int idcu_healthcheck_set_cpu(idcu_HealthCheck* check, double max_usage_percent);

int idcu_healthcheck_set_interval(idcu_HealthCheck* check, uint64_t interval_ms);
int idcu_healthcheck_set_timeout(idcu_HealthCheck* check, uint64_t timeout_ms);
```

## 健康检查结果

```c
typedef struct idcu_HealthCheckResult idcu_HealthCheckResult;
```

### 结果函数

```c
int idcu_healthcheck_result_init(idcu_HealthCheckResult* result);
void idcu_healthcheck_result_destroy(idcu_HealthCheckResult* result);

int idcu_healthcheck_result_set_status(idcu_HealthCheckResult* result, idcu_HealthStatus status);
int idcu_healthcheck_result_set_message(idcu_HealthCheckResult* result, const char* message);
```

## 健康检查类型

```c
typedef enum {
    IDCU_HEALTH_CHECK_TYPE_CUSTOM,
    IDCU_HEALTH_CHECK_TYPE_HTTP,
    IDCU_HEALTH_CHECK_TYPE_TCP,
    IDCU_HEALTH_CHECK_TYPE_DATABASE,
    IDCU_HEALTH_CHECK_TYPE_DISK,
    IDCU_HEALTH_CHECK_TYPE_MEMORY,
    IDCU_HEALTH_CHECK_TYPE_CPU
} idcu_HealthCheckType;
```

## 健康状态

```c
typedef enum {
    IDCU_HEALTH_STATUS_PASS,
    IDCU_HEALTH_STATUS_WARN,
    IDCU_HEALTH_STATUS_FAIL,
    IDCU_HEALTH_STATUS_UNKNOWN
} idcu_HealthStatus;
```

## 内置检查

```c
int idcu_disk_check(const char* path, idcu_HealthCheckResult* result);
int idcu_memory_check(idcu_HealthCheckResult* result);
int idcu_cpu_check(idcu_HealthCheckResult* result);
```

## 自定义检查函数

```c
typedef void (*idcu_HealthCheckFunc)(void* user_data, idcu_HealthCheckResult* result);
```

## 状态字符串

```c
const char* idcu_health_status_to_string(idcu_HealthStatus status);
```

## 示例

```c
#include <idcu/healthcheck/healthcheck.h>
#include <stdio.h>

int main(void) {
    idcu_HealthChecker checker;
    idcu_healthchecker_init(&checker);
    
    // 添加磁盘检查
    idcu_HealthCheck disk_check;
    idcu_healthcheck_init(&disk_check, "disk", IDCU_HEALTH_CHECK_TYPE_DISK);
    idcu_healthcheck_set_disk(&disk_check, "/", 1024 * 1024 * 1024, 10.0);
    idcu_healthchecker_add_check(&checker, &disk_check);
    
    // 添加内存检查
    idcu_HealthCheck memory_check;
    idcu_healthcheck_init(&memory_check, "memory", IDCU_HEALTH_CHECK_TYPE_MEMORY);
    idcu_healthcheck_set_memory(&memory_check, 512 * 1024 * 1024, 5.0);
    idcu_healthchecker_add_check(&checker, &memory_check);
    
    // 运行所有检查
    idcu_healthchecker_check_all(&checker);
    
    // 获取整体状态
    idcu_HealthStatus status = idcu_healthchecker_get_overall_status(&checker);
    printf("整体健康状态: %s\n", idcu_health_status_to_string(status));
    
    // 输出为 JSON
    char json_buffer[4096];
    idcu_healthchecker_to_json(&checker, json_buffer, sizeof(json_buffer));
    printf("JSON: %s\n", json_buffer);
    
    idcu_healthchecker_destroy(&checker);
    return 0;
}
```
