#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int json_init(void) {
    printf("[json] 内置JSON解析器加载完成\n");
    return 0;
}

static int json_exec(Context *ctx, void *arg) {
    const char *input = (const char *)arg;
    snprintf(ctx->output, CONTEXT_BUF, "[json] 解析内容: %s", input ? input : "null");
    printf("%s\n", ctx->output);
    return 0;
}

static Module json_mod = {
    .name = "json",
    .init = json_init,
    .exec = json_exec,
    .handle = NULL
};

Module *json_module_get(void) {
    return &json_mod;
}
