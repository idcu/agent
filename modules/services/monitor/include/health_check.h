#ifndef IDCU_MONITOR_HEALTH_CHECK_H
#define IDCU_MONITOR_HEALTH_CHECK_H

#include "error_code.h"
#include "lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // IDCU_MONITOR_HEALTH_CHECK_H

typedef enum {
    IDCU_HEALTH_UNKNOWN = 0,
    IDCU_HEALTH_HEALTHY,
    IDCU_HEALTH_WARNING,
    IDCU_HEALTH_CRITICAL,
    IDCU_HEALTH_DEAD
} idcu_HealthStatus;

typedef struct {
    uint32_t module_id;
    idcu_HealthStatus status;
    uint64_t last_heartbeat_ms;
    uint64_t error_count;
    uint64_t restart_count;
    uint64_t created_ms;
} idcu_ModuleHealth;

typedef void (*idcu_HealthCallback)(uint32_t module_id, idcu_HealthStatus old_status, idcu_HealthStatus new_status, void* user_data);

#define IDCU_MAX_MODULES_HEALTH 64

typedef struct {
    idcu_ModuleHealth modules[IDCU_MAX_MODULES_HEALTH];
    uint32_t count;
    idcu_Mutex lock;
    idcu_HealthCallback callback;
    void* callback_user_data;
    uint64_t timeout_ms;
    uint64_t warning_threshold;
    uint64_t critical_threshold;
    uint64_t last_check_ms;
} idcu_HealthMonitor;

typedef struct {
    uint32_t healthy_count;
    uint32_t warning_count;
    uint32_t critical_count;
    uint32_t dead_count;
    uint32_t unknown_count;
    uint32_t total_count;
} idcu_HealthSummary;

int idcu_health_monitor_init(idcu_HealthMonitor* monitor);
void idcu_health_monitor_destroy(idcu_HealthMonitor* monitor);
int idcu_health_register_module(idcu_HealthMonitor* monitor, uint32_t module_id);
int idcu_health_unregister_module(idcu_HealthMonitor* monitor, uint32_t module_id);
int idcu_health_update_heartbeat(idcu_HealthMonitor* monitor, uint32_t module_id);
int idcu_health_report_error(idcu_HealthMonitor* monitor, uint32_t module_id);
int idcu_health_report_restart(idcu_HealthMonitor* monitor, uint32_t module_id);
idcu_HealthStatus idcu_health_get_status(idcu_HealthMonitor* monitor, uint32_t module_id);
const idcu_ModuleHealth* idcu_health_get_info(idcu_HealthMonitor* monitor, uint32_t module_id);
int idcu_health_check_all(idcu_HealthMonitor* monitor);
void idcu_health_set_callback(idcu_HealthMonitor* monitor, idcu_HealthCallback cb, void* user_data);
uint64_t idcu_health_get_uptime_ms(void);
void idcu_health_set_thresholds(idcu_HealthMonitor* monitor, uint64_t timeout_ms, uint64_t warning, uint64_t critical);
idcu_HealthStatus idcu_health_get_overall_status(idcu_HealthMonitor* monitor);
void idcu_health_get_summary(idcu_HealthMonitor* monitor, idcu_HealthSummary* summary);
uint32_t idcu_health_get_module_count(idcu_HealthMonitor* monitor);
const idcu_ModuleHealth* idcu_health_get_all_modules(idcu_HealthMonitor* monitor, uint32_t* count);
const char* idcu_health_status_to_string(idcu_HealthStatus status);
int idcu_health_reset_error_count(idcu_HealthMonitor* monitor, uint32_t module_id);

#ifdef __cplusplus
}
#endif // IDCU_MONITOR_HEALTH_CHECK_H

#endif // IDCU_MONITOR_HEALTH_CHECK_H
