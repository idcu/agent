#include "idcu/sdk/sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
} HeartbeatData;

static int heartbeat_init(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Initializing heartbeat module");

    HeartbeatData* data = malloc(sizeof(HeartbeatData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(HeartbeatData));

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Heartbeat module initialized");
    return IDCU_ERR_OK;
}

static int heartbeat_start(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Starting heartbeat module");
    return IDCU_ERR_OK;
}

static int heartbeat_stop(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Stopping heartbeat module");
    return IDCU_ERR_OK;
}

static void heartbeat_destroy(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Destroying heartbeat module");

    HeartbeatData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    heartbeat,
    "1.0.0",
    "Heartbeat module",
    heartbeat_init,
    heartbeat_start,
    heartbeat_stop,
    heartbeat_destroy,
    NULL
);
