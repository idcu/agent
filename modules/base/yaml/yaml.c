#include "yaml.h"
#include <stdio.h>

static int yaml_init(void) {
    printf("[yaml] 内置YAML解析器加载完成\n");
    return 0;
}

static int yaml_exec(Context *ctx, void *arg) {
    const char *input = (const char *)arg;
    snprintf(ctx->output, CONTEXT_BUF, "[yaml] 解析内容: %s", input ? input : "null");
    printf("%s\n", ctx->output);
    return 0;
}

static Module yaml_mod = {
    .name = "yaml",
    .init = yaml_init,
    .exec = yaml_exec,
    .handle = NULL
};

Module *yaml_module_get(void) {
    return &yaml_mod;
}
