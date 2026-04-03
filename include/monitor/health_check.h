#ifndef HEALTH_CHECK_H
#define HEALTH_CHECK_H

#include "common/error_code.h"
#include "common/lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HEALTH_UNKNOWN = 0,
    HEALTH_HEALTHY,
    HEALTH_WARNING,
    HEALTH_CRITICAL,
    HEALTH_DEAD
} HealthStatus;

typedef struct {
    uint32_t module_id;
    HealthStatus status;
    uint64_t last_heartbeat_ms;
    uint64_t error_count;
    uint64_t restart_count;
    uint64_t created_ms;
} ModuleHealth;

typedef void (*HealthCallback)(uint32_t module_id, HealthStatus old_status, HealthStatus new_status, void* user_data);

#define MAX_MODULES_HEALTH 64

typedef struct {
    ModuleHealth modules[MAX_MODULES_HEALTH];
    uint32_t count;
    Mutex lock;
    HealthCallback callback;
    void* callback_user_data;
    uint64_t timeout_ms;
    uint64_t warning_threshold;
    uint64_t critical_threshold;
} HealthMonitor;

int health_monitor_init(HealthMonitor* monitor);
void health_monitor_destroy(HealthMonitor* monitor);
int health_register_module(HealthMonitor* monitor, uint32_t module_id);
int health_unregister_module(HealthMonitor* monitor, uint32_t module_id);
int health_update_heartbeat(HealthMonitor* monitor, uint32_t module_id);
int health_report_error(HealthMonitor* monitor, uint32_t module_id);
int health_report_restart(HealthMonitor* monitor, uint32_t module_id);
HealthStatus health_get_status(HealthMonitor* monitor, uint32_t module_id);
const ModuleHealth* health_get_info(HealthMonitor* monitor, uint32_t module_id);
int health_check_all(HealthMonitor* monitor);
void health_set_callback(HealthMonitor* monitor, HealthCallback cb, void* user_data);
uint64_t health_get_uptime_ms(void);

#ifdef __cplusplus
}
#endif

#endif
