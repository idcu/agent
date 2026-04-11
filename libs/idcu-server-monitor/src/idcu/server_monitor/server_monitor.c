#include "idcu/server_monitor/server_monitor.h"
#include <stdlib.h>
#include <string.h>

int idcu_server_monitor_init(idcu_ServerMonitor_Context** ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    *ctx = (idcu_ServerMonitor_Context*)calloc(1, sizeof(idcu_ServerMonitor_Context));
    if (!*ctx) {
        return IDCU_ERR_MEMORY;
    }

    idcu_mutex_init(&(*ctx)->lock);
    (*ctx)->initialized = 1;
    (*ctx)->operation_count = 0;
    (*ctx)->error_count = 0;

    return IDCU_ERR_OK;
}

int idcu_server_monitor_start(idcu_ServerMonitor_Context* ctx)
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

void idcu_server_monitor_stop(idcu_ServerMonitor_Context* ctx)
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

void idcu_server_monitor_destroy(idcu_ServerMonitor_Context* ctx)
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

int idcu_server_monitor_collect(idcu_ServerMonitor_Context* ctx)
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

int idcu_server_monitor_get_stats(idcu_ServerMonitor_Context* ctx, idcu_ServerStats* stats)
{
    if (!ctx || !stats) {
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

    memset(stats, 0, sizeof(idcu_ServerStats));
    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_server_monitor_get_cpu_usage(idcu_ServerMonitor_Context* ctx, double* usage)
{
    if (!ctx || !usage) {
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

    *usage = 0.0;
    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_server_monitor_get_memory_info(idcu_ServerMonitor_Context* ctx, uint64_t* total, uint64_t* used)
{
    if (!ctx) {
        ctx->error_count++;
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    if (total) {
        *total = 0;
    }
    if (used) {
        *used = 0;
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_server_monitor_get_disk_info(idcu_ServerMonitor_Context* ctx, uint64_t* total, uint64_t* used)
{
    if (!ctx) {
        ctx->error_count++;
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    if (total) {
        *total = 0;
    }
    if (used) {
        *used = 0;
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}
