#include "sdk.h"
#include <string.h>

typedef struct {
    int message_count;
} MessagingExampleData;

static void message_handler(idcu_SdkContext *ctx, const idcu_SdkMessage *msg, void *user_data) {
    MessagingExampleData *data = (MessagingExampleData *)user_data;

    idcu_sdk_log_info(ctx, "Received message from module %u, priority %d", msg->source_id,
                      msg->priority);

    if (msg->data && msg->size > 0) {
        idcu_sdk_log_info(ctx, "Message data: %.*s", (int)msg->size, msg->data);
    }

    data->message_count++;
}

static int messaging_example_init(idcu_SdkContext *ctx) {
    idcu_sdk_log_info(ctx, "Initializing messaging example module");

    MessagingExampleData *data = (MessagingExampleData *)malloc(sizeof(MessagingExampleData));
    if (!data) {
        idcu_sdk_log_error(ctx, "Failed to allocate memory");
        return IDCU_ERR_NO_MEMORY;
    }

    data->message_count = 0;
    idcu_sdk_set_user_data(ctx, data);

    idcu_sdk_register_message_handler(ctx, message_handler, data);

    return IDCU_ERR_OK;
}

static int messaging_example_start(idcu_SdkContext *ctx) {
    idcu_sdk_log_info(ctx, "Starting messaging example module");

    const char *test_msg = "Hello from SDK!";
    idcu_sdk_broadcast_message(ctx, IDCU_MSG_PRIO_NORMAL, (const uint8_t *)test_msg,
                               strlen(test_msg));

    return IDCU_ERR_OK;
}

static int messaging_example_stop(idcu_SdkContext *ctx) {
    idcu_sdk_log_info(ctx, "Stopping messaging example module");
    return IDCU_ERR_OK;
}

static void messaging_example_destroy(idcu_SdkContext *ctx) {
    idcu_sdk_log_info(ctx, "Destroying messaging example module");

    MessagingExampleData *data = (MessagingExampleData *)idcu_sdk_get_user_data(ctx);
    if (data) {
        idcu_sdk_log_info(ctx, "Total messages received: %d", data->message_count);
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(sdk_messaging_example, "1.0.0", "A messaging example module using IDCU SDK",
                       messaging_example_init, messaging_example_start, messaging_example_stop,
                       messaging_example_destroy);
