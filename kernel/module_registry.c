#include "module_registry.h"
#include <string.h>

int module_registry_init(ModuleRegistry* registry)
{
    if (!registry) {
        return ERR_INVALID_PARAM;
    }
    memset(registry, 0, sizeof(ModuleRegistry));
    int ret = mutex_init(&registry->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    registry->next_id = 1;
    return ERR_SUCCESS;
}

void module_registry_destroy(ModuleRegistry* registry)
{
    if (!registry) {
        return;
    }
    module_registry_stop_all(registry);
    mutex_destroy(&registry->lock);
    memset(registry, 0, sizeof(ModuleRegistry));
}

int module_registry_register(ModuleRegistry* registry, const ModuleInterface* iface, ModulePrio priority)
{
    if (!registry || !iface || !iface->name) {
        return ERR_INVALID_PARAM;
    }
    int ret = mutex_lock(&registry->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    for (uint32_t i = 0; i < registry->count; i++) {
        if (strcmp(registry->modules[i].iface->name, iface->name) == 0) {
            mutex_unlock(&registry->lock);
            return ERR_ALREADY_EXISTS;
        }
    }
    if (registry->count >= MAX_REGISTERED_MODULES) {
        mutex_unlock(&registry->lock);
        return ERR_QUEUE_FULL;
    }
    RegisteredModule* mod = &registry->modules[registry->count];
    mod->iface = iface;
    mod->module_id = registry->next_id++;
    mod->state = MOD_STATE_UNINIT;
    mod->priority = priority;
    mod->user_data = NULL;
    registry->count++;
    mutex_unlock(&registry->lock);
    return ERR_SUCCESS;
}

int module_registry_unregister(ModuleRegistry* registry, uint32_t module_id)
{
    if (!registry) {
        return ERR_INVALID_PARAM;
    }
    int ret = mutex_lock(&registry->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    int found_idx = -1;
    for (uint32_t i = 0; i < registry->count; i++) {
        if (registry->modules[i].module_id == module_id) {
            found_idx = (int)i;
            break;
        }
    }
    if (found_idx < 0) {
        mutex_unlock(&registry->lock);
        return ERR_NOT_FOUND;
    }
    RegisteredModule* mod = &registry->modules[found_idx];
    if (mod->state == MOD_STATE_RUNNING) {
        module_registry_stop_module(registry, module_id);
    }
    if ((uint32_t)found_idx < registry->count - 1) {
        memmove(&registry->modules[found_idx], &registry->modules[found_idx + 1],
                (registry->count - found_idx - 1) * sizeof(RegisteredModule));
    }
    registry->count--;
    mutex_unlock(&registry->lock);
    return ERR_SUCCESS;
}

const RegisteredModule* module_registry_find_by_id(ModuleRegistry* registry, uint32_t module_id)
{
    if (!registry) {
        return NULL;
    }
    int ret = mutex_lock(&registry->lock);
    if (ret != ERR_SUCCESS) {
        return NULL;
    }
    const RegisteredModule* result = NULL;
    for (uint32_t i = 0; i < registry->count; i++) {
        if (registry->modules[i].module_id == module_id) {
            result = &registry->modules[i];
            break;
        }
    }
    mutex_unlock(&registry->lock);
    return result;
}

const RegisteredModule* module_registry_find_by_name(ModuleRegistry* registry, const char* name)
{
    if (!registry || !name) {
        return NULL;
    }
    int ret = mutex_lock(&registry->lock);
    if (ret != ERR_SUCCESS) {
        return NULL;
    }
    const RegisteredModule* result = NULL;
    for (uint32_t i = 0; i < registry->count; i++) {
        if (strcmp(registry->modules[i].iface->name, name) == 0) {
            result = &registry->modules[i];
            break;
        }
    }
    mutex_unlock(&registry->lock);
    return result;
}

int module_registry_get_count(ModuleRegistry* registry)
{
    if (!registry) {
        return 0;
    }
    int ret = mutex_lock(&registry->lock);
    if (ret != ERR_SUCCESS) {
        return 0;
    }
    int count = (int)registry->count;
    mutex_unlock(&registry->lock);
    return count;
}

const RegisteredModule* module_registry_get_at(ModuleRegistry* registry, uint32_t index)
{
    if (!registry) {
        return NULL;
    }
    int ret = mutex_lock(&registry->lock);
    if (ret != ERR_SUCCESS) {
        return NULL;
    }
    const RegisteredModule* result = NULL;
    if (index < registry->count) {
        result = &registry->modules[index];
    }
    mutex_unlock(&registry->lock);
    return result;
}

int module_registry_init_module(ModuleRegistry* registry, uint32_t module_id)
{
    if (!registry) {
        return ERR_INVALID_PARAM;
    }
    int ret = mutex_lock(&registry->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    RegisteredModule* mod = NULL;
    for (uint32_t i = 0; i < registry->count; i++) {
        if (registry->modules[i].module_id == module_id) {
            mod = &registry->modules[i];
            break;
        }
    }
    if (!mod) {
        mutex_unlock(&registry->lock);
        return ERR_NOT_FOUND;
    }
    if (mod->state >= MOD_STATE_INITED) {
        mutex_unlock(&registry->lock);
        return ERR_SUCCESS;
    }
    int result = ERR_SUCCESS;
    if (mod->iface->init) {
        result = mod->iface->init();
        if (result != ERR_SUCCESS) {
            mod->state = MOD_STATE_ERROR;
            mutex_unlock(&registry->lock);
            return result;
        }
    }
    mod->state = MOD_STATE_INITED;
    mutex_unlock(&registry->lock);
    return ERR_SUCCESS;
}

int module_registry_run_module(ModuleRegistry* registry, uint32_t module_id)
{
    if (!registry) {
        return ERR_INVALID_PARAM;
    }
    int ret = mutex_lock(&registry->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    RegisteredModule* mod = NULL;
    for (uint32_t i = 0; i < registry->count; i++) {
        if (registry->modules[i].module_id == module_id) {
            mod = &registry->modules[i];
            break;
        }
    }
    if (!mod) {
        mutex_unlock(&registry->lock);
        return ERR_NOT_FOUND;
    }
    if (mod->state == MOD_STATE_RUNNING) {
        mutex_unlock(&registry->lock);
        return ERR_SUCCESS;
    }
    if (mod->state == MOD_STATE_UNINIT) {
        mutex_unlock(&registry->lock);
        int init_ret = module_registry_init_module(registry, module_id);
        if (init_ret != ERR_SUCCESS) {
            return init_ret;
        }
        ret = mutex_lock(&registry->lock);
        if (ret != ERR_SUCCESS) {
            return ret;
        }
    }
    int result = ERR_SUCCESS;
    if (mod->iface->run) {
        result = mod->iface->run();
        if (result != ERR_SUCCESS) {
            mod->state = MOD_STATE_ERROR;
            mutex_unlock(&registry->lock);
            return result;
        }
    }
    mod->state = MOD_STATE_RUNNING;
    mutex_unlock(&registry->lock);
    return ERR_SUCCESS;
}

int module_registry_stop_module(ModuleRegistry* registry, uint32_t module_id)
{
    if (!registry) {
        return ERR_INVALID_PARAM;
    }
    int ret = mutex_lock(&registry->lock);
    if (ret != ERR_SUCCESS) {
        return ret;
    }
    RegisteredModule* mod = NULL;
    for (uint32_t i = 0; i < registry->count; i++) {
        if (registry->modules[i].module_id == module_id) {
            mod = &registry->modules[i];
            break;
        }
    }
    if (!mod) {
        mutex_unlock(&registry->lock);
        return ERR_NOT_FOUND;
    }
    if (mod->state != MOD_STATE_RUNNING) {
        mutex_unlock(&registry->lock);
        return ERR_SUCCESS;
    }
    int result = ERR_SUCCESS;
    if (mod->iface->stop) {
        result = mod->iface->stop();
    }
    mod->state = MOD_STATE_STOPPED;
    mutex_unlock(&registry->lock);
    return result;
}

int module_registry_init_all(ModuleRegistry* registry)
{
    if (!registry) {
        return ERR_INVALID_PARAM;
    }
    for (uint32_t i = 0; i < registry->count; i++) {
        RegisteredModule* mod = &registry->modules[i];
        int ret = module_registry_init_module(registry, mod->module_id);
        if (ret != ERR_SUCCESS) {
            return ret;
        }
    }
    return ERR_SUCCESS;
}

int module_registry_run_all(ModuleRegistry* registry)
{
    if (!registry) {
        return ERR_INVALID_PARAM;
    }
    for (int prio = MOD_PRIO_REALTIME; prio >= MOD_PRIO_LOW; prio--) {
        for (uint32_t i = 0; i < registry->count; i++) {
            RegisteredModule* mod = &registry->modules[i];
            if (mod->priority == (ModulePrio)prio) {
                int ret = module_registry_run_module(registry, mod->module_id);
                if (ret != ERR_SUCCESS) {
                    return ret;
                }
            }
        }
    }
    return ERR_SUCCESS;
}

int module_registry_stop_all(ModuleRegistry* registry)
{
    if (!registry) {
        return ERR_INVALID_PARAM;
    }
    for (int prio = MOD_PRIO_LOW; prio <= MOD_PRIO_REALTIME; prio++) {
        for (uint32_t i = 0; i < registry->count; i++) {
            RegisteredModule* mod = &registry->modules[i];
            if (mod->priority == (ModulePrio)prio && mod->state == MOD_STATE_RUNNING) {
                module_registry_stop_module(registry, mod->module_id);
            }
        }
    }
    return ERR_SUCCESS;
}
