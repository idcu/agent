#include "idcu/sdk/sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
} HealthcheckModuleData;

static int healthcheck_module_init(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Initializing healthcheck module");

    HealthcheckModuleData* data = malloc(sizeof(HealthcheckModuleData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(HealthcheckModuleData));

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Healthcheck module initialized");
    return IDCU_ERR_OK;
}

static int healthcheck_module_start(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Starting healthcheck module");
    return IDCU_ERR_OK;
}

static int healthcheck_module_stop(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Stopping healthcheck module");
    return IDCU_ERR_OK;
}

static void healthcheck_module_destroy(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Destroying healthcheck module");

    HealthcheckModuleData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    healthcheck_module,
    "1.0.0",
    "Healthcheck module",
    healthcheck_module_init,
    healthcheck_module_start,
    healthcheck_module_stop,
    healthcheck_module_destroy,
    NULL
);
