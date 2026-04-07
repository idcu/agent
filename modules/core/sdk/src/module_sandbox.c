#include "module_sandbox.h"
#include <stdlib.h>
#include <string.h>

typedef struct idcu_module_sandbox_context {
    char module_name[IDCU_MODULE_NAME_MAX];
    idcu_SandboxResourceLimits limits;
    idcu_SandboxResourceLimits usage;
    idcu_SandboxViolationCallback violation_callback;
    void* violation_callback_user_data;
    void* user_data;
} idcu_ModuleSandboxContext;

idcu_ModuleSandboxContext* idcu_module_sandbox_create_context(void) {
    idcu_ModuleSandboxContext* ctx = (idcu_ModuleSandboxContext*)malloc(sizeof(idcu_ModuleSandboxContext));
    if (!ctx) {
        return NULL;
    }
    
    memset(ctx, 0, sizeof(idcu_ModuleSandboxContext));
    ctx->limits.memory_limit_bytes = UINT64_MAX;
    ctx->limits.cpu_limit_percent = 100;
    ctx->limits.fd_limit = 1024;
    ctx->limits.thread_limit = 256;
    ctx->limits.network_allowed = true;
    ctx->violation_callback = NULL;
    ctx->violation_callback_user_data = NULL;
    ctx->user_data = NULL;
    
    return ctx;
}

void idcu_module_sandbox_destroy_context(idcu_ModuleSandboxContext* ctx) {
    if (ctx) {
        free(ctx);
    }
}

void* idcu_module_sandbox_get_user_data(idcu_ModuleSandboxContext* ctx) {
    if (!ctx) return NULL;
    return ctx->user_data;
}

void idcu_module_sandbox_set_user_data(idcu_ModuleSandboxContext* ctx, void* user_data) {
    if (!ctx) return;
    ctx->user_data = user_data;
}

void idcu_module_sandbox_set_module_info(idcu_ModuleSandboxContext* ctx, const char* module_name) {
    if (!ctx) return;
    if (module_name) {
        strncpy(ctx->module_name, module_name, IDCU_MODULE_NAME_MAX - 1);
        ctx->module_name[IDCU_MODULE_NAME_MAX - 1] = '\0';
    }
}

int idcu_module_sandbox_set_limits(idcu_ModuleSandboxContext* ctx,
                                    const idcu_SandboxResourceLimits* limits) {
    if (!ctx || !limits) return IDCU_ERR_INVALID_PARAM;
    
    memcpy(&ctx->limits, limits, sizeof(idcu_SandboxResourceLimits));
    return IDCU_ERR_OK;
}

int idcu_module_sandbox_get_limits(idcu_ModuleSandboxContext* ctx,
                                    idcu_SandboxResourceLimits* out_limits) {
    if (!ctx || !out_limits) return IDCU_ERR_INVALID_PARAM;
    
    memcpy(out_limits, &ctx->limits, sizeof(idcu_SandboxResourceLimits));
    return IDCU_ERR_OK;
}

int idcu_module_sandbox_get_usage(idcu_ModuleSandboxContext* ctx,
                                   idcu_SandboxResourceLimits* out_usage) {
    if (!ctx || !out_usage) return IDCU_ERR_INVALID_PARAM;
    
    memcpy(out_usage, &ctx->usage, sizeof(idcu_SandboxResourceLimits));
    return IDCU_ERR_OK;
}

int idcu_module_sandbox_check_memory(idcu_ModuleSandboxContext* ctx, uint64_t requested_bytes) {
    if (!ctx) return IDCU_ERR_INVALID_PARAM;
    
    if (ctx->usage.memory_used_bytes + requested_bytes > ctx->limits.memory_limit_bytes) {
        if (ctx->violation_callback) {
            ctx->violation_callback(ctx, IDCU_SANDBOX_RESOURCE_MEMORY, 
                                    "Memory limit exceeded", ctx->violation_callback_user_data);
        }
        return IDCU_MOD_ERR_RESOURCE_LIMIT_EXCEEDED;
    }
    
    return IDCU_ERR_OK;
}

