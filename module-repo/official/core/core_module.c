
#include "module_def.h"
#include "error_code.h"
#include "log.h"

static int g_core_counter = 0;

static int core_module_init(void) {
    idcu_log_info("[core_module] Initializing core module");
    g_core_counter = 0;
    return IDCU_ERR_OK;
}

static int core_module_start(void) {
    idcu_log_info("[core_module] Starting core module");
    return IDCU_ERR_OK;
}

static int core_module_stop(void) {
    idcu_log_info("[core_module] Stopping core module");
    return IDCU_ERR_OK;
}

static void core_module_destroy(void) {
    idcu_log_info("[core_module] Destroying core module");
}

IDCU_MODULE_DEFINE(
    "core_module",
    "1.0.0",
    "Core functionality module providing basic services",
    core_module_init,
    core_module_start,
    core_module_stop,
    core_module_destroy
);
