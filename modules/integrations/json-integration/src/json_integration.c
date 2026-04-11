#include "idcu/sdk/sdk.h"
#include "idcu/json/json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
} JsonIntegrationData;

static int json_integration_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing JSON integration");

    JsonIntegrationData* data = malloc(sizeof(JsonIntegrationData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(JsonIntegrationData));

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "JSON integration initialized");
    return IDCU_ERR_OK;
}

static int json_integration_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting JSON integration");
    return IDCU_ERR_OK;
}

static void json_integration_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping JSON integration");
}

static void json_integration_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying JSON integration");

    JsonIntegrationData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    json_integration,
    "1.0.0",
    "JSON integration module",
    json_integration_init,
    json_integration_start,
    json_integration_stop,
    json_integration_destroy,
    NULL
);