int idcu_module_sandbox_check_fd(idcu_ModuleSandboxContext* ctx) {
    if (!ctx) return IDCU_ERR_INVALID_PARAM;
    
    if (ctx->usage.fd_used >= ctx->limits.fd_limit) {
        if (ctx->violation_callback) {
            ctx->violation_callback(ctx, IDCU_SANDBOX_RESOURCE_FILE_DESCRIPTORS, 
                                    "File descriptor limit exceeded", ctx->violation_callback_user_data);
        }
        return IDCU_MOD_ERR_RESOURCE_LIMIT_EXCEEDED;
    }
    
    return IDCU_ERR_OK;
}

int idcu_module_sandbox_check_thread(idcu_ModuleSandboxContext* ctx) {
    if (!ctx) return IDCU_ERR_INVALID_PARAM;
    
    if (ctx->usage.thread_used >= ctx->limits.thread_limit) {
        if (ctx->violation_callback) {
            ctx->violation_callback(ctx, IDCU_SANDBOX_RESOURCE_THREADS, 
                                    "Thread limit exceeded", ctx->violation_callback_user_data);
        }
        return IDCU_MOD_ERR_RESOURCE_LIMIT_EXCEEDED;
    }
    
    return IDCU_ERR_OK;
}

int idcu_module_sandbox_check_network(idcu_ModuleSandboxContext* ctx) {
    if (!ctx) return IDCU_ERR_INVALID_PARAM;
    
    if (!ctx->limits.network_allowed) {
        if (ctx->violation_callback) {
            ctx->violation_callback(ctx, IDCU_SANDBOX_RESOURCE_NETWORK, 
                                    "Network access denied", ctx->violation_callback_user_data);
        }
        return IDCU_MOD_ERR_PERMISSION_DENIED;
    }
    
    return IDCU_ERR_OK;
}

void idcu_module_sandbox_set_violation_callback(idcu_ModuleSandboxContext* ctx,
                                                  idcu_SandboxViolationCallback callback,
                                                  void* user_data) {
    if (!ctx) return;
    ctx->violation_callback = callback;
    ctx->violation_callback_user_data = user_data;
}

void idcu_module_sandbox_track_memory_alloc(idcu_ModuleSandboxContext* ctx, uint64_t size) {
    if (!ctx) return;
    ctx->usage.memory_used_bytes += size;
}

void idcu_module_sandbox_track_memory_free(idcu_ModuleSandboxContext* ctx, uint64_t size) {
    if (!ctx) return;
    if (ctx->usage.memory_used_bytes >= size) {
        ctx->usage.memory_used_bytes -= size;
    } else {
        ctx->usage.memory_used_bytes = 0;
    }
}

void idcu_module_sandbox_track_fd_alloc(idcu_ModuleSandboxContext* ctx) {
    if (!ctx) return;
    ctx->usage.fd_used++;
}

void idcu_module_sandbox_track_fd_free(idcu_ModuleSandboxContext* ctx) {
    if (!ctx) return;
    if (ctx->usage.fd_used > 0) {
        ctx->usage.fd_used--;
    }
}

void idcu_module_sandbox_track_thread_create(idcu_ModuleSandboxContext* ctx) {
    if (!ctx) return;
    ctx->usage.thread_used++;
}

void idcu_module_sandbox_track_thread_destroy(idcu_ModuleSandboxContext* ctx) {
    if (!ctx) return;
    if (ctx->usage.thread_used > 0) {
        ctx->usage.thread_used--;
    }
}

void idcu_module_sandbox_reset_usage(idcu_ModuleSandboxContext* ctx) {
    if (!ctx) return;
    memset(&ctx->usage, 0, sizeof(ctx->usage));
}
