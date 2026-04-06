
#include "module_def.h"
#include "idcu/common/error_code.h"
#include "idcu/log/log.h"

static int g_initialized = 0;

static int distributed_node_module_init(void) {
    idcu_log_info("[distributed_node] Initializing distributed node module");
    g_initialized = 1;
    return IDCU_ERR_OK;
}

static int distributed_node_module_start(void) {
    idcu_log_info("[distributed_node] Starting distributed node module");
    if (!g_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return IDCU_ERR_OK;
}

static int distributed_node_module_stop(void) {
    idcu_log_info("[distributed_node] Stopping distributed node module");
    return IDCU_ERR_OK;
}

static void distributed_node_module_destroy(void) {
    idcu_log_info("[distributed_node] Destroying distributed node module");
    g_initialized = 0;
}

IDCU_MODULE_DEFINE(
    "distributed_node",
    "1.0.0",
    "Distributed node communication module",
    distributed_node_module_init,
    distributed_node_module_start,
    distributed_node_module_stop,
    distributed_node_module_destroy
);
