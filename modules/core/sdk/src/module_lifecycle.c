#include "module_lifecycle.h"
#include <stdlib.h>
#include <string.h>

typedef struct idcu_module_lifecycle_context {
    idcu_ModuleLifecycleConfig config;
    idcu_ModuleState state;
    void *user_data;
} idcu_ModuleLifecycleContext;

idcu_ModuleLifecycleContext *idcu_module_lifecycle_create_context(void) {
    idcu_ModuleLifecycleContext *ctx =
        (idcu_ModuleLifecycleContext *)malloc(sizeof(idcu_ModuleLifecycleContext));
    if (!ctx) {
        return NULL;
    }

    memset(ctx, 0, sizeof(idcu_ModuleLifecycleContext));
    ctx->state = IDCU_MOD_STATE_UNINIT;
    ctx->user_data = NULL;

    return ctx;
}

void idcu_module_lifecycle_destroy_context(idcu_ModuleLifecycleContext *ctx) {
    if (ctx) {
        free(ctx);
    }
}

void *idcu_module_lifecycle_get_user_data(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return NULL;
    return ctx->user_data;
}

void idcu_module_lifecycle_set_user_data(idcu_ModuleLifecycleContext *ctx, void *user_data) {
    if (!ctx)
        return;
    ctx->user_data = user_data;
}

void idcu_module_lifecycle_set_config(idcu_ModuleLifecycleContext *ctx,
                                      const idcu_ModuleLifecycleConfig *config) {
    if (!ctx || !config)
        return;
    memcpy(&ctx->config, config, sizeof(idcu_ModuleLifecycleConfig));
}

int idcu_module_lifecycle_pre_init(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return IDCU_ERR_INVALID_PARAM;
    if (ctx->config.callbacks.on_pre_init) {
        return ctx->config.callbacks.on_pre_init(ctx);
    }
    return IDCU_ERR_OK;
}

int idcu_module_lifecycle_init(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return IDCU_ERR_INVALID_PARAM;
    if (ctx->config.callbacks.on_init) {
        int ret = ctx->config.callbacks.on_init(ctx);
        if (ret == IDCU_ERR_OK) {
            ctx->state = IDCU_MOD_STATE_INITED;
        }
        return ret;
    }
    ctx->state = IDCU_MOD_STATE_INITED;
    return IDCU_ERR_OK;
}

int idcu_module_lifecycle_post_init(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return IDCU_ERR_INVALID_PARAM;
    if (ctx->config.callbacks.on_post_init) {
        return ctx->config.callbacks.on_post_init(ctx);
    }
    return IDCU_ERR_OK;
}

int idcu_module_lifecycle_pre_start(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return IDCU_ERR_INVALID_PARAM;
    if (ctx->config.callbacks.on_pre_start) {
        return ctx->config.callbacks.on_pre_start(ctx);
    }
    return IDCU_ERR_OK;
}

int idcu_module_lifecycle_start(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return IDCU_ERR_INVALID_PARAM;
    if (ctx->config.callbacks.on_start) {
        int ret = ctx->config.callbacks.on_start(ctx);
        if (ret == IDCU_ERR_OK) {
            ctx->state = IDCU_MOD_STATE_RUNNING;
        }
        return ret;
    }
    ctx->state = IDCU_MOD_STATE_RUNNING;
    return IDCU_ERR_OK;
}

int idcu_module_lifecycle_post_start(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return IDCU_ERR_INVALID_PARAM;
    if (ctx->config.callbacks.on_post_start) {
        return ctx->config.callbacks.on_post_start(ctx);
    }
    return IDCU_ERR_OK;
}

int idcu_module_lifecycle_pre_stop(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return IDCU_ERR_INVALID_PARAM;
    if (ctx->config.callbacks.on_pre_stop) {
        return ctx->config.callbacks.on_pre_stop(ctx);
    }
    return IDCU_ERR_OK;
}

int idcu_module_lifecycle_stop(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return IDCU_ERR_INVALID_PARAM;
    if (ctx->config.callbacks.on_stop) {
        int ret = ctx->config.callbacks.on_stop(ctx);
        if (ret == IDCU_ERR_OK) {
            ctx->state = IDCU_MOD_STATE_STOPPED;
        }
        return ret;
    }
    ctx->state = IDCU_MOD_STATE_STOPPED;
    return IDCU_ERR_OK;
}

int idcu_module_lifecycle_post_stop(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return IDCU_ERR_INVALID_PARAM;
    if (ctx->config.callbacks.on_post_stop) {
        return ctx->config.callbacks.on_post_stop(ctx);
    }
    return IDCU_ERR_OK;
}

void idcu_module_lifecycle_pre_destroy(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return;
    if (ctx->config.callbacks.on_pre_destroy) {
        ctx->config.callbacks.on_pre_destroy(ctx);
    }
}

void idcu_module_lifecycle_destroy(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return;
    if (ctx->config.callbacks.on_destroy) {
        ctx->config.callbacks.on_destroy(ctx);
    }
    ctx->state = IDCU_MOD_STATE_UNINIT;
}

void idcu_module_lifecycle_post_destroy(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return;
    if (ctx->config.callbacks.on_post_destroy) {
        ctx->config.callbacks.on_post_destroy(ctx);
    }
}

idcu_ModuleState idcu_module_lifecycle_get_state(idcu_ModuleLifecycleContext *ctx) {
    if (!ctx)
        return IDCU_MOD_STATE_ERROR;
    return ctx->state;
}

void idcu_module_lifecycle_set_state(idcu_ModuleLifecycleContext *ctx, idcu_ModuleState state) {
    if (!ctx)
        return;
    ctx->state = state;
}
