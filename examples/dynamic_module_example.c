#include "module/dynamic_module.h"
#include "utils/log.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define SAMPLE_MODULE_NAME "sample_module.dll"
#else
#define SAMPLE_MODULE_NAME "libsample_module.so"
#endif

int main(void)
{
    idcu_log_init(NULL, IDCU_LOG_DEBUG);
    IDCU_LOG_INFO("=== Dynamic Module Loader Example ===");

    idcu_DynamicLoader loader;
    int ret = idcu_dynamic_loader_init(&loader, NULL);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("Failed to initialize dynamic loader: %d", ret);
        return 1;
    }
    IDCU_LOG_INFO("Dynamic loader initialized successfully");

    char module_path[256];
#ifdef _WIN32
    snprintf(module_path, sizeof(module_path), "%s", SAMPLE_MODULE_NAME);
#else
    snprintf(module_path, sizeof(module_path), "./%s", SAMPLE_MODULE_NAME);
#endif

    IDCU_LOG_INFO("Loading module: %s", module_path);
    ret = idcu_dynamic_loader_load_module(&loader, "sample_module", module_path);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("Failed to load module: %d", ret);
        IDCU_LOG_INFO("Note: Make sure you have built the sample_module.dll first");
        idcu_dynamic_loader_destroy(&loader);
        return 1;
    }
    IDCU_LOG_INFO("Module loaded successfully");

    idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&loader, "sample_module");
    if (mod) {
        IDCU_LOG_INFO("Found module: %s", mod->name);
        IDCU_LOG_INFO("Module state: %d", mod->state);

        ret = idcu_dynamic_module_init(mod);
        if (ret == IDCU_ERR_SUCCESS) {
            IDCU_LOG_INFO("Module initialized");

            ret = idcu_dynamic_module_run(mod);
            if (ret == IDCU_ERR_SUCCESS) {
                IDCU_LOG_INFO("Module running");

                idcu_dynamic_module_stop(mod);
                IDCU_LOG_INFO("Module stopped");
            }
        }
    }

    int count = idcu_dynamic_loader_get_count(&loader);
    IDCU_LOG_INFO("Loaded modules count: %d", count);

    ret = idcu_dynamic_loader_unload_module(&loader, "sample_module");
    if (ret == IDCU_ERR_SUCCESS) {
        IDCU_LOG_INFO("Module unloaded successfully");
    }

    count = idcu_dynamic_loader_get_count(&loader);
    IDCU_LOG_INFO("Modules after unload: %d", count);

    idcu_dynamic_loader_destroy(&loader);
    IDCU_LOG_INFO("Dynamic loader destroyed");

    IDCU_LOG_INFO("=== Example completed ===");
    return 0;
}
