#include "idcu/module_isolation/module_isolation.h"
#include <stdlib.h>
#include <string.h>

int idcu_module_isolation_init(idcu_ModuleIsolation_Context** ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    *ctx = (idcu_ModuleIsolation_Context*)calloc(1, sizeof(idcu_ModuleIsolation_Context));
    if (!*ctx) {
        return IDCU_ERR_MEMORY;
    }

    idcu_mutex_init(&(*ctx)->lock);
    (*ctx)->initialized = 1;
    (*ctx)->operation_count = 0;
    (*ctx)->error_count = 0;

    return IDCU_ERR_OK;
}

int idcu_module_isolation_create(idcu_ModuleIsolation_Context* ctx, const char* module_name, idcu_IsolationLevel level)
{
    (void)module_name;
    (void)level;

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

int idcu_module_isolation_destroy(idcu_ModuleIsolation_Context* ctx, const char* module_name)
{
    (void)module_name;

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

void idcu_module_isolation_cleanup(idcu_ModuleIsolation_Context* ctx)
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

int idcu_module_isolation_set_memory_limit(idcu_ModuleIsolation_Context* ctx, const char* module_name, uint64_t bytes)
{
    (void)module_name;
    (void)bytes;

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

int idcu_module_isolation_set_cpu_limit(idcu_ModuleIsolation_Context* ctx, const char* module_name, uint32_t percent)
{
    (void)module_name;
    (void)percent;

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

int idcu_module_isolation_get_status(idcu_ModuleIsolation_Context* ctx, const char* module_name, int* status)
{
    (void)module_name;

    if (!ctx || !status) {
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

    *status = 0;
    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}
