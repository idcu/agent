#include "module_def.h"
#include "idcu/log/log.h"
#include <stdio.h>

static int g_initialized = 0;
static int g_run_count = 0;

static int test_good_module_init(void)
{
    IDCU_LOG_INFO("Test Good Module: Initializing...");
    g_initialized = 1;
    g_run_count = 0;
    IDCU_LOG_INFO("Test Good Module: Initialized successfully");
    return IDCU_ERR_SUCCESS;
}

static int test_good_module_run(void)
{
    if (!g_initialized) {
        IDCU_LOG_ERROR("Test Good Module: Not initialized");
        return IDCU_ERR_MODULE_INIT;
    }
    g_run_count++;
    IDCU_LOG_INFO("Test Good Module: Running (count=%d)...", g_run_count);
    return IDCU_ERR_SUCCESS;
}

static int test_good_module_stop(void)
{
    IDCU_LOG_INFO("Test Good Module: Stopping...");
    g_initialized = 0;
    IDCU_LOG_INFO("Test Good Module: Stopped successfully");
    return IDCU_ERR_SUCCESS;
}

#ifdef _WIN32
#define MODULE_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define MODULE_EXPORT __attribute__((visibility("default")))
#else
#define MODULE_EXPORT
#endif

MODULE_EXPORT const idcu_ModuleInterface module_interface = {
    .name = "test_module_good",
    .version = IDCU_MODULE_VERSION(1, 0, 0),
    .dependencies = NULL,
    .dependency_count = 0,
    .init = test_good_module_init,
    .run = test_good_module_run,
    .stop = test_good_module_stop
};
