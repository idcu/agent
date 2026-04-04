#include "sdk.h"

typedef struct {
    int counter;
} SimpleExampleData;

static int simple_example_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing simple example module");
    
    SimpleExampleData* data = (SimpleExampleData*)malloc(sizeof(SimpleExampleData));
    if (!data) {
        idcu_sdk_log_error(ctx, "Failed to allocate memory");
        return IDCU_ERR_NO_MEMORY;
    }
    
    data->counter = 0;
    idcu_sdk_set_user_data(ctx, data);
    
    return IDCU_ERR_OK;
}

static int simple_example_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting simple example module");
    return IDCU_ERR_OK;
}

static int simple_example_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping simple example module");
    return IDCU_ERR_OK;
}

static void simple_example_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying simple example module");
    
    SimpleExampleData* data = (SimpleExampleData*)idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    sdk_simple_example,
    "1.0.0",
    "A simple example module using IDCU SDK",
    simple_example_init,
    simple_example_start,
    simple_example_stop,
    simple_example_destroy
);
