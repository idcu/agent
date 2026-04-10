#include <idcu/server-monitor/server-monitor.h>
#include <stdlib.h>
#include <string.h>

int idcu_server-monitor_init(idcu_Server-Monitor_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    *ctx = (idcu_Server-Monitor_Context*)calloc(1, sizeof(idcu_Server-Monitor_Context));
    if (!*ctx) {
        return IDCU_ERR_MEMORY;
    }
    
    idcu_mutex_init(&(*ctx)->lock);
    (*ctx)->initialized = 1;
    (*ctx)->operation_count = 0;
    (*ctx)->error_count = 0;
    
    return IDCU_ERR_OK;
}

void idcu_server-monitor_destroy(idcu_Server-Monitor_Context* ctx) {
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

int idcu_server-monitor_is_initialized(idcu_Server-Monitor_Context* ctx) {
    if (!ctx) return 0;
    return ctx->initialized;
}

uint64_t idcu_server-monitor_get_operation_count(idcu_Server-Monitor_Context* ctx) {
    if (!ctx) return 0;
    return ctx->operation_count;
}

uint64_t idcu_server-monitor_get_error_count(idcu_Server-Monitor_Context* ctx) {
    if (!ctx) return 0;
    return ctx->error_count;
}
