#include "module/module_def.h"
#include "utils/log.h"
#include <stdio.h>

static int test_init_fail_module_init(void)
{
    IDCU_LOG_ERROR("Test Init Fail Module: Initialization failed intentionally");
    return IDCU_ERR_MODULE_INIT;
}

static int test_init_fail_module_run(void)
{
    IDCU_LOG_INFO("Test Init Fail Module: Running...");
    return IDCU_ERR_SUCCESS;
}

static int test_init_fail_module_stop(void)
{
    IDCU_LOG_INFO("Test Init Fail Module: Stopping...");
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
    .name = "test_module_init_fail",
    .dependencies = NULL,
    .dependency_count = 0,
    .init = test_init_fail_module_init,
    .run = test_init_fail_module_run,
    .stop = test_init_fail_module_stop
};
