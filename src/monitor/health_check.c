#include "monitor/health_check.h"
#include "utils/log.h"
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#include <sys/time.h>
#endif

uint64_t health_get_uptime_ms(void)
{
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

int health_monitor_init(HealthMonitor* monitor)
{
    if (!monitor) {
        return ERR_INVALID_PARAM;
    }

    memset(monitor, 0, sizeof(HealthMonitor));
    int ret = mutex_init(&monitor->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    monitor->timeout_ms = 10000;
    monitor->warning_threshold = 5;
    monitor->critical_threshold = 10;

    return ERR_SUCCESS;
}

void health_monitor_destroy(HealthMonitor* monitor)
{
    if (!monitor) {
        return;
    }
    mutex_destroy(&monitor->lock);
}

int health_register_module(HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&monitor->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            mutex_unlock(&monitor->lock);
            return ERR_ALREADY_EXISTS;
        }
    }

    if (monitor->count >= MAX_MODULES_HEALTH) {
        mutex_unlock(&monitor->lock);
        return ERR_QUEUE_FULL;
    }

    ModuleHealth* m = &monitor->modules[monitor->count];
    m->module_id = module_id;
    m->status = HEALTH_HEALTHY;
    m->last_heartbeat_ms = health_get_uptime_ms();
    m->error_count = 0;
    m->restart_count = 0;
    m->created_ms = health_get_uptime_ms();
    monitor->count++;

    mutex_unlock(&monitor->lock);
    return ERR_SUCCESS;
}

int health_unregister_module(HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&monitor->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            if (i < monitor->count - 1) {
                memmove(&monitor->modules[i], &monitor->modules[i + 1],
                        (monitor->count - i - 1) * sizeof(ModuleHealth));
            }
            monitor->count--;
            mutex_unlock(&monitor->lock);
            return ERR_SUCCESS;
        }
    }

    mutex_unlock(&monitor->lock);
    return ERR_NOT_FOUND;
}

int health_update_heartbeat(HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&monitor->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            monitor->modules[i].last_heartbeat_ms = health_get_uptime_ms();
            HealthStatus old_status = monitor->modules[i].status;
            monitor->modules[i].status = HEALTH_HEALTHY;
            
            if (monitor->callback && old_status != HEALTH_HEALTHY) {
                monitor->callback(module_id, old_status, HEALTH_HEALTHY, monitor->callback_user_data);
            }
            
            mutex_unlock(&monitor->lock);
            return ERR_SUCCESS;
        }
    }

    mutex_unlock(&monitor->lock);
    return ERR_NOT_FOUND;
}

static void update_status(HealthMonitor* monitor, ModuleHealth* m)
{
    HealthStatus old_status = m->status;

    if (m->error_count >= monitor->critical_threshold) {
        m->status = HEALTH_CRITICAL;
    } else if (m->error_count >= monitor->warning_threshold) {
        m->status = HEALTH_WARNING;
    } else {
        m->status = HEALTH_HEALTHY;
    }

    if (monitor->callback && old_status != m->status) {
        monitor->callback(m->module_id, old_status, m->status, monitor->callback_user_data);
    }
}

int health_report_error(HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&monitor->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            monitor->modules[i].error_count++;
            update_status(monitor, &monitor->modules[i]);
            mutex_unlock(&monitor->lock);
            return ERR_SUCCESS;
        }
    }

    mutex_unlock(&monitor->lock);
    return ERR_NOT_FOUND;
}

int health_report_restart(HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&monitor->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            monitor->modules[i].restart_count++;
            monitor->modules[i].error_count = 0;
            monitor->modules[i].last_heartbeat_ms = health_get_uptime_ms();
            update_status(monitor, &monitor->modules[i]);
            mutex_unlock(&monitor->lock);
            return ERR_SUCCESS;
        }
    }

    mutex_unlock(&monitor->lock);
    return ERR_NOT_FOUND;
}

HealthStatus health_get_status(HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return HEALTH_UNKNOWN;
    }

    int ret = mutex_lock(&monitor->lock);
    if (ret != ERR_SUCCESS) {
        return HEALTH_UNKNOWN;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            HealthStatus status = monitor->modules[i].status;
            mutex_unlock(&monitor->lock);
            return status;
        }
    }

    mutex_unlock(&monitor->lock);
    return HEALTH_UNKNOWN;
}

const ModuleHealth* health_get_info(HealthMonitor* monitor, uint32_t module_id)
{
    if (!monitor) {
        return NULL;
    }

    int ret = mutex_lock(&monitor->lock);
    if (ret != ERR_SUCCESS) {
        return NULL;
    }

    for (uint32_t i = 0; i < monitor->count; i++) {
        if (monitor->modules[i].module_id == module_id) {
            const ModuleHealth* info = &monitor->modules[i];
            mutex_unlock(&monitor->lock);
            return info;
        }
    }

    mutex_unlock(&monitor->lock);
    return NULL;
}

int health_check_all(HealthMonitor* monitor)
{
    if (!monitor) {
        return ERR_INVALID_PARAM;
    }

    int ret = mutex_lock(&monitor->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }

    uint64_t now = health_get_uptime_ms();

    for (uint32_t i = 0; i < monitor->count; i++) {
        ModuleHealth* m = &monitor->modules[i];
        if (now - m->last_heartbeat_ms > monitor->timeout_ms) {
            HealthStatus old_status = m->status;
            m->status = HEALTH_DEAD;
            
            if (monitor->callback && old_status != HEALTH_DEAD) {
                monitor->callback(m->module_id, old_status, HEALTH_DEAD, monitor->callback_user_data);
            }
        }
    }

    mutex_unlock(&monitor->lock);
    return ERR_SUCCESS;
}

void health_set_callback(HealthMonitor* monitor, HealthCallback cb, void* user_data)
{
    if (!monitor) {
        return;
    }

    int ret = mutex_lock(&monitor->lock);
    if (ret != ERR_SUCCESS) {
        return;
    }

    monitor->callback = cb;
    monitor->callback_user_data = user_data;

    mutex_unlock(&monitor->lock);
}
