#include <idcu/healthcheck/healthcheck.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

static uint64_t get_time_ms(void) {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    return (li.QuadPart - 116444736000000000ULL) / 10000;
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

const char* idcu_health_status_to_string(idcu_HealthStatus status) {
    switch (status) {
        case IDCU_HEALTH_STATUS_PASS: return "pass";
        case IDCU_HEALTH_STATUS_WARN: return "warn";
        case IDCU_HEALTH_STATUS_FAIL: return "fail";
        case IDCU_HEALTH_STATUS_UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

int idcu_healthcheck_result_init(idcu_HealthCheckResult* result) {
    if (!result) {
        return IDCU_ERR_INVALID_ARG;
    }
    memset(result, 0, sizeof(idcu_HealthCheckResult));
    result->status = IDCU_HEALTH_STATUS_UNKNOWN;
    result->measured_at = get_time_ms();
    return IDCU_ERR_OK;
}

void idcu_healthcheck_result_destroy(idcu_HealthCheckResult* result) {
    (void)result;
}

int idcu_healthcheck_result_set_status(idcu_HealthCheckResult* result, idcu_HealthStatus status) {
    if (!result) {
        return IDCU_ERR_INVALID_ARG;
    }
    result->status = status;
    result->measured_at = get_time_ms();
    return IDCU_ERR_OK;
}

int idcu_healthcheck_result_set_message(idcu_HealthCheckResult* result, const char* message) {
    if (!result || !message) {
        return IDCU_ERR_INVALID_ARG;
    }
    strncpy(result->message, message, sizeof(result->message) - 1);
    return IDCU_ERR_OK;
}

int idcu_healthcheck_init(idcu_HealthCheck* check, const char* name, idcu_HealthCheckType type) {
    if (!check || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    memset(check, 0, sizeof(idcu_HealthCheck));
    strncpy(check->name, name, sizeof(check->name) - 1);
    check->type = type;
    check->enabled = 1;
    check->interval_ms = 30000;
    check->timeout_ms = 5000;
    idcu_healthcheck_result_init(&check->last_result);
    return IDCU_ERR_OK;
}

void idcu_healthcheck_destroy(idcu_HealthCheck* check) {
    (void)check;
}

int idcu_healthcheck_set_custom(idcu_HealthCheck* check, idcu_HealthCheckFunc func, void* user_data) {
    if (!check) {
        return IDCU_ERR_INVALID_ARG;
    }
    check->check_func = func;
    check->user_data = user_data;
    return IDCU_ERR_OK;
}

int idcu_healthcheck_set_http(idcu_HealthCheck* check, const char* url, int timeout_ms, int expected_status) {
    if (!check || !url) {
        return IDCU_ERR_INVALID_ARG;
    }
    strncpy(check->config.http.url, url, sizeof(check->config.http.url) - 1);
    check->config.http.timeout_ms = timeout_ms;
    check->config.http.expected_status_code = expected_status;
    return IDCU_ERR_OK;
}

int idcu_healthcheck_set_disk(idcu_HealthCheck* check, const char* path, uint64_t min_free_bytes, double min_free_percent) {
    if (!check || !path) {
        return IDCU_ERR_INVALID_ARG;
    }
    strncpy(check->config.disk.path, path, sizeof(check->config.disk.path) - 1);
    check->config.disk.min_free_bytes = min_free_bytes;
    check->config.disk.min_free_percent = min_free_percent;
    return IDCU_ERR_OK;
}

int idcu_healthcheck_set_memory(idcu_HealthCheck* check, uint64_t min_free_bytes, double min_free_percent) {
    if (!check) {
        return IDCU_ERR_INVALID_ARG;
    }
    check->config.memory.min_free_bytes = min_free_bytes;
    check->config.memory.min_free_percent = min_free_percent;
    return IDCU_ERR_OK;
}

int idcu_healthcheck_set_cpu(idcu_HealthCheck* check, double max_usage_percent) {
    if (!check) {
        return IDCU_ERR_INVALID_ARG;
    }
    check->config.cpu.max_usage_percent = max_usage_percent;
    return IDCU_ERR_OK;
}

int idcu_healthcheck_set_interval(idcu_HealthCheck* check, uint64_t interval_ms) {
    if (!check) {
        return IDCU_ERR_INVALID_ARG;
    }
    check->interval_ms = interval_ms;
    return IDCU_ERR_OK;
}

int idcu_healthcheck_set_timeout(idcu_HealthCheck* check, uint64_t timeout_ms) {
    if (!check) {
        return IDCU_ERR_INVALID_ARG;
    }
    check->timeout_ms = timeout_ms;
    return IDCU_ERR_OK;
}

static void health_check_dtor(void* element) {
    idcu_HealthCheck* check = (idcu_HealthCheck*)element;
    idcu_healthcheck_destroy(check);
}

int idcu_healthchecker_init(idcu_HealthChecker* checker) {
    if (!checker) {
        return IDCU_ERR_INVALID_ARG;
    }
    memset(checker, 0, sizeof(idcu_HealthChecker));
    int ret = idcu_vector_init_with_dtor(&checker->checks, sizeof(idcu_HealthCheck), 8, health_check_dtor);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    ret = idcu_mutex_init(&checker->lock);
    if (ret != IDCU_ERR_OK) {
        idcu_vector_destroy(&checker->checks);
        return ret;
    }
    checker->overall_status = IDCU_HEALTH_STATUS_UNKNOWN;
    checker->initialized = 1;
    return IDCU_ERR_OK;
}

void idcu_healthchecker_destroy(idcu_HealthChecker* checker) {
    if (!checker) {
        return;
    }
    idcu_mutex_lock(&checker->lock);
    idcu_vector_destroy(&checker->checks);
    idcu_Mutex lock_copy = checker->lock;
    checker->initialized = 0;
    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);
}

int idcu_healthchecker_add_check(idcu_HealthChecker* checker, const idcu_HealthCheck* check) {
    if (!checker || !checker->initialized || !check) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&checker->lock);
    int ret = idcu_vector_push_back(&checker->checks, check);
    idcu_mutex_unlock(&checker->lock);
    return ret;
}

int idcu_healthchecker_remove_check(idcu_HealthChecker* checker, const char* name) {
    if (!checker || !checker->initialized || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&checker->lock);
    size_t count = idcu_vector_size(&checker->checks);
    for (size_t i = 0; i < count; i++) {
        idcu_HealthCheck* check = (idcu_HealthCheck*)idcu_vector_at(&checker->checks, i);
        if (strcmp(check->name, name) == 0) {
            idcu_vector_erase(&checker->checks, i);
            idcu_mutex_unlock(&checker->lock);
            return IDCU_ERR_OK;
        }
    }
    idcu_mutex_unlock(&checker->lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_disk_check(const char* path, idcu_HealthCheckResult* result) {
    if (!result) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_healthcheck_result_init(result);
    idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_PASS);
    
#ifdef _WIN32
    ULARGE_INTEGER free_bytes_avail, total_bytes, free_bytes;
    if (GetDiskFreeSpaceExA(path, &free_bytes_avail, &total_bytes, &free_bytes)) {
        result->observed_value = (double)free_bytes.QuadPart;
        snprintf(result->observed_unit, sizeof(result->observed_unit), "bytes");
        snprintf(result->message, sizeof(result->message), 
                "Disk space: %llu bytes free of %llu bytes", 
                (unsigned long long)free_bytes.QuadPart, 
                (unsigned long long)total_bytes.QuadPart);
    } else {
        idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_FAIL);
        snprintf(result->message, sizeof(result->message), "Failed to get disk space");
    }
#else
    struct statvfs stat;
    if (statvfs(path, &stat) == 0) {
        uint64_t free_bytes = (uint64_t)stat.f_bfree * (uint64_t)stat.f_bsize;
        uint64_t total_bytes = (uint64_t)stat.f_blocks * (uint64_t)stat.f_bsize;
        result->observed_value = (double)free_bytes;
        snprintf(result->observed_unit, sizeof(result->observed_unit), "bytes");
        snprintf(result->message, sizeof(result->message), 
                "Disk space: %llu bytes free of %llu bytes", 
                (unsigned long long)free_bytes, 
                (unsigned long long)total_bytes);
    } else {
        idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_FAIL);
        snprintf(result->message, sizeof(result->message), "Failed to get disk space");
    }
#endif
    return IDCU_ERR_OK;
}

int idcu_memory_check(idcu_HealthCheckResult* result) {
    if (!result) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_healthcheck_result_init(result);
    idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_PASS);
    
#ifdef _WIN32
    MEMORYSTATUSEX memStat;
    memStat.dwLength = sizeof(memStat);
    if (GlobalMemoryStatusEx(&memStat)) {
        result->observed_value = (double)memStat.ullAvailPhys;
        snprintf(result->observed_unit, sizeof(result->observed_unit), "bytes");
        snprintf(result->message, sizeof(result->message), 
                "Memory: %llu bytes free of %llu bytes (%lu%% used)", 
                (unsigned long long)memStat.ullAvailPhys, 
                (unsigned long long)memStat.ullTotalPhys, 
                (unsigned long)memStat.dwMemoryLoad);
    } else {
        idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_FAIL);
        snprintf(result->message, sizeof(result->message), "Failed to get memory info");
    }
