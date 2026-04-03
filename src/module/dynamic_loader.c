#include "module/dynamic_module.h"
#include <string.h>
#include <stdio.h>

int dynamic_loader_init(DynamicLoader* loader, const char* module_path)
{
    if (!loader) {
        return ERR_INVALID_PARAM;
    }
    memset(loader, 0, sizeof(DynamicLoader));
    int ret = mutex_init(&loader->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    if (module_path) {
        strncpy(loader->module_path, module_path, MODULE_PATH_MAX - 1);
        loader->module_path[MODULE_PATH_MAX - 1] = '\0';
    }
    return ERR_SUCCESS;
}

void dynamic_loader_destroy(DynamicLoader* loader)
{
    if (!loader) {
        return;
    }
    int ret = mutex_lock(&loader->lock);
    if (ret == ERR_SUCCESS) {
        for (uint32_t i = 0; i < loader->count; i++) {
            DynamicModule* mod = &loader->modules[i];
            if (mod->state == MOD_STATE_RUNNING) {
                dynamic_module_stop(mod);
            }
            if (mod->handle) {
#ifdef _WIN32
                FreeLibrary(mod->handle);
#else
                dlclose(mod->handle);
#endif
                mod->handle = NULL;
            }
        }
        mutex_unlock(&loader->lock);
    }
    mutex_destroy(&loader->lock);
}

int dynamic_loader_load_module(DynamicLoader* loader, const char* name, const char* path)
{
    if (!loader || !name) {
        return ERR_INVALID_PARAM;
    }
    int ret = mutex_lock(&loader->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    for (uint32_t i = 0; i < loader->count; i++) {
        if (strcmp(loader->modules[i].name, name) == 0) {
            mutex_unlock(&loader->lock);
            return ERR_ALREADY_EXISTS;
        }
    }
    if (loader->count >= MAX_DYNAMIC_MODULES) {
        mutex_unlock(&loader->lock);
        return ERR_QUEUE_FULL;
    }
    DynamicModule* mod = &loader->modules[loader->count];
    strncpy(mod->name, name, sizeof(mod->name) - 1);
    mod->name[sizeof(mod->name) - 1] = '\0';
    if (path) {
        strncpy(mod->path, path, sizeof(mod->path) - 1);
        mod->path[sizeof(mod->path) - 1] = '\0';
    }
    char full_path[MODULE_PATH_MAX];
    if (path) {
        strncpy(full_path, path, sizeof(full_path) - 1);
    } else if (loader->module_path[0]) {
        snprintf(full_path, sizeof(full_path), "%s/%s", loader->module_path, name);
    } else {
        strncpy(full_path, name, sizeof(full_path) - 1);
    }
    full_path[sizeof(full_path) - 1] = '\0';
#ifdef _WIN32
    mod->handle = LoadLibraryA(full_path);
#else
    mod->handle = dlopen(full_path, RTLD_NOW | RTLD_LOCAL);
#endif
    if (!mod->handle) {
        mutex_unlock(&loader->lock);
        return ERR_MODULE_LOAD;
    }
#ifdef _WIN32
    mod->iface = (ModuleInterface*)GetProcAddress(mod->handle, "module_interface");
#else
    mod->iface = (ModuleInterface*)dlsym(mod->handle, "module_interface");
#endif
    if (!mod->iface) {
#ifdef _WIN32
        FreeLibrary(mod->handle);
#else
        dlclose(mod->handle);
#endif
        mod->handle = NULL;
        mutex_unlock(&loader->lock);
        return ERR_MODULE_INVALID;
    }
    mod->state = MOD_STATE_UNINIT;
    mod->ref_count = 0;
    loader->count++;
    mutex_unlock(&loader->lock);
    return ERR_SUCCESS;
}

int dynamic_loader_unload_module(DynamicLoader* loader, const char* name)
{
    if (!loader || !name) {
        return ERR_INVALID_PARAM;
    }
    int ret = mutex_lock(&loader->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    int found_idx = -1;
    for (uint32_t i = 0; i < loader->count; i++) {
        if (strcmp(loader->modules[i].name, name) == 0) {
            found_idx = (int)i;
            break;
        }
    }
    if (found_idx < 0) {
        mutex_unlock(&loader->lock);
        return ERR_NOT_FOUND;
    }
    DynamicModule* mod = &loader->modules[found_idx];
    if (mod->state == MOD_STATE_RUNNING) {
        dynamic_module_stop(mod);
    }
    if (mod->handle) {
#ifdef _WIN32
        FreeLibrary(mod->handle);
#else
        dlclose(mod->handle);
#endif
        mod->handle = NULL;
    }
    if ((uint32_t)found_idx < loader->count - 1) {
        memmove(&loader->modules[found_idx], &loader->modules[found_idx + 1],
                (loader->count - found_idx - 1) * sizeof(DynamicModule));
    }
    loader->count--;
    mutex_unlock(&loader->lock);
    return ERR_SUCCESS;
}

DynamicModule* dynamic_loader_find_module(DynamicLoader* loader, const char* name)
{
    if (!loader || !name) {
        return NULL;
    }
    int ret = mutex_lock(&loader->lock);
    if (ret != ERR_SUCCESS) {
        return NULL;
    }
    DynamicModule* result = NULL;
    for (uint32_t i = 0; i < loader->count; i++) {
        if (strcmp(loader->modules[i].name, name) == 0) {
            result = &loader->modules[i];
            break;
        }
    }
    mutex_unlock(&loader->lock);
    return result;
}

DynamicModule* dynamic_loader_get_at(DynamicLoader* loader, uint32_t index)
{
    if (!loader) {
        return NULL;
    }
    int ret = mutex_lock(&loader->lock);
    if (ret != ERR_SUCCESS) {
        return NULL;
    }
    DynamicModule* result = NULL;
    if (index < loader->count) {
        result = &loader->modules[index];
    }
    mutex_unlock(&loader->lock);
    return result;
}

int dynamic_loader_get_count(DynamicLoader* loader)
{
    if (!loader) {
        return 0;
    }
    int ret = mutex_lock(&loader->lock);
    if (ret != ERR_SUCCESS) {
        return 0;
    }
    int count = (int)loader->count;
    mutex_unlock(&loader->lock);
    return count;
}

int dynamic_module_init(DynamicModule* mod)
{
    if (!mod || !mod->iface) {
        return ERR_INVALID_PARAM;
    }
    if (mod->state >= MOD_STATE_INITED) {
        return ERR_SUCCESS;
    }
    int ret = ERR_SUCCESS;
    if (mod->iface->init) {
        ret = mod->iface->init();
        if (ret != ERR_SUCCESS) {
            mod->state = MOD_STATE_ERROR;
            return ret;
        }
    }
    mod->state = MOD_STATE_INITED;
    return ERR_SUCCESS;
}

int dynamic_module_run(DynamicModule* mod)
{
    if (!mod || !mod->iface) {
        return ERR_INVALID_PARAM;
    }
    if (mod->state == MOD_STATE_RUNNING) {
        return ERR_SUCCESS;
    }
    if (mod->state == MOD_STATE_UNINIT) {
        int ret = dynamic_module_init(mod);
        if (ret != ERR_SUCCESS) {
            return ret;
        }
    }
    int ret = ERR_SUCCESS;
    if (mod->iface->run) {
        ret = mod->iface->run();
        if (ret != ERR_SUCCESS) {
            mod->state = MOD_STATE_ERROR;
            return ret;
        }
    }
    mod->state = MOD_STATE_RUNNING;
    return ERR_SUCCESS;
}

int dynamic_module_stop(DynamicModule* mod)
{
    if (!mod || !mod->iface) {
        return ERR_INVALID_PARAM;
    }
    if (mod->state != MOD_STATE_RUNNING) {
        return ERR_SUCCESS;
    }
    int ret = ERR_SUCCESS;
    if (mod->iface->stop) {
        ret = mod->iface->stop();
    }
    mod->state = MOD_STATE_STOPPED;
    return ret;
}
