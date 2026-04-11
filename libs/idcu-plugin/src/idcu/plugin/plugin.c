#include "idcu/plugin/plugin.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

static void* plugin_load_library(const char* path)
{
#ifdef _WIN32
    return LoadLibraryA(path);
#else
    return dlopen(path, RTLD_NOW | RTLD_LOCAL);
#endif
}

static void plugin_unload_library(void* handle)
{
#ifdef _WIN32
    FreeLibrary((HMODULE)handle);
#else
    dlclose(handle);
#endif
}

static void* plugin_get_symbol(void* handle, const char* symbol_name)
{
#ifdef _WIN32
    return GetProcAddress((HMODULE)handle, symbol_name);
#else
    return dlsym(handle, symbol_name);
#endif
}

int idcu_plugin_system_init(idcu_Plugin_Context** ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    *ctx = (idcu_Plugin_Context*)calloc(1, sizeof(idcu_Plugin_Context));
    if (!*ctx) {
        return IDCU_ERR_MEMORY;
    }

    idcu_mutex_init(&(*ctx)->lock);
    (*ctx)->initialized = 1;
    (*ctx)->plugin_count = 0;
    (*ctx)->operation_count = 0;
    (*ctx)->error_count = 0;

    return IDCU_ERR_OK;
}

void idcu_plugin_system_shutdown(idcu_Plugin_Context* ctx)
{
    if (!ctx) {
        return;
    }

    idcu_mutex_lock(&ctx->lock);
    idcu_Mutex lock_copy = ctx->lock;
    ctx->initialized = 0;

    for (int i = 0; i < ctx->plugin_count; i++) {
        if (ctx->plugins[i]) {
            if (ctx->plugins[i]->handle) {
                plugin_unload_library(ctx->plugins[i]->handle);
            }
            free(ctx->plugins[i]);
            ctx->plugins[i] = NULL;
        }
    }
    ctx->plugin_count = 0;

    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);

    free(ctx);
}

int idcu_plugin_load(idcu_Plugin_Context* ctx, const char* path, idcu_PluginHandle** handle)
{
    if (!ctx || !path || !handle) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    if (ctx->plugin_count >= IDCU_PLUGIN_MAX_ITEMS) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_LIMIT_EXCEEDED;
    }

    idcu_PluginHandle* plugin = (idcu_PluginHandle*)calloc(1, sizeof(idcu_PluginHandle));
    if (!plugin) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_MEMORY;
    }

    strncpy(plugin->path, path, IDCU_PLUGIN_PATH_MAX - 1);
    plugin->path[IDCU_PLUGIN_PATH_MAX - 1] = '\0';

    plugin->handle = plugin_load_library(path);
    if (!plugin->handle) {
        free(plugin);
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_MODULE_LOAD;
    }

    idcu_PluginGetInfoFunc get_info = (idcu_PluginGetInfoFunc)plugin_get_symbol(plugin->handle, "idcu_plugin_get_info");
    if (get_info) {
        get_info(&plugin->info);
    } else {
        strncpy(plugin->info.name, "unknown", IDCU_PLUGIN_NAME_MAX - 1);
        strncpy(plugin->info.version, "1.0.0", IDCU_PLUGIN_VERSION_MAX - 1);
    }

    plugin->loaded = 1;
    ctx->plugins[ctx->plugin_count++] = plugin;
    *handle = plugin;

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_plugin_unload(idcu_Plugin_Context* ctx, idcu_PluginHandle* handle)
{
    if (!ctx || !handle) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    int found_index = -1;
    for (int i = 0; i < ctx->plugin_count; i++) {
        if (ctx->plugins[i] == handle) {
            found_index = i;
            break;
        }
    }

    if (found_index == -1) {
        idcu_mutex_unlock(&ctx->lock);
        return IDCU_ERR_NOT_FOUND;
    }

    if (handle->handle) {
        plugin_unload_library(handle->handle);
    }
    free(handle);

    for (int i = found_index; i < ctx->plugin_count - 1; i++) {
        ctx->plugins[i] = ctx->plugins[i + 1];
    }
    ctx->plugin_count--;

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_plugin_get_info(idcu_PluginHandle* handle, idcu_PluginInfo* info)
{
    if (!handle || !info) {
        return IDCU_ERR_INVALID_ARG;
    }

    *info = handle->info;
    return IDCU_ERR_OK;
}

int idcu_plugin_get_name(idcu_PluginHandle* handle, char* buffer, size_t buffer_size)
{
    if (!handle || !buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_ARG;
    }

    strncpy(buffer, handle->info.name, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return IDCU_ERR_OK;
}

int idcu_plugin_is_loaded(idcu_Plugin_Context* ctx, const char* name)
{
    if (!ctx || !name) {
        return 0;
    }

    idcu_mutex_lock(&ctx->lock);

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        return 0;
    }

    int result = 0;
    for (int i = 0; i < ctx->plugin_count; i++) {
        if (strcmp(ctx->plugins[i]->info.name, name) == 0) {
            result = 1;
            break;
        }
    }

    idcu_mutex_unlock(&ctx->lock);
    return result;
}

int idcu_plugin_scan_directory(idcu_Plugin_Context* ctx, const char* directory)
{
    if (!ctx || !directory) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_plugin_load_all(idcu_Plugin_Context* ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_plugin_unload_all(idcu_Plugin_Context* ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    for (int i = 0; i < ctx->plugin_count; i++) {
        if (ctx->plugins[i]) {
            if (ctx->plugins[i]->handle) {
                plugin_unload_library(ctx->plugins[i]->handle);
            }
            free(ctx->plugins[i]);
            ctx->plugins[i] = NULL;
        }
    }
    ctx->plugin_count = 0;

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_plugin_get_count(idcu_Plugin_Context* ctx)
{
    if (!ctx) {
        return 0;
    }

    idcu_mutex_lock(&ctx->lock);
    int count = ctx->initialized ? ctx->plugin_count : 0;
    idcu_mutex_unlock(&ctx->lock);
    return count;
}

int idcu_plugin_get_all_names(idcu_Plugin_Context* ctx, char** names, size_t max_names, size_t* actual_count)
{
    if (!ctx || !names || !actual_count) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    *actual_count = (size_t)ctx->plugin_count < max_names ? (size_t)ctx->plugin_count : max_names;
    for (size_t i = 0; i < *actual_count; i++) {
        names[i] = ctx->plugins[i]->info.name;
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_plugin_get_symbol(idcu_PluginHandle* handle, const char* symbol_name, void** symbol)
{
    if (!handle || !symbol_name || !symbol) {
        return IDCU_ERR_INVALID_ARG;
    }

    if (!handle->handle) {
        return IDCU_ERR_MODULE_NOT_FOUND;
    }

    *symbol = plugin_get_symbol(handle->handle, symbol_name);
    if (!*symbol) {
        return IDCU_ERR_NOT_FOUND;
    }

    return IDCU_ERR_OK;
}
