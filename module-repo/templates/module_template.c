
#include "module_def.h"
#include "idcu/common/error_code.h"
#include "idcu/log/log.h"

typedef struct {
    int initialized;
} MyModuleContext;

static MyModuleContext g_ctx;

static int my_module_init(void) {
    idcu_log_info("Initializing my module");
    
    g_ctx.initialized = 1;
    
    return IDCU_ERR_OK;
}

static int my_module_start(void) {
    idcu_log_info("Starting my module");
    
    if (!g_ctx.initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    return IDCU_ERR_OK;
}

static int my_module_stop(void) {
    idcu_log_info("Stopping my module");
    
    g_ctx.initialized = 0;
    
    return IDCU_ERR_OK;
}

static void my_module_destroy(void) {
    idcu_log_info("Destroying my module");
}

IDCU_REGISTER_MODULE(my_module, IDCU_MODULE_VERSION(1, 0, 0), my_module_init, my_module_start, my_module_stop);
