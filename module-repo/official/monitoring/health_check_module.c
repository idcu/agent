
#include "module_def.h"
#include "idcu/common/error_code.h"
#include "idcu/log/log.h"
#include "health_check.h"

static idcu_HealthMonitor g_health_monitor;
static int g_initialized = 0;

static int health_check_module_init(void) {
    idcu_log_info("[health_check] Initializing health check module");
    
    int ret = idcu_health_monitor_init(&g_health_monitor);
    if (ret != IDCU_ERR_OK) {
        idcu_log_error("[health_check] Failed to initialize health monitor");
        return ret;
    }
    
    g_initialized = 1;
    return IDCU_ERR_OK;
}

static int health_check_module_start(void) {
    idcu_log_info("[health_check] Starting health check module");
    
    if (!g_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    return IDCU_ERR_OK;
}

static int health_check_module_stop(void) {
    idcu_log_info("[health_check] Stopping health check module");
    return IDCU_ERR_OK;
}

static void health_check_module_destroy(void) {
    idcu_log_info("[health_check] Destroying health check module");
    
    if (g_initialized) {
        idcu_health_monitor_destroy(&g_health_monitor);
        g_initialized = 0;
    }
}

IDCU_MODULE_DEFINE(
    "health_check",
    "1.0.0",
    "Health check monitoring module",
    health_check_module_init,
    health_check_module_start,
    health_check_module_stop,
    health_check_module_destroy
);
