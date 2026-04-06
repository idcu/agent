#include "idcu/healthcheck/healthcheck.h"
#include "idcu/log/log.h"
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#include <sys/time.h>
#endif

uint64_t idcu_health_get_uptime_ms(void)
{
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

int idcu_health_monitor_init(idcu_HealthMonitor* monitor)
{
    if (!monitor) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(monitor, 0, sizeof(idcu_HealthMonitor));
    int ret = idcu_mutex_init(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    monitor->timeout_ms = 10000;
    monitor->warning_threshold = 5;
    monitor->critical_threshold = 10;
    monitor->last_check_ms = idcu_health_get_uptime_ms();

    return IDCU_ERR_SUCCESS;
}

void idcu_health_monitor_destroy(idcu_HealthMonitor* monitor)
{
    if (!monitor) {
        return;
    }
    idcu_mutex_destroy(&monitor->lock);
}

int idcu_health_register_module(idcu_HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            idcu_mutex_unlock(&monitor->lock);
            return IDCU_ERR_ALREADY_EXISTS;
        }
    }

    if (monitor->count >= IDCU_MAX_MODULES_HEALTH) {
        idcu_mutex_unlock(&monitor->lock);
        return IDCU_ERR_QUEUE_FULL;
    }

    idcu_ModuleHealth* m = &monitor->modules[monitor->count];
    m->module_id = module_id;
    m->status = IDCU_HEALTH_HEALTHY;
    m->last_heartbeat_ms = idcu_health_get_uptime_ms();
    m->error_count = 0;
    m->restart_count = 0;
    m->created_ms = idcu_health_get_uptime_ms();
    monitor->count++;

    idcu_mutex_unlock(&monitor->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_health_unregister_module(idcu_HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            if (i < monitor->count - 1) {
                memmove(&monitor->modules[i], &monitor->modules[i + 1],
                        (monitor->count - i - 1) * sizeof(idcu_ModuleHealth));
            }
            monitor->count--;
            idcu_mutex_unlock(&monitor->lock);
            return IDCU_ERR_SUCCESS;
        }
    }

    idcu_mutex_unlock(&monitor->lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_health_update_heartbeat(idcu_HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            monitor->modules[i].last_heartbeat_ms = idcu_health_get_uptime_ms();
            idcu_HealthStatus old_status = monitor->modules[i].status;
            monitor->modules[i].status = IDCU_HEALTH_HEALTHY;
            
            if (monitor->callback && old_status != IDCU_HEALTH_HEALTHY) {
                monitor->callback(module_id, old_status, IDCU_HEALTH_HEALTHY, monitor->callback_user_data);
            }
            
            idcu_mutex_unlock(&monitor->lock);
            return IDCU_ERR_SUCCESS;
        }
    }

    idcu_mutex_unlock(&monitor->lock);
    return IDCU_ERR_NOT_FOUND;
}

static void update_status(idcu_HealthMonitor* monitor, idcu_ModuleHealth* m)
{
    idcu_HealthStatus old_status = m->status;

    if (m->error_count >= monitor->critical_threshold) {
        m->status = IDCU_HEALTH_CRITICAL;
    } else if (m->error_count >= monitor->warning_threshold) {
        m->status = IDCU_HEALTH_WARNING;
    } else {
        m->status = IDCU_HEALTH_HEALTHY;
    }

    if (monitor->callback && old_status != m->status) {
        monitor->callback(m->module_id, old_status, m->status, monitor->callback_user_data);
    }
}

int idcu_health_report_error(idcu_HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            monitor->modules[i].error_count++;
            update_status(monitor, &monitor->modules[i]);
            idcu_mutex_unlock(&monitor->lock);
            return IDCU_ERR_SUCCESS;
        }
    }

    idcu_mutex_unlock(&monitor->lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_health_report_restart(idcu_HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            monitor->modules[i].restart_count++;
            monitor->modules[i].error_count = 0;
            monitor->modules[i].last_heartbeat_ms = idcu_health_get_uptime_ms();
            update_status(monitor, &monitor->modules[i]);
            idcu_mutex_unlock(&monitor->lock);
            return IDCU_ERR_SUCCESS;
        }
    }

    idcu_mutex_unlock(&monitor->lock);
    return IDCU_ERR_NOT_FOUND;
}

idcu_HealthStatus idcu_health_get_status(idcu_HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return IDCU_HEALTH_UNKNOWN;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return IDCU_HEALTH_UNKNOWN;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            idcu_HealthStatus status = monitor->modules[i].status;
            idcu_mutex_unlock(&monitor->lock);
            return status;
        }
    }

    idcu_mutex_unlock(&monitor->lock);
    return IDCU_HEALTH_UNKNOWN;
}

const idcu_ModuleHealth* idcu_health_get_info(idcu_HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return NULL;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return NULL;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            const idcu_ModuleHealth* info = &monitor->modules[i];
            idcu_mutex_unlock(&monitor->lock);
            return info;
        }
    }

    idcu_mutex_unlock(&monitor->lock);
    return NULL;
}

