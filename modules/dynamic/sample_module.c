#include "module/module_def.h"
#include "utils/log.h"
#include <stdio.h>

static int g_initialized = 0;

static int sample_module_init(void)
{
    IDCU_LOG_INFO("Sample module: Initializing...");
    g_initialized = 1;
    IDCU_LOG_INFO("Sample module: Initialized successfully");
    return IDCU_ERR_SUCCESS;
}

static int sample_module_run(void)
{
    if (!g_initialized) {
        IDCU_LOG_ERROR("Sample module: Not initialized");
        return IDCU_ERR_MODULE_INIT;
    }
    IDCU_LOG_INFO("Sample module: Running...");
    return IDCU_ERR_SUCCESS;
}

static int sample_module_stop(void)
{
    IDCU_LOG_INFO("Sample module: Stopping...");
    g_initialized = 0;
    IDCU_LOG_INFO("Sample module: Stopped successfully");
    return IDCU_ERR_SUCCESS;
}

#ifdef _WIN32
__declspec(dllexport)
#endif
const idcu_ModuleInterface module_interface = {
    .name = "sample_module",
    .dependencies = NULL,
    .dependency_count = 0,
    .init = sample_module_init,
    .run = sample_module_run,
    .stop = sample_module_stop
};
