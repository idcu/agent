#include "idcu/module_system/module_system.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IDCU_MAX_MODULES 64

typedef struct idcu_ModuleEntry {
    idcu_ModuleInfo info;
    idcu_ModuleInitFunc init;
    idcu_ModuleCleanupFunc cleanup;
    idcu_ModuleUpdateFunc update;
    int is_loaded;
    int is_running;
} idcu_ModuleEntry;

static idcu_ModuleEntry g_modules[IDCU_MAX_MODULES];
static int g_module_count = 0;
static int g_initialized = 0;

int idcu_module_system_init(void) {
    if (g_initialized) {
        IDCU_LOG_WARNING("[module-system] Already initialized");
        return IDCU_ERR_OK;
    }

    memset(g_modules, 0, sizeof(g_modules));
    g_module_count = 0;
    g_initialized = 1;

    IDCU_LOG_INFO("[module-system] Initialized");
    return IDCU_ERR_OK;
}

void idcu_module_system_shutdown(void) {
    if (!g_initialized) {
        return;
    }

    for (int i = g_module_count - 1; i >= 0; i--) {
        idcu_ModuleEntry *entry = &g_modules[i];
        if (entry->is_running) {
            if (entry->cleanup) {
                entry->cleanup();
            }
            entry->is_running = 0;
            IDCU_LOG_INFO("[module-system] Stopped module: %s", entry->info.name);
        }
        if (entry->is_loaded) {
            entry->is_loaded = 0;
            IDCU_LOG_INFO("[module-system] Unloaded module: %s", entry->info.name);
        }
    }

    g_module_count = 0;
    g_initialized = 0;

    IDCU_LOG_INFO("[module-system] Shutdown");
}

