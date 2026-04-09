#ifndef IDCU_SDK_SDK_H
#define IDCU_SDK_SDK_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/msgbus/types.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_sdk_init(idcu_SdkContext** ctx);
void idcu_sdk_destroy(idcu_SdkContext* ctx);

int idcu_sdk_register_module(idcu_SdkContext* ctx, const idcu_SdkModuleDef* def);

void idcu_sdk_log_debug(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_info(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_warn(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_error(idcu_SdkContext* ctx, const char* fmt, ...);

int idcu_sdk_get_config_string(idcu_SdkContext* ctx, const char* key, const char** out_value);
int idcu_sdk_get_config_int(idcu_SdkContext* ctx, const char* key, int* out_value);
int idcu_sdk_get_config_bool(idcu_SdkContext* ctx, const char* key, bool* out_value);

int idcu_sdk_publish_message(idcu_SdkContext* ctx,
                              idcu_MsgTopic topic,
                              const void* data,
                              size_t data_size,
                              idcu_MsgPriority priority);

int idcu_sdk_subscribe_message(idcu_SdkContext* ctx,
                                idcu_MsgTopic topic,
                                idcu_MsgHandler handler,
                                void* user_data,
                                idcu_MsgSubscriber** subscriber);

#define IDCU_SDK_MODULE_DEFINE(name_, ver_, desc_, init_fn_, start_fn_, stop_fn_, destroy_fn_, user_data_) \
    static const idcu_SdkModuleDef _idcu_sdk_module_##name_ = { \
        .name = #name_, \
        .version = ver_, \
        .description = desc_, \
        .init_fn = init_fn_, \
        .start_fn = start_fn_, \
        .stop_fn = stop_fn_, \
        .destroy_fn = destroy_fn_, \
        .user_data = user_data_ \
    }

#define IDCU_SDK_GET_MODULE(name_) (&_idcu_sdk_module_##name_)

#ifdef __cplusplus
}
#endif

#endif
