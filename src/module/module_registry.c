#include "module/module_registry.h"
#include <string.h>

int idcu_module_registry_init(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    memset(registry, 0, sizeof(idcu_ModuleRegistry));
    int ret = idcu_mutex_init(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    registry->next_id = 1;
    return IDCU_ERR_SUCCESS;
}

void idcu_module_registry_destroy(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return;
    }
    idcu_module_registry_stop_all(registry);
    idcu_mutex_destroy(&registry->lock);
    memset(registry, 0, sizeof(idcu_ModuleRegistry));
}

int idcu_module_registry_register(idcu_ModuleRegistry* registry, const idcu_ModuleInterface* iface, idcu_ModulePrio priority)
{
    if (!registry || !iface || !iface->name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    for (uint32_t i = 0; i < registry->count; i++) {
        if (strcmp(registry->modules[i].iface->name, iface->name) == 0) {
            idcu_mutex_unlock(&registry->lock);
            return IDCU_ERR_ALREADY_EXISTS;
        }
    }
    if (registry->count >= IDCU_MAX_REGISTERED_MODULES) {
        idcu_mutex_unlock(&registry->lock);
        return IDCU_ERR_QUEUE_FULL;
    }
    idcu_RegisteredModule* mod = &registry->modules[registry->count];
    mod->iface = iface;
    mod->module_id = registry->next_id++;
    mod->state = IDCU_MOD_STATE_UNINIT;
    mod->priority = priority;
    mod->user_data = NULL;
    registry->count++;
    idcu_mutex_unlock(&registry->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_module_registry_unregister(idcu_ModuleRegistry* registry, uint32_t module_id)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
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
        idcu_mutex_unlock(&registry->lock);
        return IDCU_ERR_NOT_FOUND;
    }
    idcu_RegisteredModule* mod = &registry->modules[found_idx];
    if (mod->state == IDCU_MOD_STATE_RUNNING) {
        idcu_module_registry_stop_module(registry, module_id);
    }
    if ((uint32_t)found_idx < registry->count - 1) {
        memmove(&registry->modules[found_idx], &registry->modules[found_idx + 1],
                (registry->count - found_idx - 1) * sizeof(idcu_RegisteredModule));
    }
    registry->count--;
    idcu_mutex_unlock(&registry->lock);
    return IDCU_ERR_SUCCESS;
}

const idcu_RegisteredModule* idcu_module_registry_find_by_id(idcu_ModuleRegistry* registry, uint32_t module_id)
{
    if (!registry) {
        return NULL;
    }
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return NULL;
    }
    const idcu_RegisteredModule* result = NULL;
    for (uint32_t i = 0; i < registry->count; i++) {
        if (registry->modules[i].module_id == module_id) {
            result = &registry->modules[i];
            break;
        }
    }
    idcu_mutex_unlock(&registry->lock);
    return result;
}

const idcu_RegisteredModule* idcu_module_registry_find_by_name(idcu_ModuleRegistry* registry, const char* name)
{
    if (!registry || !name) {
        return NULL;
    }
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return NULL;
    }
    const idcu_RegisteredModule* result = NULL;
    for (uint32_t i = 0; i < registry->count; i++) {
        if (strcmp(registry->modules[i].iface->name, name) == 0) {
            result = &registry->modules[i];
            break;
        }
    }
    idcu_mutex_unlock(&registry->lock);
    return result;
}

int idcu_module_registry_get_count(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return 0;
    }
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }
    int count = (int)registry->count;
    idcu_mutex_unlock(&registry->lock);
    return count;
}

const idcu_RegisteredModule* idcu_module_registry_get_at(idcu_ModuleRegistry* registry, uint32_t index)
{
    if (!registry) {
        return NULL;
    }
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return NULL;
    }
    const idcu_RegisteredModule* result = NULL;
    if (index < registry->count) {
        result = &registry->modules[index];
    }
    idcu_mutex_unlock(&registry->lock);
    return result;
}

