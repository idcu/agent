
#include "module_def.h"
#include "error_code.h"
#include "log.h"
#include "metrics.h"

static int g_initialized = 0;

static int metrics_module_init(void) {
    idcu_log_info("[metrics] Initializing metrics module");
    
    int ret = idcu_global_metrics_init();
    if (ret != IDCU_ERR_OK) {
        idcu_log_error("[metrics] Failed to initialize metrics collector");
        return ret;
    }
    
    ret = idcu_global_metrics_register_default();
    if (ret != IDCU_ERR_OK) {
        idcu_log_error("[metrics] Failed to register default metrics");
        return ret;
    }
    
    g_initialized = 1;
    return IDCU_ERR_OK;
}

static int metrics_module_start(void) {
    idcu_log_info("[metrics] Starting metrics module");
    
    if (!g_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    return IDCU_ERR_OK;
}

static int metrics_module_stop(void) {
    idcu_log_info("[metrics] Stopping metrics module");
    return IDCU_ERR_OK;
}

static void metrics_module_destroy(void) {
    idcu_log_info("[metrics] Destroying metrics module");
    
    if (g_initialized) {
        idcu_global_metrics_destroy();
        g_initialized = 0;
    }
}

IDCU_MODULE_DEFINE(
    "metrics",
    "1.0.0",
    "Performance metrics collection module",
    metrics_module_init,
    metrics_module_start,
    metrics_module_stop,
    metrics_module_destroy
);
