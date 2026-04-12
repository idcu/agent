#include "idcu/sdk/sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
} StorageModuleData;

static int storage_module_init(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Initializing storage module");

    StorageModuleData* data = malloc(sizeof(StorageModuleData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(StorageModuleData));

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Storage module initialized");
    return IDCU_ERR_OK;
}

static int storage_module_start(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Starting storage module");
    return IDCU_ERR_OK;
}

static int storage_module_stop(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Stopping storage module");
    return IDCU_ERR_OK;
}

static void storage_module_destroy(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Destroying storage module");

    StorageModuleData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    storage_module,
    "1.0.0",
    "Storage module",
    storage_module_init,
    storage_module_start,
    storage_module_stop,
    storage_module_destroy,
    NULL
);