int idcu_health_check_all(idcu_HealthMonitor* monitor)
{
    if (!monitor) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    uint64_t now = idcu_health_get_uptime_ms();

    for (uint32_t i = 0; i < monitor->count; i++) {
        idcu_ModuleHealth* m = &monitor->modules[i];
        if (now - m->last_heartbeat_ms > monitor->timeout_ms) {
            idcu_HealthStatus old_status = m->status;
            m->status = IDCU_HEALTH_DEAD;
            
            if (monitor->callback && old_status != IDCU_HEALTH_DEAD) {
                monitor->callback(m->module_id, old_status, IDCU_HEALTH_DEAD, monitor->callback_user_data);
            }
        }
    }

    idcu_mutex_unlock(&monitor->lock);
    return IDCU_ERR_SUCCESS;
}

void idcu_health_set_callback(idcu_HealthMonitor* monitor, idcu_HealthCallback cb, void* user_data)
{
    if (!monitor) {
        return;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return;
    }

    monitor->callback = cb;
    monitor->callback_user_data = user_data;

    idcu_mutex_unlock(&monitor->lock);
}

void idcu_health_set_thresholds(idcu_HealthMonitor* monitor, uint64_t timeout_ms, uint64_t warning, uint64_t critical)
{
    if (!monitor) {
        return;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return;
    }

    monitor->timeout_ms = timeout_ms;
    monitor->warning_threshold = warning;
    monitor->critical_threshold = critical;

    idcu_mutex_unlock(&monitor->lock);
}

idcu_HealthStatus idcu_health_get_overall_status(idcu_HealthMonitor* monitor)
{
    if (!monitor) {
        return IDCU_HEALTH_UNKNOWN;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return IDCU_HEALTH_UNKNOWN;
    }

    idcu_HealthStatus overall = IDCU_HEALTH_HEALTHY;

    for (uint32_t i = 0; i < monitor->count; i++) {
        idcu_HealthStatus status = monitor->modules[i].status;
        if (status == IDCU_HEALTH_DEAD) {
            overall = IDCU_HEALTH_DEAD;
            break;
        } else if (status == IDCU_HEALTH_CRITICAL) {
            if (overall != IDCU_HEALTH_DEAD) {
                overall = IDCU_HEALTH_CRITICAL;
            }
        } else if (status == IDCU_HEALTH_WARNING && overall == IDCU_HEALTH_HEALTHY) {
            overall = IDCU_HEALTH_WARNING;
        }
    }

    idcu_mutex_unlock(&monitor->lock);
    return overall;
}

void idcu_health_get_summary(idcu_HealthMonitor* monitor, idcu_HealthSummary* summary)
{
    if (!monitor || !summary) {
        return;
    }

    memset(summary, 0, sizeof(idcu_HealthSummary));

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return;
    }

    summary->total_count = monitor->count;

    for (uint32_t i = 0; i < monitor->count; i++) {
        idcu_HealthStatus status = monitor->modules[i].status;
        switch (status) {
            case IDCU_HEALTH_HEALTHY:
                summary->healthy_count++;
                break;
            case IDCU_HEALTH_WARNING:
                summary->warning_count++;
                break;
            case IDCU_HEALTH_CRITICAL:
                summary->critical_count++;
                break;
            case IDCU_HEALTH_DEAD:
                summary->dead_count++;
                break;
            default:
                summary->unknown_count++;
                break;
        }
    }

    idcu_mutex_unlock(&monitor->lock);
}

uint32_t idcu_health_get_module_count(idcu_HealthMonitor* monitor)
{
    if (!monitor) {
        return 0;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }

    uint32_t count = monitor->count;

    idcu_mutex_unlock(&monitor->lock);
    return count;
}

const idcu_ModuleHealth* idcu_health_get_all_modules(idcu_HealthMonitor* monitor, uint32_t* count)
{
    if (!monitor || !count) {
        return NULL;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        *count = 0;
        return NULL;
    }

    *count = monitor->count;
    const idcu_ModuleHealth* modules = monitor->modules;

    idcu_mutex_unlock(&monitor->lock);
    return modules;
}

const char* idcu_health_status_to_string(idcu_HealthStatus status)
{
    switch (status) {
        case IDCU_HEALTH_HEALTHY:
            return "HEALTHY";
        case IDCU_HEALTH_WARNING:
            return "WARNING";
        case IDCU_HEALTH_CRITICAL:
            return "CRITICAL";
        case IDCU_HEALTH_DEAD:
            return "DEAD";
        default:
            return "UNKNOWN";
    }
}

int idcu_health_reset_error_count(idcu_HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&monitor->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            monitor->modules[i].error_count = 0;
            update_status(monitor, &monitor->modules[i]);
            idcu_mutex_unlock(&monitor->lock);
            return IDCU_ERR_SUCCESS;
        }
    }

    idcu_mutex_unlock(&monitor->lock);
    return IDCU_ERR_NOT_FOUND;
}