int idcu_module_register(const char *name, const idcu_ModuleInfo *info, idcu_ModuleInitFunc init,
                         idcu_ModuleCleanupFunc cleanup, idcu_ModuleUpdateFunc update) {
    if (!g_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    if (!name || !info) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (g_module_count >= IDCU_MAX_MODULES) {
        return IDCU_ERR_LIMIT_EXCEEDED;
    }

    for (int i = 0; i < g_module_count; i++) {
        if (strcmp(g_modules[i].info.name, name) == 0) {
            return IDCU_ERR_ALREADY_EXISTS;
        }
    }

    idcu_ModuleEntry *entry = &g_modules[g_module_count];
    memcpy(&entry->info, info, sizeof(idcu_ModuleInfo));
    strncpy(entry->info.name, name, IDCU_MODULE_NAME_MAX - 1);
    entry->info.name[IDCU_MODULE_NAME_MAX - 1] = '\0';

    entry->init = init;
    entry->cleanup = cleanup;
    entry->update = update;
    entry->is_loaded = 0;
    entry->is_running = 0;

    g_module_count++;
    IDCU_LOG_INFO("[module-system] Registered module: %s v%s", info->name, info->version);
    return IDCU_ERR_OK;
}

int idcu_module_unregister(const char *name) {
    if (!g_initialized || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (int i = 0; i < g_module_count; i++) {
        idcu_ModuleEntry *entry = &g_modules[i];
        if (strcmp(entry->info.name, name) == 0) {
            if (entry->is_running) {
                if (entry->cleanup) {
                    entry->cleanup();
                }
                entry->is_running = 0;
            }
            entry->is_loaded = 0;

            if (i < g_module_count - 1) {
                memmove(&g_modules[i], &g_modules[i + 1],
                        (g_module_count - i - 1) * sizeof(idcu_ModuleEntry));
            }
            memset(&g_modules[g_module_count - 1], 0, sizeof(idcu_ModuleEntry));
            g_module_count--;

            IDCU_LOG_INFO("[module-system] Unregistered module: %s", name);
            return IDCU_ERR_OK;
        }
    }

    return IDCU_ERR_NOT_FOUND;
}

int idcu_module_load(const char *name, idcu_ModuleHandle **handle) {
    if (!g_initialized || !name || !handle) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (int i = 0; i < g_module_count; i++) {
        idcu_ModuleEntry *entry = &g_modules[i];
        if (strcmp(entry->info.name, name) == 0) {
            if (entry->is_loaded) {
                return IDCU_ERR_ALREADY_EXISTS;
            }

            entry->is_loaded = 1;
            *handle = (idcu_ModuleHandle *)entry;
            IDCU_LOG_INFO("[module-system] Loaded module: %s", name);
            return IDCU_ERR_OK;
        }
    }

    return IDCU_ERR_NOT_FOUND;
}

int idcu_module_unload(idcu_ModuleHandle *handle) {
    if (!g_initialized || !handle) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_ModuleEntry *entry = (idcu_ModuleEntry *)handle;

    if (entry->is_running) {
        if (entry->cleanup) {
            entry->cleanup();
        }
        entry->is_running = 0;
    }

    entry->is_loaded = 0;
    IDCU_LOG_INFO("[module-system] Unloaded module: %s", entry->info.name);
    return IDCU_ERR_OK;
}

int idcu_module_start(idcu_ModuleHandle *handle) {
    if (!g_initialized || !handle) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_ModuleEntry *entry = (idcu_ModuleEntry *)handle;

    if (!entry->is_loaded) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    if (entry->is_running) {
        return IDCU_ERR_OK;
    }

    int ret = IDCU_ERR_OK;
    if (entry->init) {
        ret = entry->init();
        if (ret != IDCU_ERR_OK) {
            IDCU_LOG_ERROR("[module-system] Failed to init module %s: %d", entry->info.name, ret);
            return ret;
        }
    }

    entry->is_running = 1;
    IDCU_LOG_INFO("[module-system] Started module: %s", entry->info.name);
    return IDCU_ERR_OK;
}

int idcu_module_stop(idcu_ModuleHandle *handle) {
    if (!g_initialized || !handle) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_ModuleEntry *entry = (idcu_ModuleEntry *)handle;

    if (!entry->is_running) {
        return IDCU_ERR_OK;
    }

    if (entry->cleanup) {
        entry->cleanup();
    }

    entry->is_running = 0;
    IDCU_LOG_INFO("[module-system] Stopped module: %s", entry->info.name);
    return IDCU_ERR_OK;
}

int idcu_module_restart(idcu_ModuleHandle *handle) {
    if (!g_initialized || !handle) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_module_stop(handle);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    return idcu_module_start(handle);
}

int idcu_module_is_loaded(const char *name) {
    if (!g_initialized || !name) {
        return 0;
    }

    for (int i = 0; i < g_module_count; i++) {
        if (strcmp(g_modules[i].info.name, name) == 0) {
            return g_modules[i].is_loaded;
        }
    }
    return 0;
}

int idcu_module_is_running(const char *name) {
    if (!g_initialized || !name) {
        return 0;
    }

    for (int i = 0; i < g_module_count; i++) {
        if (strcmp(g_modules[i].info.name, name) == 0) {
            return g_modules[i].is_running;
        }
    }
    return 0;
}

int idcu_module_get_info(const char *name, idcu_ModuleInfo *info) {
    if (!g_initialized || !name || !info) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (int i = 0; i < g_module_count; i++) {
        if (strcmp(g_modules[i].info.name, name) == 0) {
            memcpy(info, &g_modules[i].info, sizeof(idcu_ModuleInfo));
            return IDCU_ERR_OK;
        }
    }
    return IDCU_ERR_NOT_FOUND;
}

int idcu_module_get_count(void) {
    if (!g_initialized) {
        return 0;
    }
    return g_module_count;
}

int idcu_module_get_all_names(char **names, size_t max_names, size_t *actual_count) {
    if (!g_initialized || !names || !actual_count) {
        return IDCU_ERR_INVALID_PARAM;
    }

    *actual_count = (g_module_count < max_names) ? g_module_count : max_names;
    for (size_t i = 0; i < *actual_count; i++) {
        strncpy(names[i], g_modules[i].info.name, IDCU_MODULE_NAME_MAX - 1);
        names[i][IDCU_MODULE_NAME_MAX - 1] = '\0';
    }
    return IDCU_ERR_OK;
}

int idcu_module_system_update(uint64_t delta_ms) {
    if (!g_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    for (int i = 0; i < g_module_count; i++) {
        idcu_ModuleEntry *entry = &g_modules[i];
        if (entry->is_running && entry->update) {
            int ret = entry->update(delta_ms);
            if (ret != IDCU_ERR_OK) {
                IDCU_LOG_WARNING("[module-system] Module %s update failed: %d", entry->info.name,
                                 ret);
            }
        }
    }
    return IDCU_ERR_OK;
}

int idcu_module_system_save_state(const char *file_path) {
    if (!g_initialized || !file_path) {
        return IDCU_ERR_INVALID_PARAM;
    }

    FILE *fp = fopen(file_path, "w");
    if (!fp) {
        return IDCU_ERR_IO_ERROR;
    }

    fprintf(fp, "module_count=%d\n", g_module_count);
    for (int i = 0; i < g_module_count; i++) {
        idcu_ModuleEntry *entry = &g_modules[i];
        fprintf(fp, "[module%d]\n", i);
        fprintf(fp, "name=%s\n", entry->info.name);
        fprintf(fp, "loaded=%d\n", entry->is_loaded);
        fprintf(fp, "running=%d\n", entry->is_running);
    }

    fclose(fp);
    IDCU_LOG_INFO("[module-system] Saved state to %s", file_path);
    return IDCU_ERR_OK;
}

int idcu_module_system_load_state(const char *file_path) {
    (void)file_path;
    IDCU_LOG_WARNING("[module-system] Load state not fully implemented");
    return IDCU_ERR_OK;
}
