#include "idcu/sdk/sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
} HttpClientModuleData;

static int http_client_module_init(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Initializing HTTP client module");

    HttpClientModuleData* data = malloc(sizeof(HttpClientModuleData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(HttpClientModuleData));

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "HTTP client module initialized");
    return IDCU_ERR_OK;
}

static int http_client_module_start(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Starting HTTP client module");
    return IDCU_ERR_OK;
}

static int http_client_module_stop(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Stopping HTTP client module");
    return IDCU_ERR_OK;
}

static void http_client_module_destroy(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Destroying HTTP client module");

    HttpClientModuleData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    http_client_module,
    "1.0.0",
    "HTTP client module",
    http_client_module_init,
    http_client_module_start,
    http_client_module_stop,
    http_client_module_destroy,
    NULL
);
