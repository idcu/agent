
#include "module_def.h"
#include "idcu/common/error_code.h"
#include "idcu/log/log.h"

static int g_initialized = 0;

static int network_layer_module_init(void) {
    idcu_log_info("[network_layer] Initializing network layer module");
    g_initialized = 1;
    return IDCU_ERR_OK;
}

static int network_layer_module_start(void) {
    idcu_log_info("[network_layer] Starting network layer module");
    if (!g_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return IDCU_ERR_OK;
}

static int network_layer_module_stop(void) {
    idcu_log_info("[network_layer] Stopping network layer module");
    return IDCU_ERR_OK;
}

static void network_layer_module_destroy(void) {
    idcu_log_info("[network_layer] Destroying network layer module");
    g_initialized = 0;
}

IDCU_MODULE_DEFINE(
    "network_layer",
    "1.0.0",
    "TCP/UDP network communication module",
    network_layer_module_init,
    network_layer_module_start,
    network_layer_module_stop,
    network_layer_module_destroy
);