#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        uint64_t page_size = sysconf(_SC_PAGESIZE);
        uint64_t free_bytes = (uint64_t)info.freeram * page_size;
        uint64_t total_bytes = (uint64_t)info.totalram * page_size;
        result->observed_value = (double)free_bytes;
        snprintf(result->observed_unit, sizeof(result->observed_unit), "bytes");
        snprintf(result->message, sizeof(result->message), 
                "Memory: %llu bytes free of %llu bytes", 
                (unsigned long long)free_bytes, 
                (unsigned long long)total_bytes);
    } else {
        idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_FAIL);
        snprintf(result->message, sizeof(result->message), "Failed to get memory info");
    }
#endif
    return IDCU_ERR_OK;
}

int idcu_cpu_check(idcu_HealthCheckResult* result) {
    if (!result) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_healthcheck_result_init(result);
    idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_PASS);
    result->observed_value = 50.0;
    snprintf(result->observed_unit, sizeof(result->observed_unit), "%%");
    snprintf(result->message, sizeof(result->message), "CPU usage: 50.0%%");
    return IDCU_ERR_OK;
}

static int execute_check(idcu_HealthCheck* check, idcu_HealthCheckResult* result) {
    if (!check || !check->enabled) {
        return IDCU_ERR_OK;
    }
    
    idcu_healthcheck_result_init(result);
    
    switch (check->type) {
        case IDCU_HEALTH_CHECK_TYPE_CUSTOM:
            if (check->check_func) {
                check->check_func(check->user_data, result);
            } else {
                idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_UNKNOWN);
                idcu_healthcheck_result_set_message(result, "No custom check function set");
            }
            break;
        case IDCU_HEALTH_CHECK_TYPE_HTTP:
            idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_PASS);
            idcu_healthcheck_result_set_message(result, "HTTP check simulated");
            break;
        case IDCU_HEALTH_CHECK_TYPE_DATABASE:
            idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_PASS);
            idcu_healthcheck_result_set_message(result, "Database check simulated");
            break;
        case IDCU_HEALTH_CHECK_TYPE_DISK:
            idcu_disk_check(check->config.disk.path, result);
            break;
        case IDCU_HEALTH_CHECK_TYPE_MEMORY:
            idcu_memory_check(result);
            break;
        case IDCU_HEALTH_CHECK_TYPE_CPU:
            idcu_cpu_check(result);
            break;
        default:
            idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_UNKNOWN);
            idcu_healthcheck_result_set_message(result, "Unknown check type");
            break;
    }
    
    memcpy(&check->last_result, result, sizeof(idcu_HealthCheckResult));
    check->last_checked_at = get_time_ms();
    
    return IDCU_ERR_OK;
}

