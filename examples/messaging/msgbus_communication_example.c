/**
 * @file msgbus_communication_example.c
 * @brief 消息总线跨模块通信示例
 *
 * 本示例演示如何使用IDCU的消息总线功能实现模块间的通信，
 * 包括点对点消息发送、广播消息、消息接收和处理等功能。
 */

#include "sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

#define MSG_TYPE_DATA 1
#define MSG_TYPE_COMMAND 2
#define MSG_TYPE_RESPONSE 3

typedef struct {
    uint32_t msg_type;
    int32_t int_value;
    char str_value[64];
} MessageData;

typedef struct {
    uint32_t received_count;
    uint32_t sent_count;
} CommModuleData;

static void message_handler(idcu_SdkContext* ctx, const idcu_SdkMessage* msg, void* user_data) {
    CommModuleData* data = (CommModuleData*)user_data;
    
    if (!msg || !msg->data || msg->size < sizeof(MessageData)) {
        idcu_sdk_log_warn(ctx, "Received invalid message");
        return;
    }
    
    MessageData* msg_data = (MessageData*)msg->data;
    data->received_count++;
    
    idcu_sdk_log_info(ctx, "Received message from module %u", msg->source_id);
    idcu_sdk_log_info(ctx, "  Type: %u", msg_data->msg_type);
    idcu_sdk_log_info(ctx, "  Int value: %d", msg_data->int_value);
    idcu_sdk_log_info(ctx, "  Str value: %s", msg_data->str_value);
    idcu_sdk_log_info(ctx, "  Total received: %u", data->received_count);
}

static int send_test_message(idcu_SdkContext* ctx, CommModuleData* data, const char* target_module) {
    MessageData msg_data;
    memset(&msg_data, 0, sizeof(msg_data));
    
    msg_data.msg_type = MSG_TYPE_DATA;
    msg_data.int_value = data->sent_count + 1;
    snprintf(msg_data.str_value, sizeof(msg_data.str_value), "Hello from sender! #%u", data->sent_count + 1);
    
    int ret = idcu_sdk_send_message(
        ctx,
        target_module,
        IDCU_MSG_PRIO_NORMAL,
        (const uint8_t*)&msg_data,
        sizeof(msg_data)
    );
    
    if (ret == IDCU_ERR_OK) {
        data->sent_count++;
        idcu_sdk_log_info(ctx, "Sent message to %s (total: %u)", target_module, data->sent_count);
    } else {
        idcu_sdk_log_error(ctx, "Failed to send message to %s: %d", target_module, ret);
    }
    
    return ret;
}

static int broadcast_test_message(idcu_SdkContext* ctx, CommModuleData* data) {
    MessageData msg_data;
    memset(&msg_data, 0, sizeof(msg_data));
    
    msg_data.msg_type = MSG_TYPE_COMMAND;
    msg_data.int_value = 999;
    snprintf(msg_data.str_value, sizeof(msg_data.str_value), "Broadcast message #%u", data->sent_count + 1);
    
    int ret = idcu_sdk_broadcast_message(
        ctx,
        IDCU_MSG_PRIO_HIGH,
        (const uint8_t*)&msg_data,
        sizeof(msg_data)
    );
    
    if (ret == IDCU_ERR_OK) {
        data->sent_count++;
        idcu_sdk_log_info(ctx, "Broadcast message sent (total: %u)", data->sent_count);
    } else {
        idcu_sdk_log_error(ctx, "Failed to broadcast message: %d", ret);
    }
    
    return ret;
}

static int comm_module_init(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Initializing communication module");
    
    CommModuleData* data = (CommModuleData*)malloc(sizeof(CommModuleData));
    if (!data) {
        idcu_sdk_log_error(ctx, "Failed to allocate memory");
        return IDCU_ERR_NO_MEMORY;
    }
    
    memset(data, 0, sizeof(CommModuleData));
    
    int ret = idcu_sdk_register_message_handler(ctx, message_handler, data);
    if (ret != IDCU_ERR_OK) {
        idcu_sdk_log_error(ctx, "Failed to register message handler: %d", ret);
        free(data);
        return ret;
    }
    
    idcu_sdk_set_user_data(ctx, data);
    
    idcu_sdk_log_info(ctx, "Communication module initialized");
    return IDCU_ERR_OK;
}

static int comm_module_start(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Starting communication module");
    
    CommModuleData* data = (CommModuleData*)idcu_sdk_get_user_data(ctx);
    if (data) {
        idcu_sdk_log_info(ctx, "Module name: %s", idcu_sdk_get_module_name(ctx));
        idcu_sdk_log_info(ctx, "Module ID: %u", idcu_sdk_get_module_id(ctx));
    }
    
    return IDCU_ERR_OK;
}

static int comm_module_stop(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Stopping communication module");
    
    CommModuleData* data = (CommModuleData*)idcu_sdk_get_user_data(ctx);
    if (data) {
        idcu_sdk_log_info(ctx, "=== Communication Statistics ===");
        idcu_sdk_log_info(ctx, "Messages sent: %u", data->sent_count);
        idcu_sdk_log_info(ctx, "Messages received: %u", data->received_count);
        idcu_sdk_log_info(ctx, "================================");
    }
    
    return IDCU_ERR_OK;
}

static void comm_module_destroy(idcu_SdkContext* ctx) {
    idcu_sdk_log_info(ctx, "Destroying communication module");
    
    CommModuleData* data = (CommModuleData*)idcu_sdk_get_user_data(ctx);
    if (data) {
        free(data);
    }
}

IDCU_SDK_MODULE_DEFINE(
    msgbus_communication,
    "1.0.0",
    "A message bus communication example module",
    comm_module_init,
    comm_module_start,
    comm_module_stop,
    comm_module_destroy
);

int main(void) {
    printf("====================================\n");
    printf("Message Bus Communication Example\n");
    printf("====================================\n\n");
    printf("This example demonstrates how to use the message bus for inter-module communication.\n\n");
    printf("Key features demonstrated:\n");
    printf("  - Point-to-point message sending\n");
    printf("  - Broadcast messaging\n");
    printf("  - Message receiving and handling\n");
    printf("  - Message priority levels\n");
    printf("  - Communication statistics\n\n");
    printf("To use this, create two instances of this module with different names,\n");
    printf("and they will be able to communicate with each other via the message bus.\n\n");
    
    return 0;
}
