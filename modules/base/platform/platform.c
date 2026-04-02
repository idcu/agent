#include "platform.h"
#include <stdio.h>

static int platform_init(void) {
#ifdef _WIN32
    printf("[platform] 当前系统: Windows\n");
#else
    printf("[platform] 当前系统: 非Windows\n");
#endif
    return 0;
}

static int platform_exec(Context *ctx, void *arg) {
    snprintf(ctx->output, CONTEXT_BUF, "[platform] 运行正常");
    printf("%s\n", ctx->output);
    return 0;
}

static Module platform_mod = {
    .name = "platform",
    .init = platform_init,
    .exec = platform_exec,
    .handle = NULL
};

Module *platform_module_get(void) {
    return &platform_mod;
}
