
#include "module/module_def.h"
#include "common/error_code.h"
#include "utils/log.h"

static int g_initialized = 0;

static int sandbox_module_init(void) {
    idcu_log_info("[sandbox] Initializing sandbox module");
    g_initialized = 1;
    return IDCU_ERR_OK;
}

static int sandbox_module_start(void) {
    idcu_log_info("[sandbox] Starting sandbox module");
    if (!g_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    return IDCU_ERR_OK;
}

static int sandbox_module_stop(void) {
    idcu_log_info("[sandbox] Stopping sandbox module");
    return IDCU_ERR_OK;
}

static void sandbox_module_destroy(void) {
    idcu_log_info("[sandbox] Destroying sandbox module");
    g_initialized = 0;
}

IDCU_MODULE_DEFINE(
    "sandbox",
    "1.0.0",
    "Sandbox security module",
    sandbox_module_init,
    sandbox_module_start,
    sandbox_module_stop,
    sandbox_module_destroy
);
