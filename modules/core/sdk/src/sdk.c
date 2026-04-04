#include "sdk.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct idcu_sdk_context {
    void* user_data;
    uint32_t module_id;
    char module_name[IDCU_MODULE_NAME_MAX];
    idcu_SdkMessageHandler msg_handler;
    void* msg_handler_user_data;
} idcu_SdkContext;

static idcu_MessageBus* g_msg_bus = NULL;

idcu_SdkContext* idcu_sdk_create_context(void) {
    idcu_SdkContext* ctx = (idcu_SdkContext*)malloc(sizeof(idcu_SdkContext));
    if (!ctx) {
        return NULL;
    }
    
    memset(ctx, 0, sizeof(idcu_SdkContext));
    ctx->module_id = 0;
    ctx->user_data = NULL;
    ctx->msg_handler = NULL;
    ctx->msg_handler_user_data = NULL;
    
    return ctx;
}

void idcu_sdk_destroy_context(idcu_SdkContext* ctx) {
    if (ctx) {
        free(ctx);
    }
}

void* idcu_sdk_get_user_data(idcu_SdkContext* ctx) {
    if (!ctx) return NULL;
    return ctx->user_data;
}

void idcu_sdk_set_user_data(idcu_SdkContext* ctx, void* user_data) {
    if (!ctx) return;
    ctx->user_data = user_data;
}

uint32_t idcu_sdk_get_module_id(idcu_SdkContext* ctx) {
    if (!ctx) return 0;
    return ctx->module_id;
}

const char* idcu_sdk_get_module_name(idcu_SdkContext* ctx) {
    if (!ctx) return NULL;
    return ctx->module_name;
}

int idcu_sdk_send_message(idcu_SdkContext* ctx, const char* target_module, 
                          idcu_MsgPriority priority, const uint8_t* data, uint32_t size) {
    (void)target_module;
    
    if (!ctx || !g_msg_bus) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    if (data && size > 0) {
        return idcu_msg_send_zerocopy(g_msg_bus, ctx->module_id, 0, priority, data, size);
    } else {
        idcu_StackContext empty_ctx = {0};
        return idcu_msg_send(g_msg_bus, ctx->module_id, 0, priority, &empty_ctx);
    }
}

int idcu_sdk_broadcast_message(idcu_SdkContext* ctx, idcu_MsgPriority priority, 
                                const uint8_t* data, uint32_t size) {
    (void)data;
    (void)size;
    
    if (!ctx || !g_msg_bus) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    idcu_StackContext stack_ctx = {0};
    return idcu_msg_broadcast(g_msg_bus, ctx->module_id, priority, &stack_ctx);
}

int idcu_sdk_register_message_handler(idcu_SdkContext* ctx, idcu_SdkMessageHandler handler, void* user_data) {
    if (!ctx) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    ctx->msg_handler = handler;
    ctx->msg_handler_user_data = user_data;
    return IDCU_ERR_OK;
}

int idcu_sdk_log(idcu_SdkContext* ctx, idcu_LogLevel level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    
    const char* module_name = ctx ? ctx->module_name : "unknown";
    idcu_log_printf(level, __FILE__, __LINE__, "[%s] %s", module_name, buffer);
    
    va_end(args);
    return IDCU_ERR_OK;
}

void idcu_sdk_set_message_bus(idcu_MessageBus* bus) {
    g_msg_bus = bus;
}

void idcu_sdk_set_module_info(idcu_SdkContext* ctx, uint32_t module_id, const char* module_name) {
    if (!ctx) return;
    ctx->module_id = module_id;
    if (module_name) {
        strncpy(ctx->module_name, module_name, IDCU_MODULE_NAME_MAX - 1);
        ctx->module_name[IDCU_MODULE_NAME_MAX - 1] = '\0';
    }
}