int idcu_module_registry_init_module(idcu_ModuleRegistry* registry, uint32_t module_id)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    idcu_RegisteredModule* mod = NULL;
    for (uint32_t i = 0; i < registry->count; i++) {
        if (registry->modules[i].module_id == module_id) {
            mod = &registry->modules[i];
            break;
        }
    }
    if (!mod) {
        idcu_mutex_unlock(&registry->lock);
        return IDCU_ERR_NOT_FOUND;
    }
    if (mod->state >= IDCU_MOD_STATE_INITED) {
        idcu_mutex_unlock(&registry->lock);
        return IDCU_ERR_SUCCESS;
    }
    int result = IDCU_ERR_SUCCESS;
    if (mod->iface->init) {
        result = mod->iface->init();
        if (result != IDCU_ERR_SUCCESS) {
            mod->state = IDCU_MOD_STATE_ERROR;
            idcu_mutex_unlock(&registry->lock);
            return result;
        }
    }
    mod->state = IDCU_MOD_STATE_INITED;
    idcu_mutex_unlock(&registry->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_module_registry_run_module(idcu_ModuleRegistry* registry, uint32_t module_id)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    idcu_RegisteredModule* mod = NULL;
    for (uint32_t i = 0; i < registry->count; i++) {
        if (registry->modules[i].module_id == module_id) {
            mod = &registry->modules[i];
            break;
        }
    }
    if (!mod) {
        idcu_mutex_unlock(&registry->lock);
        return IDCU_ERR_NOT_FOUND;
    }
    if (mod->state == IDCU_MOD_STATE_RUNNING) {
        idcu_mutex_unlock(&registry->lock);
        return IDCU_ERR_SUCCESS;
    }
    if (mod->state == IDCU_MOD_STATE_UNINIT) {
        idcu_mutex_unlock(&registry->lock);
        int init_ret = idcu_module_registry_init_module(registry, module_id);
        if (init_ret != IDCU_ERR_SUCCESS) {
            return init_ret;
        }
        ret = idcu_mutex_lock(&registry->lock);
        if (ret != IDCU_ERR_SUCCESS) {
            return ret;
        }
    }
    int result = IDCU_ERR_SUCCESS;
    if (mod->iface->run) {
        result = mod->iface->run();
        if (result != IDCU_ERR_SUCCESS) {
            mod->state = IDCU_MOD_STATE_ERROR;
            idcu_mutex_unlock(&registry->lock);
            return result;
        }
    }
    mod->state = IDCU_MOD_STATE_RUNNING;
    idcu_mutex_unlock(&registry->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_module_registry_stop_module(idcu_ModuleRegistry* registry, uint32_t module_id)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    idcu_RegisteredModule* mod = NULL;
    for (uint32_t i = 0; i < registry->count; i++) {
        if (registry->modules[i].module_id == module_id) {
            mod = &registry->modules[i];
            break;
        }
    }
    if (!mod) {
        idcu_mutex_unlock(&registry->lock);
        return IDCU_ERR_NOT_FOUND;
    }
    if (mod->state != IDCU_MOD_STATE_RUNNING) {
        idcu_mutex_unlock(&registry->lock);
        return IDCU_ERR_SUCCESS;
    }
    int result = IDCU_ERR_SUCCESS;
    if (mod->iface->stop) {
        result = mod->iface->stop();
    }
    mod->state = IDCU_MOD_STATE_STOPPED;
    idcu_mutex_unlock(&registry->lock);
    return result;
}

int idcu_module_registry_init_all(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    for (uint32_t i = 0; i < registry->count; i++) {
        idcu_RegisteredModule* mod = &registry->modules[i];
        int ret = idcu_module_registry_init_module(registry, mod->module_id);
        if (ret != IDCU_ERR_SUCCESS) {
            return ret;
        }
    }
    return IDCU_ERR_SUCCESS;
}

int idcu_module_registry_run_all(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    for (int prio = IDCU_MOD_PRIO_REALTIME; prio >= IDCU_MOD_PRIO_LOW; prio--) {
        for (uint32_t i = 0; i < registry->count; i++) {
            idcu_RegisteredModule* mod = &registry->modules[i];
            if (mod->priority == (idcu_ModulePrio)prio) {
                int ret = idcu_module_registry_run_module(registry, mod->module_id);
                if (ret != IDCU_ERR_SUCCESS) {
                    return ret;
                }
            }
        }
    }
    return IDCU_ERR_SUCCESS;
}

int idcu_module_registry_stop_all(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    for (int prio = IDCU_MOD_PRIO_LOW; prio <= IDCU_MOD_PRIO_REALTIME; prio++) {
        for (uint32_t i = 0; i < registry->count; i++) {
            idcu_RegisteredModule* mod = &registry->modules[i];
            if (mod->priority == (idcu_ModulePrio)prio && mod->state == IDCU_MOD_STATE_RUNNING) {
                idcu_module_registry_stop_module(registry, mod->module_id);
            }
        }
    }
    return IDCU_ERR_SUCCESS;
}
