#include "idcu/log/log.h"
#include "module_def.h"
#include <stdio.h>

static int g_initialized = 0;

static int sample_module_init(void) {
    IDCU_LOG_INFO("Sample module: Initializing...");
    g_initialized = 1;
    IDCU_LOG_INFO("Sample module: Initialized successfully");
    return IDCU_ERR_SUCCESS;
}

static int sample_module_run(void) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("Sample module: Not initialized");
        return IDCU_ERR_MODULE_INIT;
    }
    IDCU_LOG_INFO("Sample module: Running...");
    return IDCU_ERR_SUCCESS;
}

static int sample_module_stop(void) {
    IDCU_LOG_INFO("Sample module: Stopping...");
    g_initialized = 0;
    IDCU_LOG_INFO("Sample module: Stopped successfully");
    return IDCU_ERR_SUCCESS;
}

#ifdef _WIN32
#define MODULE_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define MODULE_EXPORT __attribute__((visibility("default")))
#else
#define MODULE_EXPORT
#endif

MODULE_EXPORT const idcu_ModuleInterface module_interface = {.name = "sample_module",
                                                             .version =
                                                                 IDCU_MODULE_VERSION(1, 0, 0),
                                                             .dependencies = NULL,
                                                             .dependency_count = 0,
                                                             .init = sample_module_init,
                                                             .run = sample_module_run,
                                                             .stop = sample_module_stop};
