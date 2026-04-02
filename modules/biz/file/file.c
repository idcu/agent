#include "file.h"
#include <stdio.h>

static int file_init(void) {
    printf("[file] 文件操作模块加载完成\n");
    return 0;
}

static int file_exec(Context *ctx, void *arg) {
    const char *path = (const char *)arg;
    snprintf(ctx->output, CONTEXT_BUF, "[file] 处理文件: %s", path ? path : "unknown");
    printf("%s\n", ctx->output);
    return 0;
}

static Module file_mod = {
    .name = "file",
    .init = file_init,
    .exec = file_exec,
    .handle = NULL
};

Module *file_module_get(void) {
    return &file_mod;
}
