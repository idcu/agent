#include "module_communication.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PENDING_REQUESTS 32
#define MAX_TOPIC_HANDLERS 16

typedef struct {
    uint64_t request_id;
    idcu_ModuleResponseCallback callback;
    void* user_data;
    int64_t timestamp;
} idcu_PendingRequest;

typedef struct {
    char topic[IDCU_MSG_TOPIC_MAX];
    idcu_ModuleMessageHandler handler;
    void* user_data;
} idcu_TopicHandler;

typedef struct idcu_module_comm_context {
    idcu_MessageBus* msg_bus;
    uint32_t module_id;
    char module_name[IDCU_MODULE_NAME_MAX];
    void* user_data;
    idcu_PendingRequest pending_requests[MAX_PENDING_REQUESTS];
    int pending_request_count;
    idcu_TopicHandler topic_handlers[MAX_TOPIC_HANDLERS];
    int topic_handler_count;
    uint64_t next_request_id;
} idcu_ModuleCommContext;

static int64_t get_current_timestamp(void) {
    return (int64_t)time(NULL) * 1000;
}

idcu_ModuleCommContext* idcu_module_comm_create_context(void) {
    idcu_ModuleCommContext* ctx = (idcu_ModuleCommContext*)malloc(sizeof(idcu_ModuleCommContext));
    if (!ctx) {
        return NULL;
    }
    
    memset(ctx, 0, sizeof(idcu_ModuleCommContext));
    ctx->msg_bus = NULL;
    ctx->module_id = 0;
    ctx->user_data = NULL;
    ctx->pending_request_count = 0;
    ctx->topic_handler_count = 0;
    ctx->next_request_id = 1;
    
    return ctx;
}

void idcu_module_comm_destroy_context(idcu_ModuleCommContext* ctx) {
    if (ctx) {
        free(ctx);
    }
}

void* idcu_module_comm_get_user_data(idcu_ModuleCommContext* ctx) {
    if (!ctx) return NULL;
    return ctx->user_data;
}

void idcu_module_comm_set_user_data(idcu_ModuleCommContext* ctx, void* user_data) {
    if (!ctx) return;
    ctx->user_data = user_data;
}

void idcu_module_comm_set_message_bus(idcu_ModuleCommContext* ctx, idcu_MessageBus* bus) {
    if (!ctx) return;
    ctx->msg_bus = bus;
}

void idcu_module_comm_set_module_info(idcu_ModuleCommContext* ctx, uint32_t module_id, const char* module_name) {
    if (!ctx) return;
    ctx->module_id = module_id;
    if (module_name) {
        strncpy(ctx->module_name, module_name, IDCU_MODULE_NAME_MAX - 1);
        ctx->module_name[IDCU_MODULE_NAME_MAX - 1] = '\0';
    }
}

uint32_t idcu_module_comm_get_module_id(idcu_ModuleCommContext* ctx) {
    if (!ctx) return 0;
    return ctx->module_id;
}

const char* idcu_module_comm_get_module_name(idcu_ModuleCommContext* ctx) {
    if (!ctx) return NULL;
    return ctx->module_name;
}

int idcu_module_comm_send_request(idcu_ModuleCommContext* ctx,
                                  const char* target_module,
                                  const char* topic,
                                  idcu_MsgPriority priority,
                                  const uint8_t* payload,
                                  uint32_t payload_size,
                                  idcu_ModuleResponseCallback callback,
                                  void* callback_user_data,
                                  uint64_t* out_request_id) {
    if (!ctx || !ctx->msg_bus) return IDCU_ERR_NOT_INITIALIZED;
    
    if (ctx->pending_request_count >= MAX_PENDING_REQUESTS) {
        return IDCU_ERR_NO_RESOURCE;
    }
    
    uint64_t request_id = ctx->next_request_id++;
    
    if (out_request_id) {
        *out_request_id = request_id;
    }
    
    if (callback) {
        ctx->pending_requests[ctx->pending_request_count].request_id = request_id;
        ctx->pending_requests[ctx->pending_request_count].callback = callback;
        ctx->pending_requests[ctx->pending_request_count].user_data = callback_user_data;
        ctx->pending_requests[ctx->pending_request_count].timestamp = get_current_timestamp();
        ctx->pending_request_count++;
    }
    
    idcu_StackContext stack_ctx = {0};
    return idcu_msg_send(ctx->msg_bus, ctx->module_id, 0, priority, &stack_ctx);
}

