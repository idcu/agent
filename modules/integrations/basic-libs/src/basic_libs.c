#include "idcu/sdk/sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
} BasicLibsData;

static int basic_libs_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing basic libs integration");

    BasicLibsData* data = malloc(sizeof(BasicLibsData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(BasicLibsData));

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Basic libs integration initialized");
    return IDCU_ERR_OK;
}

static int basic_libs_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting basic libs integration");
    return IDCU_ERR_OK;
}

static void basic_libs_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping basic libs integration");
}

static void basic_libs_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying basic libs integration");

    BasicLibsData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    basic_libs,
    "1.0.0",
    "Basic libraries integration module",
    basic_libs_init,
    basic_libs_start,
    basic_libs_stop,
    basic_libs_destroy,
    NULL
);
