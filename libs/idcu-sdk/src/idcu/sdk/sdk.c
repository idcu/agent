#include <idcu/sdk/sdk.h>
#include <idcu/microkernel/kernel.h>
#include <idcu/msgbus/msgbus.h>
#include <idcu/common/vector.h>
#include <idcu/common/lock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

struct idcu_SdkContext {
    idcu_MicroKernel* kernel;
    idcu_Vector* modules;
    idcu_Mutex mutex;
    void* user_data;
};

static void sdk_log_vprintf(idcu_SdkContext* ctx, const char* level, const char* fmt, va_list args) {
    (void)ctx;
    (void)level;
    vprintf(fmt, args);
    printf("\n");
}

int idcu_sdk_init(idcu_SdkContext** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_SdkContext* c = (idcu_SdkContext*)malloc(sizeof(idcu_SdkContext));
    if (!c) {
        return IDCU_ERR_MEMORY;
    }

    memset(c, 0, sizeof(idcu_SdkContext));

    int ret = idcu_mutex_init(&c->mutex);
    if (ret != IDCU_ERR_OK) {
        free(c);
        return ret;
    }

    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = false,
        .log_level = 0,
        .max_modules = 32
    };

    ret = idcu_kernel_init(&c->kernel, &config);
    if (ret != IDCU_ERR_OK) {
        idcu_mutex_destroy(&c->mutex);
        free(c);
        return ret;
    }

    c->modules = (idcu_Vector*)malloc(sizeof(idcu_Vector));
    if (!c->modules) {
        idcu_kernel_destroy(c->kernel);
        idcu_mutex_destroy(&c->mutex);
        free(c);
        return IDCU_ERR_MEMORY;
    }

    ret = idcu_vector_init(c->modules, sizeof(idcu_SdkModuleDef*), 8);
    if (ret != IDCU_ERR_OK) {
        free(c->modules);
        idcu_kernel_destroy(c->kernel);
        idcu_mutex_destroy(&c->mutex);
        free(c);
        return ret;
    }

    *ctx = c;
    return IDCU_ERR_OK;
}

void idcu_sdk_destroy(idcu_SdkContext* ctx) {
    if (!ctx) {
        return;
    }

    idcu_mutex_lock(&ctx->mutex);

    if (ctx->modules) {
        idcu_vector_destroy(ctx->modules);
        free(ctx->modules);
    }

    if (ctx->kernel) {
        idcu_kernel_destroy(ctx->kernel);
    }

    idcu_mutex_unlock(&ctx->mutex);
    idcu_mutex_destroy(&ctx->mutex);

    free(ctx);
}

int idcu_sdk_register_module(idcu_SdkContext* ctx, const idcu_SdkModuleDef* def) {
    if (!ctx || !def) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->mutex);
    idcu_vector_push_back(ctx->modules, &def);
    idcu_mutex_unlock(&ctx->mutex);

    return IDCU_ERR_OK;
}

void idcu_sdk_log_debug(idcu_SdkContext* ctx, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    sdk_log_vprintf(ctx, "DEBUG", fmt, args);
    va_end(args);
}

void idcu_sdk_log_info(idcu_SdkContext* ctx, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    sdk_log_vprintf(ctx, "INFO", fmt, args);
    va_end(args);
}

void idcu_sdk_log_warn(idcu_SdkContext* ctx, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    sdk_log_vprintf(ctx, "WARN", fmt, args);
    va_end(args);
}

void idcu_sdk_log_error(idcu_SdkContext* ctx, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    sdk_log_vprintf(ctx, "ERROR", fmt, args);
    va_end(args);
}

int idcu_sdk_get_config_string(idcu_SdkContext* ctx, const char* key, const char** out_value) {
    (void)ctx;
    (void)key;
    if (!out_value) {
        return IDCU_ERR_INVALID_ARG;
    }
    *out_value = NULL;
    return IDCU_ERR_NOT_FOUND;
}

int idcu_sdk_get_config_int(idcu_SdkContext* ctx, const char* key, int* out_value) {
    (void)ctx;
    (void)key;
    if (!out_value) {
        return IDCU_ERR_INVALID_ARG;
    }
    *out_value = 0;
    return IDCU_ERR_NOT_FOUND;
}

int idcu_sdk_get_config_bool(idcu_SdkContext* ctx, const char* key, bool* out_value) {
    (void)ctx;
    (void)key;
    if (!out_value) {
        return IDCU_ERR_INVALID_ARG;
    }
    *out_value = false;
    return IDCU_ERR_NOT_FOUND;
}

int idcu_sdk_publish_message(idcu_SdkContext* ctx,
                              idcu_MsgTopic topic,
                              const void* data,
                              size_t data_size,
                              idcu_MsgPriority priority) {
    if (!ctx || !ctx->kernel) {
        return IDCU_ERR_INVALID_ARG;
    }
    return idcu_msgbus_publish(NULL, topic, data, data_size, priority);
}

int idcu_sdk_subscribe_message(idcu_SdkContext* ctx,
                                idcu_MsgTopic topic,
                                idcu_MsgHandler handler,
                                void* user_data,
                                idcu_MsgSubscriber** subscriber) {
    if (!ctx || !ctx->kernel || !handler || !subscriber) {
        return IDCU_ERR_INVALID_ARG;
    }
    return idcu_msgbus_subscribe(NULL, topic, handler, user_data, subscriber);
}

int idcu_sdk_set_user_data(idcu_SdkContext* ctx, void* user_data) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&ctx->mutex);
    ctx->user_data = user_data;
    idcu_mutex_unlock(&ctx->mutex);
    return IDCU_ERR_OK;
}

void* idcu_sdk_get_user_data(idcu_SdkContext* ctx) {
    if (!ctx) {
        return NULL;
    }
    idcu_mutex_lock(&ctx->mutex);
    void* data = ctx->user_data;
    idcu_mutex_unlock(&ctx->mutex);
    return data;
}
