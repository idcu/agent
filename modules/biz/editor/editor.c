#include "editor.h"
#include <stdio.h>

static int editor_init(void) {
    printf("[editor] 编辑模块加载完成\n");
    return 0;
}

static int editor_exec(Context *ctx, void *arg) {
    const char *act = (const char *)arg;
    snprintf(ctx->output, CONTEXT_BUF, "[editor] 执行操作: %s", act ? act : "edit");
    printf("%s\n", ctx->output);
    return 0;
}

static Module editor_mod = {
    .name = "editor",
    .init = editor_init,
    .exec = editor_exec,
    .handle = NULL
};

Module *editor_module_get(void) {
    return &editor_mod;
}
