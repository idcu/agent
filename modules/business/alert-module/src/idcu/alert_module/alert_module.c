#include "idcu/sdk/sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
} AlertModuleData;

static int alert_module_init(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Initializing alert module");

    AlertModuleData* data = malloc(sizeof(AlertModuleData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(AlertModuleData));

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Alert module initialized");
    return IDCU_ERR_OK;
}

static int alert_module_start(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Starting alert module");
    return IDCU_ERR_OK;
}

static int alert_module_stop(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Stopping alert module");
    return IDCU_ERR_OK;
}

static void alert_module_destroy(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Destroying alert module");

    AlertModuleData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    alert_module,
    "1.0.0",
    "Alert module",
    alert_module_init,
    alert_module_start,
    alert_module_stop,
    alert_module_destroy,
    NULL
);
