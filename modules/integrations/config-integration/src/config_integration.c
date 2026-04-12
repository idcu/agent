#include "idcu/sdk/sdk.h"
#include "idcu/config/config.h"
#include "idcu/yaml/yaml.h"
#include "idcu/json/json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
    char* config_path;
    bool hot_reload_enabled;
} ConfigIntegrationData;

static int config_integration_init(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Initializing config integration");

    ConfigIntegrationData* data = malloc(sizeof(ConfigIntegrationData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(ConfigIntegrationData));

    const char* config_path = NULL;
    idcu_sdk_get_config_string(ctx, "config.path", &config_path);
    data->config_path = config_path ? strdup(config_path) : strdup("config/default");

    bool hot_reload = false;
    idcu_sdk_get_config_bool(ctx, "config.hot_reload", &hot_reload);
    data->hot_reload_enabled = hot_reload;

    idcu_ErrorCode err = idcu_config_init(data->config_path);
    if (err != IDCU_ERR_OK) {
        free(data->config_path);
        free(data);
        return err;
    }

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Config integration initialized");
    return IDCU_ERR_OK;
}

static int config_integration_start(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Starting config integration");
    return IDCU_ERR_OK;
}

static int config_integration_stop(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Stopping config integration");
    return IDCU_ERR_OK;
}

static void config_integration_destroy(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Destroying config integration");

    ConfigIntegrationData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        if (data->initialized) {
            idcu_config_shutdown();
        }
        free(data->config_path);
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    config_integration,
    "1.0.0",
    "Config system integration module",
    config_integration_init,
    config_integration_start,
    config_integration_stop,
    config_integration_destroy,
    NULL
);
