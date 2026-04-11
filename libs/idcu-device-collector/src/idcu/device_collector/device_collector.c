#include "idcu/device_collector/device_collector.h"
#include <stdlib.h>
#include <string.h>

int idcu_device_collector_init(idcu_DeviceCollector_Context** ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    *ctx = (idcu_DeviceCollector_Context*)calloc(1, sizeof(idcu_DeviceCollector_Context));
    if (!*ctx) {
        return IDCU_ERR_MEMORY;
    }

    idcu_mutex_init(&(*ctx)->lock);
    (*ctx)->initialized = 1;
    (*ctx)->operation_count = 0;
    (*ctx)->error_count = 0;

    return IDCU_ERR_OK;
}

int idcu_device_collector_start(idcu_DeviceCollector_Context* ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

void idcu_device_collector_stop(idcu_DeviceCollector_Context* ctx)
{
    if (!ctx) {
        return;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        return;
    }

    idcu_mutex_unlock(&ctx->lock);
}

void idcu_device_collector_destroy(idcu_DeviceCollector_Context* ctx)
{
    if (!ctx) {
        return;
    }

    idcu_mutex_lock(&ctx->lock);
    idcu_Mutex lock_copy = ctx->lock;
    ctx->initialized = 0;
    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);

    free(ctx);
}

int idcu_device_collector_scan(idcu_DeviceCollector_Context* ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_device_collector_get_device_count(idcu_DeviceCollector_Context* ctx, size_t* count)
{
    if (!ctx || !count) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    *count = 0;
    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_device_collector_get_device_info(idcu_DeviceCollector_Context* ctx, size_t index, idcu_DeviceInfo* info)
{
    (void)index;

    if (!ctx || !info) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    memset(info, 0, sizeof(idcu_DeviceInfo));
    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_device_collector_get_device_by_type(idcu_DeviceCollector_Context* ctx, idcu_DeviceType type, idcu_DeviceInfo* info_list, size_t max_count, size_t* actual_count)
{
    (void)type;
    (void)info_list;
    (void)max_count;

    if (!ctx || !actual_count) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    *actual_count = 0;
    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}
