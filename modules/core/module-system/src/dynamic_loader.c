#include "dynamic_module.h"
#include "log.h"
#include <string.h>
#include <stdio.h>

int idcu_dynamic_loader_init(idcu_DynamicLoader* loader, const char* module_path)
{
    if (!loader) {
        return IDCU_ERR_INVALID_PARAM;
    }
    memset(loader, 0, sizeof(idcu_DynamicLoader));
    int ret = idcu_mutex_init(&loader->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    if (module_path) {
        strncpy(loader->module_path, module_path, IDCU_MODULE_PATH_MAX - 1);
        loader->module_path[IDCU_MODULE_PATH_MAX - 1] = '\0';
    }
    return IDCU_ERR_SUCCESS;
}

void idcu_dynamic_loader_destroy(idcu_DynamicLoader* loader)
{
    if (!loader) {
        return;
    }
    int ret = idcu_mutex_lock(&loader->lock);
    if (ret == IDCU_ERR_SUCCESS) {
        for (uint32_t i = 0; i < loader->count; i++) {
            idcu_DynamicModule* mod = &loader->modules[i];
            if (mod->state == IDCU_MOD_STATE_RUNNING) {
                idcu_dynamic_module_stop(mod);
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
        idcu_mutex_unlock(&loader->lock);
    }
    idcu_mutex_destroy(&loader->lock);
}

int idcu_dynamic_loader_load_module(idcu_DynamicLoader* loader, const char* name, const char* path)
{
    if (!loader || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&loader->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    for (uint32_t i = 0; i < loader->count; i++) {
        if (strcmp(loader->modules[i].name, name) == 0) {
            idcu_mutex_unlock(&loader->lock);
            return IDCU_ERR_ALREADY_EXISTS;
        }
    }
    if (loader->count >= IDCU_MAX_DYNAMIC_MODULES) {
        idcu_mutex_unlock(&loader->lock);
        return IDCU_ERR_QUEUE_FULL;
    }
    idcu_DynamicModule* mod = &loader->modules[loader->count];
    strncpy(mod->name, name, sizeof(mod->name) - 1);
    mod->name[sizeof(mod->name) - 1] = '\0';
    if (path) {
        strncpy(mod->path, path, sizeof(mod->path) - 1);
        mod->path[sizeof(mod->path) - 1] = '\0';
    }
    char full_path[IDCU_MODULE_PATH_MAX];
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
    if (!mod->handle) {
        DWORD err = GetLastError();
        IDCU_LOG_ERROR("Failed to load module %s (Windows error: %lu)", full_path, err);
    }
#else
    mod->handle = dlopen(full_path, RTLD_NOW | RTLD_LOCAL);
    if (!mod->handle) {
        const char* err = dlerror();
        IDCU_LOG_ERROR("Failed to load module %s: %s", full_path, err ? err : "unknown error");
    }
#endif
    if (!mod->handle) {
        idcu_mutex_unlock(&loader->lock);
        return IDCU_ERR_MODULE_LOAD;
    }
#ifdef _WIN32
    mod->iface = (idcu_ModuleInterface*)GetProcAddress(mod->handle, "module_interface");
    if (!mod->iface) {
        DWORD err = GetLastError();
        IDCU_LOG_ERROR("Failed to find module_interface in %s (Windows error: %lu)", full_path, err);
    }
#else
    mod->iface = (idcu_ModuleInterface*)dlsym(mod->handle, "module_interface");
    if (!mod->iface) {
        const char* err = dlerror();
        IDCU_LOG_ERROR("Failed to find module_interface in %s: %s", full_path, err ? err : "unknown error");
    }
#endif
    if (!mod->iface) {
#ifdef _WIN32
        FreeLibrary(mod->handle);
#else
        dlclose(mod->handle);
#endif
        mod->handle = NULL;
        idcu_mutex_unlock(&loader->lock);
        return IDCU_ERR_MODULE_INVALID;
    }
    mod->state = IDCU_MOD_STATE_UNINIT;
    mod->ref_count = 0;
    loader->count++;
    idcu_mutex_unlock(&loader->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_dynamic_loader_unload_module(idcu_DynamicLoader* loader, const char* name)
{
    if (!loader || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&loader->lock);
    if (ret != IDCU_ERR_SUCCESS) {
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
        idcu_mutex_unlock(&loader->lock);
        return IDCU_ERR_NOT_FOUND;
    }
    idcu_DynamicModule* mod = &loader->modules[found_idx];
    if (mod->state == IDCU_MOD_STATE_RUNNING) {
        idcu_dynamic_module_stop(mod);
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
                (loader->count - found_idx - 1) * sizeof(idcu_DynamicModule));
    }
    loader->count--;
    idcu_mutex_unlock(&loader->lock);
    return IDCU_ERR_SUCCESS;
}

idcu_DynamicModule* idcu_dynamic_loader_find_module(idcu_DynamicLoader* loader, const char* name)
{
    if (!loader || !name) {
        return NULL;
    }
    int ret = idcu_mutex_lock(&loader->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return NULL;
    }
    idcu_DynamicModule* result = NULL;
    for (uint32_t i = 0; i < loader->count; i++) {
        if (strcmp(loader->modules[i].name, name) == 0) {
            result = &loader->modules[i];
            break;
        }
    }
    idcu_mutex_unlock(&loader->lock);
    return result;
}

idcu_DynamicModule* idcu_dynamic_loader_get_at(idcu_DynamicLoader* loader, uint32_t index)
{
    if (!loader) {
        return NULL;
    }
    int ret = idcu_mutex_lock(&loader->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return NULL;
    }
    idcu_DynamicModule* result = NULL;
    if (index < loader->count) {
        result = &loader->modules[index];
    }
    idcu_mutex_unlock(&loader->lock);
    return result;
}

int idcu_dynamic_loader_get_count(idcu_DynamicLoader* loader)
{
    if (!loader) {
        return 0;
    }
    int ret = idcu_mutex_lock(&loader->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }
    int count = (int)loader->count;
    idcu_mutex_unlock(&loader->lock);
    return count;
}

int idcu_dynamic_module_init(idcu_DynamicModule* mod)
{
    if (!mod || !mod->iface) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (mod->state >= IDCU_MOD_STATE_INITED) {
        return IDCU_ERR_SUCCESS;
    }
    int ret = IDCU_ERR_SUCCESS;
    if (mod->iface->init) {
        ret = mod->iface->init();
        if (ret != IDCU_ERR_SUCCESS) {
            mod->state = IDCU_MOD_STATE_ERROR;
            return ret;
        }
    }
    mod->state = IDCU_MOD_STATE_INITED;
    return IDCU_ERR_SUCCESS;
}

int idcu_dynamic_module_run(idcu_DynamicModule* mod)
{
    if (!mod || !mod->iface) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (mod->state == IDCU_MOD_STATE_RUNNING) {
        return IDCU_ERR_SUCCESS;
    }
    if (mod->state == IDCU_MOD_STATE_UNINIT) {
        int ret = idcu_dynamic_module_init(mod);
        if (ret != IDCU_ERR_SUCCESS) {
            return ret;
        }
    }
    int ret = IDCU_ERR_SUCCESS;
    if (mod->iface->run) {
        ret = mod->iface->run();
        if (ret != IDCU_ERR_SUCCESS) {
            mod->state = IDCU_MOD_STATE_ERROR;
            return ret;
        }
    }
    mod->state = IDCU_MOD_STATE_RUNNING;
    return IDCU_ERR_SUCCESS;
}

int idcu_dynamic_module_stop(idcu_DynamicModule* mod)
{
    if (!mod || !mod->iface) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (mod->state != IDCU_MOD_STATE_RUNNING) {
        return IDCU_ERR_SUCCESS;
    }
    int ret = IDCU_ERR_SUCCESS;
    if (mod->iface->stop) {
        ret = mod->iface->stop();
    }
    mod->state = IDCU_MOD_STATE_STOPPED;
    return ret;
}

int idcu_dynamic_module_restart(idcu_DynamicLoader* loader, const char* name)
{
    if (!loader || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&loader->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    idcu_DynamicModule* mod = NULL;
    for (uint32_t i = 0; i < loader->count; i++) {
        if (strcmp(loader->modules[i].name, name) == 0) {
            mod = &loader->modules[i];
            break;
        }
    }
    if (!mod) {
        idcu_mutex_unlock(&loader->lock);
        return IDCU_ERR_NOT_FOUND;
    }
    ret = idcu_dynamic_module_stop(mod);
    if (ret != IDCU_ERR_SUCCESS) {
        idcu_mutex_unlock(&loader->lock);
        return ret;
    }
    mod->state = IDCU_MOD_STATE_UNINIT;
    ret = idcu_dynamic_module_init(mod);
    if (ret != IDCU_ERR_SUCCESS) {
        idcu_mutex_unlock(&loader->lock);
        return ret;
    }
    ret = idcu_dynamic_module_run(mod);
    idcu_mutex_unlock(&loader->lock);
    return ret;
}

int idcu_dynamic_module_reload(idcu_DynamicLoader* loader, const char* name, const char* path)
{
    if (!loader || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_dynamic_loader_unload_module(loader, name);
    if (ret != IDCU_ERR_SUCCESS && ret != IDCU_ERR_NOT_FOUND) {
        return ret;
    }
    return idcu_dynamic_loader_load_module(loader, name, path);
}

int idcu_dynamic_loader_hotplug_load(idcu_DynamicLoader* loader, const char* name, const char* path)
{
    if (!loader || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    IDCU_LOG_INFO("Hotplug loading module: %s", name);
    int ret = idcu_dynamic_loader_load_module(loader, name, path);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("Failed to hotplug load module %s: %d", name, ret);
        return ret;
    }
    idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(loader, name);
    if (!mod) {
        return IDCU_ERR_GENERAL;
    }
    ret = idcu_dynamic_module_init(mod);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("Failed to init hotplug module %s: %d", name, ret);
        return ret;
    }
    ret = idcu_dynamic_module_run(mod);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("Failed to run hotplug module %s: %d", name, ret);
        return ret;
    }
    IDCU_LOG_INFO("Hotplug module %s loaded and running", name);
    return IDCU_ERR_SUCCESS;
}

int idcu_dynamic_loader_hotplug_unload(idcu_DynamicLoader* loader, const char* name)
{
    if (!loader || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    IDCU_LOG_INFO("Hotplug unloading module: %s", name);
    int ret = idcu_mutex_lock(&loader->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    idcu_DynamicModule* mod = NULL;
    for (uint32_t i = 0; i < loader->count; i++) {
        if (strcmp(loader->modules[i].name, name) == 0) {
            mod = &loader->modules[i];
            break;
        }
    }
    if (!mod) {
        idcu_mutex_unlock(&loader->lock);
        return IDCU_ERR_NOT_FOUND;
    }
    if (mod->state == IDCU_MOD_STATE_RUNNING) {
        idcu_dynamic_module_stop(mod);
    }
    idcu_mutex_unlock(&loader->lock);
    ret = idcu_dynamic_loader_unload_module(loader, name);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("Failed to hotplug unload module %s: %d", name, ret);
        return ret;
    }
    IDCU_LOG_INFO("Hotplug module %s unloaded", name);
    return IDCU_ERR_SUCCESS;
}
