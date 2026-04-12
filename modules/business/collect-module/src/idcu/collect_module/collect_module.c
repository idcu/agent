#include "idcu/sdk/sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
} CollectModuleData;

static int collect_module_init(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Initializing collect module");

    CollectModuleData* data = malloc(sizeof(CollectModuleData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(CollectModuleData));

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Collect module initialized");
    return IDCU_ERR_OK;
}

static int collect_module_start(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Starting collect module");
    return IDCU_ERR_OK;
}

static int collect_module_stop(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Stopping collect module");
    return IDCU_ERR_OK;
}

static void collect_module_destroy(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Destroying collect module");

    CollectModuleData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    collect_module,
    "1.0.0",
    "Collect module",
    collect_module_init,
    collect_module_start,
    collect_module_stop,
    collect_module_destroy,
    NULL
);
