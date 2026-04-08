#include "module_error.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct idcu_module_error_context {
    idcu_ModuleError last_error;
    idcu_ModuleErrorHandler handler;
    void *handler_user_data;
    void *user_data;
} idcu_ModuleErrorContext;

static const char *error_messages[] = {
    [IDCU_MOD_ERR_INVALID_STATE - IDCU_MOD_ERR_BASE] = "Invalid module state",
    [IDCU_MOD_ERR_DEPENDENCY_MISSING - IDCU_MOD_ERR_BASE] = "Dependency missing",
    [IDCU_MOD_ERR_DEPENDENCY_VERSION_MISMATCH - IDCU_MOD_ERR_BASE] = "Dependency version mismatch",
    [IDCU_MOD_ERR_DEPENDENCY_CYCLE - IDCU_MOD_ERR_BASE] = "Dependency cycle detected",
    [IDCU_MOD_ERR_RESOURCE_EXHAUSTED - IDCU_MOD_ERR_BASE] = "Resource exhausted",
    [IDCU_MOD_ERR_RESOURCE_LIMIT_EXCEEDED - IDCU_MOD_ERR_BASE] = "Resource limit exceeded",
    [IDCU_MOD_ERR_INITIALIZATION_FAILED - IDCU_MOD_ERR_BASE] = "Module initialization failed",
    [IDCU_MOD_ERR_START_FAILED - IDCU_MOD_ERR_BASE] = "Module start failed",
    [IDCU_MOD_ERR_STOP_FAILED - IDCU_MOD_ERR_BASE] = "Module stop failed",
    [IDCU_MOD_ERR_COMMUNICATION_FAILED - IDCU_MOD_ERR_BASE] = "Communication failed",
    [IDCU_MOD_ERR_TIMEOUT - IDCU_MOD_ERR_BASE] = "Operation timed out",
    [IDCU_MOD_ERR_PERMISSION_DENIED - IDCU_MOD_ERR_BASE] = "Permission denied",
    [IDCU_MOD_ERR_INVALID_CONFIG - IDCU_MOD_ERR_BASE] = "Invalid configuration",
    [IDCU_MOD_ERR_MODULE_NOT_FOUND - IDCU_MOD_ERR_BASE] = "Module not found",
    [IDCU_MOD_ERR_MODULE_ALREADY_LOADED - IDCU_MOD_ERR_BASE] = "Module already loaded",
    [IDCU_MOD_ERR_SANDBOX_VIOLATION - IDCU_MOD_ERR_BASE] = "Sandbox violation"};

idcu_ModuleErrorContext *idcu_module_error_create_context(void) {
    idcu_ModuleErrorContext *ctx =
        (idcu_ModuleErrorContext *)malloc(sizeof(idcu_ModuleErrorContext));
    if (!ctx) {
        return NULL;
    }

    memset(ctx, 0, sizeof(idcu_ModuleErrorContext));
    ctx->last_error.error_code = IDCU_ERR_OK;
    ctx->handler = NULL;
    ctx->handler_user_data = NULL;
    ctx->user_data = NULL;

    return ctx;
}

void idcu_module_error_destroy_context(idcu_ModuleErrorContext *ctx) {
    if (ctx) {
        free(ctx);
    }
}

void *idcu_module_error_get_user_data(idcu_ModuleErrorContext *ctx) {
    if (!ctx)
        return NULL;
    return ctx->user_data;
}

void idcu_module_error_set_user_data(idcu_ModuleErrorContext *ctx, void *user_data) {
    if (!ctx)
        return;
    ctx->user_data = user_data;
}

void idcu_module_error_set_handler(idcu_ModuleErrorContext *ctx, idcu_ModuleErrorHandler handler,
                                   void *user_data) {
    if (!ctx)
        return;
    ctx->handler = handler;
    ctx->handler_user_data = user_data;
}

int idcu_module_error_raise(idcu_ModuleErrorContext *ctx, int error_code, const char *error_message,
                            const char *module_name, const char *file, int line) {
    return idcu_module_error_raise_with_data(ctx, error_code, error_message, module_name, file,
                                             line, NULL);
}

int idcu_module_error_raise_with_data(idcu_ModuleErrorContext *ctx, int error_code,
                                      const char *error_message, const char *module_name,
                                      const char *file, int line, void *user_data) {
    if (!ctx)
        return IDCU_ERR_INVALID_PARAM;

    ctx->last_error.error_code = error_code;
    ctx->last_error.error_message = error_message;
    ctx->last_error.module_name = module_name;
    ctx->last_error.file = file;
    ctx->last_error.line = line;
    ctx->last_error.timestamp = (int64_t)time(NULL) * 1000;
    ctx->last_error.user_data = user_data;

    if (ctx->handler) {
        ctx->handler(ctx, &ctx->last_error, ctx->handler_user_data);
    }

    return error_code;
}

const char *idcu_module_error_get_message(int error_code) {
    if (error_code < IDCU_MOD_ERR_BASE || error_code > IDCU_MOD_ERR_LAST) {
        return "Unknown error";
    }

    int index = error_code - IDCU_MOD_ERR_BASE;
    if (index < 0 || index >= (int)(sizeof(error_messages) / sizeof(error_messages[0]))) {
        return "Unknown error";
    }

    return error_messages[index];
}

void idcu_module_error_clear(idcu_ModuleErrorContext *ctx) {
    if (!ctx)
        return;
    ctx->last_error.error_code = IDCU_ERR_OK;
    ctx->last_error.error_message = NULL;
    ctx->last_error.module_name = NULL;
    ctx->last_error.file = NULL;
    ctx->last_error.line = 0;
    ctx->last_error.timestamp = 0;
    ctx->last_error.user_data = NULL;
}

int idcu_module_error_get_last(idcu_ModuleErrorContext *ctx, idcu_ModuleError *out_error) {
    if (!ctx || !out_error)
        return IDCU_ERR_INVALID_PARAM;

    memcpy(out_error, &ctx->last_error, sizeof(idcu_ModuleError));
    return IDCU_ERR_OK;
}
