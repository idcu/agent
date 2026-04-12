#include "idcu/sdk/sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
} MetricsModuleData;

static int metrics_module_init(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Initializing metrics module");

    MetricsModuleData* data = malloc(sizeof(MetricsModuleData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(MetricsModuleData));

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Metrics module initialized");
    return IDCU_ERR_OK;
}

static int metrics_module_start(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Starting metrics module");
    return IDCU_ERR_OK;
}

static int metrics_module_stop(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Stopping metrics module");
    return IDCU_ERR_OK;
}

static void metrics_module_destroy(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Destroying metrics module");

    MetricsModuleData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    metrics_module,
    "1.0.0",
    "Metrics module",
    metrics_module_init,
    metrics_module_start,
    metrics_module_stop,
    metrics_module_destroy,
    NULL
);
