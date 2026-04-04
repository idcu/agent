
#include "module_def.h"
#include "error_code.h"
#include "log.h"

static int g_log_counter = 0;

static int log_module_init(void) {
    idcu_log_info("[log_module] Initializing log module");
    g_log_counter = 0;
    return IDCU_ERR_OK;
}

static int log_module_start(void) {
    idcu_log_info("[log_module] Starting log module");
    return IDCU_ERR_OK;
}

static int log_module_stop(void) {
    idcu_log_info("[log_module] Stopping log module");
    return IDCU_ERR_OK;
}

static void log_module_destroy(void) {
    idcu_log_info("[log_module] Destroying log module");
}

IDCU_MODULE_DEFINE(
    "log_module",
    "1.0.0",
    "Logging management module",
    log_module_init,
    log_module_start,
    log_module_stop,
    log_module_destroy
);
