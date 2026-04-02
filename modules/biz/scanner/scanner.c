#include "scanner.h"
#include <stdio.h>

static int scanner_init(void) {
    printf("[scanner] 扫描模块加载完成\n");
    return 0;
}

static int scanner_exec(Context *ctx, void *arg) {
    const char *path = (const char *)arg;
    snprintf(ctx->output, CONTEXT_BUF, "[scanner] 扫描目录: %s", path ? path : ".");
    printf("%s\n", ctx->output);
    return 0;
}

static Module scanner_mod = {
    .name = "scanner",
    .init = scanner_init,
    .exec = scanner_exec,
    .handle = NULL
};

Module *scanner_module_get(void) {
    return &scanner_mod;
}