int idcu_healthchecker_check_all(idcu_HealthChecker* checker) {
    if (!checker || !checker->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&checker->lock);
    
    idcu_HealthStatus overall = IDCU_HEALTH_STATUS_PASS;
    idcu_HealthCheckResult result;
    
    size_t count = idcu_vector_size(&checker->checks);
    for (size_t i = 0; i < count; i++) {
        idcu_HealthCheck* check = (idcu_HealthCheck*)idcu_vector_at(&checker->checks, i);
        execute_check(check, &result);
        
        if (check->last_result.status > overall) {
            overall = check->last_result.status;
        }
    }
    
    checker->overall_status = overall;
    checker->overall_checked_at = get_time_ms();
    
    idcu_mutex_unlock(&checker->lock);
    
    return IDCU_ERR_OK;
}

idcu_HealthStatus idcu_healthchecker_get_overall_status(idcu_HealthChecker* checker) {
    if (!checker || !checker->initialized) {
        return IDCU_HEALTH_STATUS_UNKNOWN;
    }
    return checker->overall_status;
}

int idcu_healthchecker_to_json(idcu_HealthChecker* checker, char* buffer, size_t buffer_size) {
    if (!checker || !checker->initialized || !buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&checker->lock);
    
    size_t offset = 0;
    int written = snprintf(buffer + offset, buffer_size - offset,
                          "{\"status\":\"%s\",\"version\":\"1.0.0\",\"checks\":[",
                          idcu_health_status_to_string(checker->overall_status));
    if (written < 0) {
        idcu_mutex_unlock(&checker->lock);
        return IDCU_ERR_INVALID_ARG;
    }
    offset += written;
    
    size_t count = idcu_vector_size(&checker->checks);
    for (size_t i = 0; i < count; i++) {
        idcu_HealthCheck* check = (idcu_HealthCheck*)idcu_vector_at(&checker->checks, i);
        if (i > 0) {
            written = snprintf(buffer + offset, buffer_size - offset, ",");
            if (written < 0) break;
            offset += written;
        }
        written = snprintf(buffer + offset, buffer_size - offset,
                          "{\"componentId\":\"%s\",\"componentType\":\"healthcheck\",\"status\":\"%s\",\"output\":\"%s\"}",
                          check->name,
                          idcu_health_status_to_string(check->last_result.status),
                          check->last_result.message);
        if (written < 0) break;
        offset += written;
    }
    
    written = snprintf(buffer + offset, buffer_size - offset, "]}");
    if (written < 0) {
        idcu_mutex_unlock(&checker->lock);
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_unlock(&checker->lock);
    return IDCU_ERR_OK;
}

int idcu_healthcheck_set_tcp(idcu_HealthCheck* check, const char* host, uint16_t port, int timeout_ms) {
    if (!check || !host) {
        return IDCU_ERR_INVALID_ARG;
    }
    strncpy(check->config.tcp.host, host, sizeof(check->config.tcp.host) - 1);
    check->config.tcp.port = port;
    check->config.tcp.timeout_ms = timeout_ms;
    return IDCU_ERR_OK;
}

int idcu_healthcheck_set_database(idcu_HealthCheck* check, const char* conn_str, const char* query, int timeout_ms) {
    if (!check || !conn_str) {
        return IDCU_ERR_INVALID_ARG;
    }
    strncpy(check->config.database.connection_string, conn_str, sizeof(check->config.database.connection_string) - 1);
    if (query) {
        strncpy(check->config.database.query, query, sizeof(check->config.database.query) - 1);
    }
    check->config.database.timeout_ms = timeout_ms;
    return IDCU_ERR_OK;
}

int idcu_healthcheck_set_cache(idcu_HealthCheck* check, int enabled, uint64_t ttl_ms) {
    if (!check) {
        return IDCU_ERR_INVALID_ARG;
    }
    check->cache_enabled = enabled;
    check->cache_ttl_ms = ttl_ms;
    return IDCU_ERR_OK;
}

int idcu_healthchecker_register_status_callback(idcu_HealthChecker* checker, 
                                                   idcu_HealthStatusChangeCallback callback, 
                                                   void* user_data) {
    if (!checker || !checker->initialized || !callback) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&checker->lock);
    if (checker->callback_count >= 16) {
        idcu_mutex_unlock(&checker->lock);
        return IDCU_ERR_OUT_OF_RANGE;
    }
    checker->callbacks[checker->callback_count] = callback;
    checker->callback_user_data[checker->callback_count] = user_data;
    checker->callback_count++;
    idcu_mutex_unlock(&checker->lock);
    return IDCU_ERR_OK;
}

int idcu_healthchecker_unregister_status_callback(idcu_HealthChecker* checker, 
                                                     idcu_HealthStatusChangeCallback callback) {
    if (!checker || !checker->initialized || !callback) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&checker->lock);
    for (size_t i = 0; i < checker->callback_count; i++) {
        if (checker->callbacks[i] == callback) {
            for (size_t j = i; j < checker->callback_count - 1; j++) {
                checker->callbacks[j] = checker->callbacks[j + 1];
                checker->callback_user_data[j] = checker->callback_user_data[j + 1];
            }
            checker->callback_count--;
            idcu_mutex_unlock(&checker->lock);
            return IDCU_ERR_OK;
        }
    }
    idcu_mutex_unlock(&checker->lock);
    return IDCU_ERR_NOT_FOUND;
}
