#include "idcu/scheduler/scheduler.h"
#include <stdlib.h>
#include <string.h>

int idcu_scheduler_init(idcu_Scheduler_Context** ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    *ctx = (idcu_Scheduler_Context*)calloc(1, sizeof(idcu_Scheduler_Context));
    if (!*ctx) {
        return IDCU_ERR_MEMORY;
    }

    idcu_mutex_init(&(*ctx)->lock);
    (*ctx)->initialized = 1;
    (*ctx)->operation_count = 0;
    (*ctx)->error_count = 0;

    return IDCU_ERR_OK;
}

int idcu_scheduler_start(idcu_Scheduler_Context* ctx)
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

void idcu_scheduler_stop(idcu_Scheduler_Context* ctx)
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

void idcu_scheduler_destroy(idcu_Scheduler_Context* ctx)
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

idcu_TaskId idcu_scheduler_add_once(idcu_Scheduler_Context* ctx, const char* name, uint64_t delay_ms, idcu_TaskFunc func, void* user_data)
{
    (void)name;
    (void)delay_ms;
    (void)func;
    (void)user_data;

    if (!ctx) {
        return 0;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return 0;
    }

    idcu_mutex_unlock(&ctx->lock);
    return 1;
}

idcu_TaskId idcu_scheduler_add_periodic(idcu_Scheduler_Context* ctx, const char* name, uint64_t interval_ms, idcu_TaskFunc func, void* user_data)
{
    (void)name;
    (void)interval_ms;
    (void)func;
    (void)user_data;

    if (!ctx) {
        return 0;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return 0;
    }

    idcu_mutex_unlock(&ctx->lock);
    return 2;
}

idcu_TaskId idcu_scheduler_add_cron(idcu_Scheduler_Context* ctx, const char* name, const char* cron_expr, idcu_TaskFunc func, void* user_data)
{
    (void)name;
    (void)cron_expr;
    (void)func;
    (void)user_data;

    if (!ctx) {
        return 0;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return 0;
    }

    idcu_mutex_unlock(&ctx->lock);
    return 3;
}

int idcu_scheduler_cancel_task(idcu_Scheduler_Context* ctx, idcu_TaskId id)
{
    (void)id;

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

int idcu_scheduler_get_task_status(idcu_Scheduler_Context* ctx, idcu_TaskId id, idcu_TaskStatus* status)
{
    (void)id;

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

    *status = IDCU_TASK_STATUS_PENDING;
    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}
