# idcu-healthcheck API Documentation

Health checking library for monitoring system health.

## Health Checker

```c
typedef struct idcu_HealthChecker idcu_HealthChecker;
```

### Health Checker Functions

```c
int idcu_healthchecker_init(idcu_HealthChecker* checker);
void idcu_healthchecker_destroy(idcu_HealthChecker* checker);

int idcu_healthchecker_add_check(idcu_HealthChecker* checker, const idcu_HealthCheck* check);
int idcu_healthchecker_remove_check(idcu_HealthChecker* checker, const char* name);

int idcu_healthchecker_check_all(idcu_HealthChecker* checker);
idcu_HealthStatus idcu_healthchecker_get_overall_status(idcu_HealthChecker* checker);

int idcu_healthchecker_to_json(idcu_HealthChecker* checker, char* buffer, size_t buffer_size);
```

## Health Check

```c
typedef struct idcu_HealthCheck idcu_HealthCheck;
```

### Health Check Functions

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

## Health Check Result

```c
typedef struct idcu_HealthCheckResult idcu_HealthCheckResult;
```

### Result Functions

```c
int idcu_healthcheck_result_init(idcu_HealthCheckResult* result);
void idcu_healthcheck_result_destroy(idcu_HealthCheckResult* result);

int idcu_healthcheck_result_set_status(idcu_HealthCheckResult* result, idcu_HealthStatus status);
int idcu_healthcheck_result_set_message(idcu_HealthCheckResult* result, const char* message);
```

## Health Check Types

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

## Health Status

```c
typedef enum {
    IDCU_HEALTH_STATUS_PASS,
    IDCU_HEALTH_STATUS_WARN,
    IDCU_HEALTH_STATUS_FAIL,
    IDCU_HEALTH_STATUS_UNKNOWN
} idcu_HealthStatus;
```

## Built-in Checks

```c
int idcu_disk_check(const char* path, idcu_HealthCheckResult* result);
int idcu_memory_check(idcu_HealthCheckResult* result);
int idcu_cpu_check(idcu_HealthCheckResult* result);
```

## Custom Check Function

```c
typedef void (*idcu_HealthCheckFunc)(void* user_data, idcu_HealthCheckResult* result);
```

## Status String

```c
const char* idcu_health_status_to_string(idcu_HealthStatus status);
```

## Example

```c
#include <idcu/healthcheck/healthcheck.h>
#include <stdio.h>

int main(void) {
    idcu_HealthChecker checker;
    idcu_healthchecker_init(&checker);
    
    // Add disk check
    idcu_HealthCheck disk_check;
    idcu_healthcheck_init(&disk_check, "disk", IDCU_HEALTH_CHECK_TYPE_DISK);
    idcu_healthcheck_set_disk(&disk_check, "/", 1024 * 1024 * 1024, 10.0);
    idcu_healthchecker_add_check(&checker, &disk_check);
    
    // Add memory check
    idcu_HealthCheck memory_check;
    idcu_healthcheck_init(&memory_check, "memory", IDCU_HEALTH_CHECK_TYPE_MEMORY);
    idcu_healthcheck_set_memory(&memory_check, 512 * 1024 * 1024, 5.0);
    idcu_healthchecker_add_check(&checker, &memory_check);
    
    // Run all checks
    idcu_healthchecker_check_all(&checker);
    
    // Get overall status
    idcu_HealthStatus status = idcu_healthchecker_get_overall_status(&checker);
    printf("Overall health: %s\n", idcu_health_status_to_string(status));
    
    // Output as JSON
    char json_buffer[4096];
    idcu_healthchecker_to_json(&checker, json_buffer, sizeof(json_buffer));
    printf("JSON: %s\n", json_buffer);
    
    idcu_healthchecker_destroy(&checker);
    return 0;
}
```
