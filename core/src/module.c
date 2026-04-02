#include "module.h"
#include <stdio.h>
#include <string.h>

static Module g_modules[MAX_MODULES];
static int g_mod_count = 0;

void module_register(Module *m) {
    if (!m || g_mod_count >= MAX_MODULES)
        return;
    g_modules[g_mod_count++] = *m;
    printf("[module] 注册: %s\n", m->name);
    if (m->init) m->init();
}

Module *module_find(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < g_mod_count; i++) {
        if (strcmp(g_modules[i].name, name) == 0)
            return &g_modules[i];
    }
    return NULL;
}

int module_load_by_getter(const char *getter_name) {
    typedef Module *(*ModuleGetter)(void);

    if (!strcmp(getter_name, "json_module_get")) {
        extern Module *json_module_get(void);
        module_register(json_module_get());
    } else if (!strcmp(getter_name, "yaml_module_get")) {
        extern Module *yaml_module_get(void);
        module_register(yaml_module_get());
    } else if (!strcmp(getter_name, "platform_module_get")) {
        extern Module *platform_module_get(void);
        module_register(platform_module_get());
    } else if (!strcmp(getter_name, "config_module_get")) {
        extern Module *config_module_get(void);
        module_register(config_module_get());
    } else if (!strcmp(getter_name, "file_module_get")) {
        extern Module *file_module_get(void);
        module_register(file_module_get());
    } else if (!strcmp(getter_name, "editor_module_get")) {
        extern Module *editor_module_get(void);
        module_register(editor_module_get());
    } else if (!strcmp(getter_name, "scanner_module_get")) {
        extern Module *scanner_module_get(void);
        module_register(scanner_module_get());
    } else {
        printf("[module] 未知模块: %s\n", getter_name);
        return -1;
    }
    return 0;
}
