
#include "module/module_def.h"
#include "common/error_code.h"
#include "utils/log.h"

static int g_initialized = 0;

static int node_discovery_module_init(void) {
    idcu_log_info("[node_discovery] Initializing node discovery module");
    g_initialized = 1;
    return IDCU_ERR_OK;
}

static int node_discovery_module_start(void) {
    idcu_log_info("[node_discovery] Starting node discovery module");
    if (!g_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return IDCU_ERR_OK;
}

static int node_discovery_module_stop(void) {
    idcu_log_info("[node_discovery] Stopping node discovery module");
    return IDCU_ERR_OK;
}

static void node_discovery_module_destroy(void) {
    idcu_log_info("[node_discovery] Destroying node discovery module");
    g_initialized = 0;
}

IDCU_MODULE_DEFINE(
    "node_discovery",
    "1.0.0",
    "Node discovery protocol module",
    node_discovery_module_init,
    node_discovery_module_start,
    node_discovery_module_stop,
    node_discovery_module_destroy
);
