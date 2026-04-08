#include "idcu/plugin/plugin.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define IDCU_PLUGIN_HANDLE HMODULE
#define IDCU_PLUGIN_LOAD(path) LoadLibraryA(path)
#define IDCU_PLUGIN_UNLOAD(handle) FreeLibrary(handle)
#define IDCU_PLUGIN_GET_SYMBOL(handle, name) GetProcAddress(handle, name)
#define IDCU_PLUGIN_EXT ".dll"
#else
#include <dlfcn.h>
#define IDCU_PLUGIN_HANDLE void *
#define IDCU_PLUGIN_LOAD(path) dlopen(path, RTLD_NOW | RTLD_LOCAL)
#define IDCU_PLUGIN_UNLOAD(handle) dlclose(handle)
#define IDCU_PLUGIN_GET_SYMBOL(handle, name) dlsym(handle, name)
#define IDCU_PLUGIN_EXT ".so"
#endif

#define IDCU_MAX_PLUGINS 64

typedef struct idcu_PluginEntry {
    char path[IDCU_PLUGIN_PATH_MAX];
    char name[IDCU_PLUGIN_NAME_MAX];
    IDCU_PLUGIN_HANDLE handle;
    idcu_PluginInfo info;
    idcu_PluginInitFunc init;
    idcu_PluginCleanupFunc cleanup;
    int is_loaded;
    int is_initialized;
} idcu_PluginEntry;

static idcu_PluginEntry g_plugins[IDCU_MAX_PLUGINS];
static size_t g_plugin_count = 0;
static int g_initialized = 0;

int idcu_plugin_system_init(void) {
    if (g_initialized) {
        IDCU_LOG_WARNING("[plugin] System already initialized");
        return IDCU_ERR_OK;
    }

    memset(g_plugins, 0, sizeof(g_plugins));
    g_plugin_count = 0;
    g_initialized = 1;

    IDCU_LOG_INFO("[plugin] System initialized");
    return IDCU_ERR_OK;
}

void idcu_plugin_system_shutdown(void) {
    if (!g_initialized) {
        return;
    }

    idcu_plugin_unload_all();

    memset(g_plugins, 0, sizeof(g_plugins));
    g_plugin_count = 0;
    g_initialized = 0;

    IDCU_LOG_INFO("[plugin] System shutdown");
}

int idcu_plugin_load(const char *path, idcu_PluginHandle **handle) {
    if (!g_initialized || !path || !handle) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (g_plugin_count >= IDCU_MAX_PLUGINS) {
        return IDCU_ERR_LIMIT_EXCEEDED;
    }

    for (size_t i = 0; i < g_plugin_count; i++) {
        if (strcmp(g_plugins[i].path, path) == 0) {
            *handle = (idcu_PluginHandle *)&g_plugins[i];
            return IDCU_ERR_OK;
        }
    }

    IDCU_PLUGIN_HANDLE lib_handle = IDCU_PLUGIN_LOAD(path);
    if (!lib_handle) {
#ifdef _WIN32
        DWORD err = GetLastError();
        IDCU_LOG_ERROR("[plugin] Failed to load %s: error %lu", path, err);
#else
        IDCU_LOG_ERROR("[plugin] Failed to load %s: %s", path, dlerror());
#endif
        return IDCU_ERR_IO_ERROR;
    }

    idcu_PluginEntry *entry = &g_plugins[g_plugin_count];
    strncpy(entry->path, path, IDCU_PLUGIN_PATH_MAX - 1);
    entry->path[IDCU_PLUGIN_PATH_MAX - 1] = '\0';
    entry->handle = lib_handle;
    entry->is_loaded = 1;
    entry->is_initialized = 0;

    idcu_PluginGetInfoFunc get_info =
        (idcu_PluginGetInfoFunc)IDCU_PLUGIN_GET_SYMBOL(lib_handle, "idcu_plugin_get_info");
    if (get_info) {
        get_info(&entry->info);
        strncpy(entry->name, entry->info.name, IDCU_PLUGIN_NAME_MAX - 1);
        entry->name[IDCU_PLUGIN_NAME_MAX - 1] = '\0';
    } else {
        const char *name = strrchr(path, '/');
#ifdef _WIN32
        if (!name)
            name = strrchr(path, '\\');
#endif
        if (name)
            name++;
        else
            name = path;
        strncpy(entry->name, name, IDCU_PLUGIN_NAME_MAX - 1);
        entry->name[IDCU_PLUGIN_NAME_MAX - 1] = '\0';
        char *dot = strrchr(entry->name, '.');
        if (dot)
            *dot = '\0';
        strncpy(entry->info.name, entry->name, IDCU_PLUGIN_NAME_MAX - 1);
    }

    entry->init = (idcu_PluginInitFunc)IDCU_PLUGIN_GET_SYMBOL(lib_handle, "idcu_plugin_init");
    entry->cleanup =
        (idcu_PluginCleanupFunc)IDCU_PLUGIN_GET_SYMBOL(lib_handle, "idcu_plugin_cleanup");

    if (entry->init) {
        int ret = entry->init();
        if (ret != IDCU_ERR_OK) {
            IDCU_PLUGIN_UNLOAD(lib_handle);
            memset(entry, 0, sizeof(idcu_PluginEntry));
            IDCU_LOG_ERROR("[plugin] Plugin %s init failed: %d", entry->name, ret);
            return ret;
        }
        entry->is_initialized = 1;
    }

    g_plugin_count++;
    *handle = (idcu_PluginHandle *)entry;

    IDCU_LOG_INFO("[plugin] Loaded plugin: %s", entry->name);
    return IDCU_ERR_OK;
}

