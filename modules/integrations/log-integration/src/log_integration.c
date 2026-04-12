#include "idcu/sdk/sdk.h"
#include "idcu/log/log.h"
#include "idcu/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
    idcu_LogLevel level;
    char* log_file;
} LogIntegrationData;

static int log_integration_init(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Initializing log integration");

    LogIntegrationData* data = malloc(sizeof(LogIntegrationData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(LogIntegrationData));

    const char* level_str = NULL;
    idcu_sdk_get_config_string(ctx, "log.level", &level_str);
    data->level = IDCU_LOG_INFO;
    if (level_str) {
        if (strcmp(level_str, "debug") == 0) data->level = IDCU_LOG_DEBUG;
        else if (strcmp(level_str, "warn") == 0) data->level = IDCU_LOG_WARN;
        else if (strcmp(level_str, "error") == 0) data->level = IDCU_LOG_ERROR;
        else if (strcmp(level_str, "fatal") == 0) data->level = IDCU_LOG_FATAL;
    }

    const char* log_file = NULL;
    idcu_sdk_get_config_string(ctx, "log.file", &log_file);
    data->log_file = log_file ? strdup(log_file) : NULL;

    int err = idcu_log_init(data->log_file, data->level);
    if (err != IDCU_ERR_OK) {
        free(data->log_file);
        free(data);
        return err;
    }

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Log integration initialized");
    return IDCU_ERR_OK;
}

static int log_integration_start(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Starting log integration");
    return IDCU_ERR_OK;
}

static int log_integration_stop(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Stopping log integration");
    return IDCU_ERR_OK;
}

static void log_integration_destroy(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Destroying log integration");

    LogIntegrationData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        if (data->initialized) {
            idcu_log_shutdown();
        }
        free(data->log_file);
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    log_integration,
    "1.0.0",
    "Log system integration module",
    log_integration_init,
    log_integration_start,
    log_integration_stop,
    log_integration_destroy,
    NULL
);
