#include "idcu/sdk/sdk.h"
#include "idcu/coroutine/coroutine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
} SdkCompleteData;

static int sdk_complete_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing SDK complete integration");

    SdkCompleteData* data = malloc(sizeof(SdkCompleteData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(SdkCompleteData));

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "SDK complete integration initialized");
    return IDCU_ERR_OK;
}

static int sdk_complete_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting SDK complete integration");
    return IDCU_ERR_OK;
}

static void sdk_complete_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping SDK complete integration");
}

static void sdk_complete_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying SDK complete integration");

    SdkCompleteData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    sdk_complete,
    "1.0.0",
    "SDK complete integration module",
    sdk_complete_init,
    sdk_complete_start,
    sdk_complete_stop,
    sdk_complete_destroy,
    NULL
);
