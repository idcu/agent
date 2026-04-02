#include "core.h"
#include "module.h"
#include "context.h"
#include <stdio.h>
#include <string.h>

static Context g_ctx;

void core_init(void) {
    context_init(&g_ctx, "");
    printf("[core] 初始化成功\n");
}

int core_load_modules(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == 0)
            continue;

        char name[64] = {0};
        char getter[64] = {0};
        if (sscanf(line, "%63s %63s", name, getter) == 2) {
            module_load_by_getter(getter);
        }
    }
    fclose(f);
    return 0;
}

void core_run_agent(const char *task) {
    context_init(&g_ctx, task);

    Module *scanner = module_find("scanner");
    Module *file    = module_find("file");
    Module *editor  = module_find("editor");

    if (scanner) scanner->exec(&g_ctx, ".");
    if (file)    file->exec(&g_ctx, "demo.c");
    if (editor)  editor->exec(&g_ctx, "edit demo.c");
}