int idcu_module_comm_send_response(idcu_ModuleCommContext* ctx,
                                   const idcu_ModuleMessage* request,
                                   int error_code,
                                   const uint8_t* payload,
                                   uint32_t payload_size) {
    if (!ctx || !ctx->msg_bus || !request) return IDCU_ERR_INVALID_PARAM;
    
    idcu_StackContext stack_ctx = {0};
    return idcu_msg_send(ctx->msg_bus, ctx->module_id, request->source_module_id, request->priority, &stack_ctx);
}

int idcu_module_comm_send_notification(idcu_ModuleCommContext* ctx,
                                       const char* target_module,
                                       const char* topic,
                                       idcu_MsgPriority priority,
                                       const uint8_t* payload,
                                       uint32_t payload_size) {
    if (!ctx || !ctx->msg_bus) return IDCU_ERR_NOT_INITIALIZED;
    
    idcu_StackContext stack_ctx = {0};
    return idcu_msg_send(ctx->msg_bus, ctx->module_id, 0, priority, &stack_ctx);
}

int idcu_module_comm_broadcast_event(idcu_ModuleCommContext* ctx,
                                     const char* topic,
                                     idcu_MsgPriority priority,
                                     const uint8_t* payload,
                                     uint32_t payload_size) {
    if (!ctx || !ctx->msg_bus) return IDCU_ERR_NOT_INITIALIZED;
    
    idcu_StackContext stack_ctx = {0};
    return idcu_msg_broadcast(ctx->msg_bus, ctx->module_id, priority, &stack_ctx);
}

int idcu_module_comm_register_topic_handler(idcu_ModuleCommContext* ctx,
                                            const char* topic,
                                            idcu_ModuleMessageHandler handler,
                                            void* user_data) {
    if (!ctx || !topic || !handler) return IDCU_ERR_INVALID_PARAM;
    
    if (ctx->topic_handler_count >= MAX_TOPIC_HANDLERS) {
        return IDCU_ERR_NO_RESOURCE;
    }
    
    strncpy(ctx->topic_handlers[ctx->topic_handler_count].topic, topic, IDCU_MSG_TOPIC_MAX - 1);
    ctx->topic_handlers[ctx->topic_handler_count].topic[IDCU_MSG_TOPIC_MAX - 1] = '\0';
    ctx->topic_handlers[ctx->topic_handler_count].handler = handler;
    ctx->topic_handlers[ctx->topic_handler_count].user_data = user_data;
    ctx->topic_handler_count++;
    
    return IDCU_ERR_OK;
}

int idcu_module_comm_unregister_topic_handler(idcu_ModuleCommContext* ctx, const char* topic) {
    if (!ctx || !topic) return IDCU_ERR_INVALID_PARAM;
    
    for (int i = 0; i < ctx->topic_handler_count; i++) {
        if (strcmp(ctx->topic_handlers[i].topic, topic) == 0) {
            for (int j = i; j < ctx->topic_handler_count - 1; j++) {
                ctx->topic_handlers[j] = ctx->topic_handlers[j + 1];
            }
            ctx->topic_handler_count--;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NOT_FOUND;
}

int idcu_module_comm_cancel_request(idcu_ModuleCommContext* ctx, uint64_t request_id) {
    if (!ctx) return IDCU_ERR_INVALID_PARAM;
    
    for (int i = 0; i < ctx->pending_request_count; i++) {
        if (ctx->pending_requests[i].request_id == request_id) {
            for (int j = i; j < ctx->pending_request_count - 1; j++) {
                ctx->pending_requests[j] = ctx->pending_requests[j + 1];
            }
            ctx->pending_request_count--;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NOT_FOUND;
}

void idcu_module_message_free(idcu_ModuleMessage* msg) {
    if (msg) {
        free((void*)msg);
    }
}
