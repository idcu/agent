#include "idcu/sdk/sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool initialized;
} TaskQueueData;

static int task_queue_init(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Initializing task queue module");

    TaskQueueData* data = malloc(sizeof(TaskQueueData));
    if (!data) {
        return IDCU_ERR_MEMORY;
    }
    memset(data, 0, sizeof(TaskQueueData));

    data->initialized = true;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_log_info(ctx, "Task queue module initialized");
    return IDCU_ERR_OK;
}

static int task_queue_start(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Starting task queue module");
    return IDCU_ERR_OK;
}

static int task_queue_stop(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Stopping task queue module");
    return IDCU_ERR_OK;
}

static void task_queue_destroy(idcu_SdkContext* ctx, void* user_data) {
    (void)user_data;
    idcu_sdk_log_info(ctx, "Destroying task queue module");

    TaskQueueData* data = idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    task_queue,
    "1.0.0",
    "Task queue module",
    task_queue_init,
    task_queue_start,
    task_queue_stop,
    task_queue_destroy,
    NULL
);
