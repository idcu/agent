#include "idcu/sdk/sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
} CacheModuleData;

static int cache_module_init(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Initializing cache module");

    CacheModuleData* data = malloc(sizeof(CacheModuleData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(CacheModuleData));

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Cache module initialized");
    return IDCU_ERR_OK;
}

static int cache_module_start(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Starting cache module");
    return IDCU_ERR_OK;
}

static int cache_module_stop(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Stopping cache module");
    return IDCU_ERR_OK;
}

static void cache_module_destroy(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Destroying cache module");

    CacheModuleData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    cache_module,
    "1.0.0",
    "Cache module",
    cache_module_init,
    cache_module_start,
    cache_module_stop,
    cache_module_destroy,
    NULL
);
