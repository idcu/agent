#ifndef IDCU_SDK_SDK_H
#define IDCU_SDK_SDK_H

#include "module_def.h"
#include "msg_bus.h"
#include "log.h"
#include "error_code.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_SDK_VERSION "1.0.0"

typedef struct idcu_sdk_context idcu_SdkContext;

typedef struct {
    const char* name;
    const char* version;
    const char* description;
    int (*on_init)(idcu_SdkContext* ctx);
    int (*on_start)(idcu_SdkContext* ctx);
    int (*on_stop)(idcu_SdkContext* ctx);
    void (*on_destroy)(idcu_SdkContext* ctx);
    const char** dependencies;
    int dependency_count;
} idcu_SdkModuleDef;

typedef struct {
    uint32_t source_id;
    const uint8_t* data;
    uint32_t size;
    idcu_MsgPriority priority;
} idcu_SdkMessage;

typedef void (*idcu_SdkMessageHandler)(idcu_SdkContext* ctx, const idcu_SdkMessage* msg, void* user_data);

idcu_SdkContext* idcu_sdk_create_context(void);
void idcu_sdk_destroy_context(idcu_SdkContext* ctx);

void* idcu_sdk_get_user_data(idcu_SdkContext* ctx);
void idcu_sdk_set_user_data(idcu_SdkContext* ctx, void* user_data);

void idcu_sdk_set_message_bus(idcu_MessageBus* bus);
void idcu_sdk_set_module_info(idcu_SdkContext* ctx, uint32_t module_id, const char* module_name);

uint32_t idcu_sdk_get_module_id(idcu_SdkContext* ctx);
const char* idcu_sdk_get_module_name(idcu_SdkContext* ctx);

int idcu_sdk_send_message(idcu_SdkContext* ctx, const char* target_module, 
                          idcu_MsgPriority priority, const uint8_t* data, uint32_t size);

int idcu_sdk_broadcast_message(idcu_SdkContext* ctx, idcu_MsgPriority priority, 
                                const uint8_t* data, uint32_t size);

int idcu_sdk_register_message_handler(idcu_SdkContext* ctx, idcu_SdkMessageHandler handler, void* user_data);

int idcu_sdk_log(idcu_SdkContext* ctx, idcu_LogLevel level, const char* fmt, ...);

#define idcu_sdk_log_debug(ctx, fmt, ...) \
    idcu_sdk_log(ctx, IDCU_LOG_DEBUG, fmt, ##__VA_ARGS__)

#define idcu_sdk_log_info(ctx, fmt, ...) \
    idcu_sdk_log(ctx, IDCU_LOG_INFO, fmt, ##__VA_ARGS__)

#define idcu_sdk_log_warn(ctx, fmt, ...) \
    idcu_sdk_log(ctx, IDCU_LOG_WARN, fmt, ##__VA_ARGS__)

#define idcu_sdk_log_error(ctx, fmt, ...) \
    idcu_sdk_log(ctx, IDCU_LOG_ERROR, fmt, ##__VA_ARGS__)

#define idcu_sdk_log_fatal(ctx, fmt, ...) \
    idcu_sdk_log(ctx, IDCU_LOG_FATAL, fmt, ##__VA_ARGS__)

#define IDCU_SDK_MODULE_DEFINE(module_name, module_version, module_desc, init_fn, start_fn, stop_fn, destroy_fn) \
    static idcu_SdkContext* g_sdk_ctx = NULL; \
    \
    static int sdk_wrapper_init(void) { \
        g_sdk_ctx = idcu_sdk_create_context(); \
        if (!g_sdk_ctx) return IDCU_ERR_NO_MEMORY; \
        if (init_fn) return init_fn(g_sdk_ctx); \
        return IDCU_ERR_OK; \
    } \
    \
    static int sdk_wrapper_start(void) { \
        if (start_fn && g_sdk_ctx) return start_fn(g_sdk_ctx); \
        return IDCU_ERR_OK; \
    } \
    \
    static int sdk_wrapper_stop(void) { \
        if (stop_fn && g_sdk_ctx) return stop_fn(g_sdk_ctx); \
        return IDCU_ERR_OK; \
    } \
    \
    static void sdk_wrapper_destroy(void) { \
        if (destroy_fn && g_sdk_ctx) destroy_fn(g_sdk_ctx); \
        if (g_sdk_ctx) { \
            idcu_sdk_destroy_context(g_sdk_ctx); \
            g_sdk_ctx = NULL; \
        } \
    } \
    \
    IDCU_REGISTER_MODULE(module_name, IDCU_MODULE_VERSION(1, 0, 0), sdk_wrapper_init, sdk_wrapper_start, sdk_wrapper_stop)

#ifdef __cplusplus
}
#endif

#endif
