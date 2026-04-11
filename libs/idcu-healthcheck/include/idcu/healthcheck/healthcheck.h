#ifndef IDCU_HEALTHCHECK_HEALTHCHECK_H
#define IDCU_HEALTHCHECK_HEALTHCHECK_H

#include <idcu/common/error_code.h>
#include <idcu/common/vector.h>
#include <idcu/common/lock.h>
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
    union {
        struct {
            char url[1024];
            int timeout_ms;
            int expected_status_code;
        } http;
        struct {
            char path[1024];
            uint64_t min_free_bytes;
            double min_free_percent;
        } disk;
        struct {
            uint64_t min_free_bytes;
            double min_free_percent;
        } memory;
        struct {
            double max_usage_percent;
        } cpu;
    } config;
} idcu_HealthCheck;

typedef struct
{
    idcu_Vector checks;
    idcu_Mutex lock;
    uint64_t overall_checked_at;
    idcu_HealthStatus overall_status;
    int initialized;
} idcu_HealthChecker;

int  idcu_healthchecker_init(idcu_HealthChecker* checker);
void idcu_healthchecker_destroy(idcu_HealthChecker* checker);

int  idcu_healthchecker_add_check(idcu_HealthChecker* checker, const idcu_HealthCheck* check);
int  idcu_healthchecker_remove_check(idcu_HealthChecker* checker, const char* name);

int  idcu_healthchecker_check_all(idcu_HealthChecker* checker);
idcu_HealthStatus idcu_healthchecker_get_overall_status(idcu_HealthChecker* checker);

int  idcu_healthchecker_to_json(idcu_HealthChecker* checker, char* buffer, size_t buffer_size);

int  idcu_healthcheck_init(idcu_HealthCheck* check, const char* name, idcu_HealthCheckType type);
void idcu_healthcheck_destroy(idcu_HealthCheck* check);
int  idcu_healthcheck_set_custom(idcu_HealthCheck* check, idcu_HealthCheckFunc func, void* user_data);
int  idcu_healthcheck_set_http(idcu_HealthCheck* check, const char* url, int timeout_ms, int expected_status);
int  idcu_healthcheck_set_disk(idcu_HealthCheck* check, const char* path, uint64_t min_free_bytes, double min_free_percent);
int  idcu_healthcheck_set_memory(idcu_HealthCheck* check, uint64_t min_free_bytes, double min_free_percent);
int  idcu_healthcheck_set_cpu(idcu_HealthCheck* check, double max_usage_percent);
int  idcu_healthcheck_set_interval(idcu_HealthCheck* check, uint64_t interval_ms);
int  idcu_healthcheck_set_timeout(idcu_HealthCheck* check, uint64_t timeout_ms);

int  idcu_healthcheck_result_init(idcu_HealthCheckResult* result);
void idcu_healthcheck_result_destroy(idcu_HealthCheckResult* result);
int  idcu_healthcheck_result_set_status(idcu_HealthCheckResult* result, idcu_HealthStatus status);
int  idcu_healthcheck_result_set_message(idcu_HealthCheckResult* result, const char* message);
const char* idcu_health_status_to_string(idcu_HealthStatus status);

int idcu_disk_check(const char* path, idcu_HealthCheckResult* result);
int idcu_memory_check(idcu_HealthCheckResult* result);
int idcu_cpu_check(idcu_HealthCheckResult* result);

#ifdef __cplusplus
}
#endif

#endif
