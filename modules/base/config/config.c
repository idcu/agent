#include "config.h"
#include <stdio.h>

static int config_init(void) {
    printf("[config] 配置模块加载完成\n");
    return 0;
}

static int config_exec(Context *ctx, void *arg) {
    const char *path = (const char *)arg;
    snprintf(ctx->output, CONTEXT_BUF, "[config] 读取配置: %s", path ? path : "modules.conf");
    printf("%s\n", ctx->output);
    return 0;
}

static Module config_mod = {
    .name = "config",
    .init = config_init,
    .exec = config_exec,
    .handle = NULL
};

Module *config_module_get(void) {
    return &config_mod;
}