int idcu_plugin_unload(idcu_PluginHandle *handle) {
    if (!g_initialized || !handle) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_PluginEntry *entry = (idcu_PluginEntry *)handle;

    if (!entry->is_loaded) {
        return IDCU_ERR_OK;
    }

    if (entry->is_initialized && entry->cleanup) {
        entry->cleanup();
        entry->is_initialized = 0;
    }

    if (entry->handle) {
        IDCU_PLUGIN_UNLOAD(entry->handle);
        entry->handle = NULL;
    }

    entry->is_loaded = 0;
    IDCU_LOG_INFO("[plugin] Unloaded plugin: %s", entry->name);
    return IDCU_ERR_OK;
}

int idcu_plugin_get_info(idcu_PluginHandle *handle, idcu_PluginInfo *info) {
    if (!g_initialized || !handle || !info) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_PluginEntry *entry = (idcu_PluginEntry *)handle;
    memcpy(info, &entry->info, sizeof(idcu_PluginInfo));
    return IDCU_ERR_OK;
}

int idcu_plugin_get_name(idcu_PluginHandle *handle, char *buffer, size_t buffer_size) {
    if (!g_initialized || !handle || !buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_PluginEntry *entry = (idcu_PluginEntry *)handle;
    strncpy(buffer, entry->name, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return IDCU_ERR_OK;
}

int idcu_plugin_is_loaded(const char *name) {
    if (!g_initialized || !name) {
        return 0;
    }

    for (size_t i = 0; i < g_plugin_count; i++) {
        if (strcmp(g_plugins[i].name, name) == 0 && g_plugins[i].is_loaded) {
            return 1;
        }
    }
    return 0;
}

int idcu_plugin_scan_directory(const char *directory) {
    (void)directory;
    IDCU_LOG_WARNING("[plugin] Directory scanning not fully implemented");
    return IDCU_ERR_OK;
}

int idcu_plugin_load_all(void) {
    IDCU_LOG_WARNING("[plugin] Load all not fully implemented");
    return IDCU_ERR_OK;
}

int idcu_plugin_unload_all(void) {
    for (size_t i = 0; i < g_plugin_count; i++) {
        idcu_plugin_unload((idcu_PluginHandle *)&g_plugins[i]);
    }

    memset(g_plugins, 0, sizeof(g_plugins));
    g_plugin_count = 0;

    return IDCU_ERR_OK;
}

int idcu_plugin_get_count(void) {
    if (!g_initialized) {
        return 0;
    }
    return (int)g_plugin_count;
}

int idcu_plugin_get_all_names(char **names, size_t max_names, size_t *actual_count) {
    if (!g_initialized || !names || !actual_count) {
        return IDCU_ERR_INVALID_PARAM;
    }

    *actual_count = (g_plugin_count < max_names) ? g_plugin_count : max_names;
    for (size_t i = 0; i < *actual_count; i++) {
        strncpy(names[i], g_plugins[i].name, IDCU_PLUGIN_NAME_MAX - 1);
        names[i][IDCU_PLUGIN_NAME_MAX - 1] = '\0';
    }
    return IDCU_ERR_OK;
}

int idcu_plugin_get_symbol(idcu_PluginHandle *handle, const char *symbol_name, void **symbol) {
    if (!g_initialized || !handle || !symbol_name || !symbol) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_PluginEntry *entry = (idcu_PluginEntry *)handle;
    if (!entry->handle) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    *symbol = IDCU_PLUGIN_GET_SYMBOL(entry->handle, symbol_name);
    if (!*symbol) {
        return IDCU_ERR_NOT_FOUND;
    }

    return IDCU_ERR_OK;
}
