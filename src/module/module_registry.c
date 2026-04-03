#include "module/module_registry.h"
#include <string.h>

#if defined(__GNUC__) && !defined(__MINGW32__)
extern const idcu_ModuleInterface __start_modules;
extern const idcu_ModuleInterface __stop_modules;
#endif

extern const idcu_ModuleInterface __idcu_module_base_log;
extern const idcu_ModuleInterface __idcu_module_biz_collect;
extern const idcu_ModuleInterface __idcu_module_core_module;

int idcu_module_registry_discover_modules(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret;
#if defined(__GNUC__) && !defined(__MINGW32__)
    const idcu_ModuleInterface* mod = &__start_modules;
    while (mod < &__stop_modules) {
        if (mod->name != NULL) {
            ret = idcu_module_registry_register(registry, mod, IDCU_MOD_PRIO_NORMAL);
            if (ret != IDCU_ERR_SUCCESS && ret != IDCU_ERR_ALREADY_EXISTS) {
                return ret;
            }
        }
        mod++;
    }
#else
    ret = idcu_module_registry_register(registry, &__idcu_module_base_log, IDCU_MOD_PRIO_NORMAL);
    if (ret != IDCU_ERR_SUCCESS && ret != IDCU_ERR_ALREADY_EXISTS) {
        return ret;
    }
    ret = idcu_module_registry_register(registry, &__idcu_module_biz_collect, IDCU_MOD_PRIO_NORMAL);
    if (ret != IDCU_ERR_SUCCESS && ret != IDCU_ERR_ALREADY_EXISTS) {
        return ret;
    }
    ret = idcu_module_registry_register(registry, &__idcu_module_core_module, IDCU_MOD_PRIO_NORMAL);
    if (ret != IDCU_ERR_SUCCESS && ret != IDCU_ERR_ALREADY_EXISTS) {
        return ret;
    }
#endif
    
    ret = idcu_module_registry_build_dependency_graph(registry);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    ret = idcu_module_registry_topological_sort(registry);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    return IDCU_ERR_SUCCESS;
}

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
    registry->topological_valid = 0;
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
    mod->in_degree = 0;
    mod->adjacency_count = 0;
    registry->count++;
    registry->topological_valid = 0;
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
    registry->topological_valid = 0;
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

int idcu_module_registry_build_dependency_graph(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    for (uint32_t i = 0; i < registry->count; i++) {
        registry->modules[i].in_degree = 0;
        registry->modules[i].adjacency_count = 0;
    }
    
    for (uint32_t i = 0; i < registry->count; i++) {
        idcu_RegisteredModule* mod = &registry->modules[i];
        
        if (!mod->iface->dependencies || mod->iface->dependency_count == 0) {
            continue;
        }
        
        for (int d = 0; d < mod->iface->dependency_count; d++) {
            const char* dep_name = mod->iface->dependencies[d].dependency_name;
            int dep_found = 0;
            
            for (uint32_t j = 0; j < registry->count; j++) {
                if (strcmp(registry->modules[j].iface->name, dep_name) == 0) {
                    idcu_RegisteredModule* dep_mod = &registry->modules[j];
                    
                    if (dep_mod->adjacency_count < IDCU_MAX_REGISTERED_MODULES) {
                        dep_mod->adjacency[dep_mod->adjacency_count++] = i;
                    }
                    
                    mod->in_degree++;
                    dep_found = 1;
                    break;
                }
            }
            
            if (!dep_found) {
                idcu_mutex_unlock(&registry->lock);
                return IDCU_ERR_NOT_FOUND;
            }
        }
    }
    
    idcu_mutex_unlock(&registry->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_module_registry_topological_sort(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    uint32_t temp_in_degree[IDCU_MAX_REGISTERED_MODULES];
    for (uint32_t i = 0; i < registry->count; i++) {
        temp_in_degree[i] = registry->modules[i].in_degree;
    }
    
    uint32_t queue[IDCU_MAX_REGISTERED_MODULES];
    uint32_t queue_front = 0;
    uint32_t queue_back = 0;
    
    for (uint32_t i = 0; i < registry->count; i++) {
        if (temp_in_degree[i] == 0) {
            queue[queue_back++] = i;
        }
    }
    
    uint32_t topological_idx = 0;
    
    while (queue_front < queue_back) {
        uint32_t u = queue[queue_front++];
        registry->topological_order[topological_idx++] = registry->modules[u].module_id;
        
        idcu_RegisteredModule* mod = &registry->modules[u];
        for (uint32_t i = 0; i < mod->adjacency_count; i++) {
            uint32_t v = mod->adjacency[i];
            temp_in_degree[v]--;
            
            if (temp_in_degree[v] == 0) {
                queue[queue_back++] = v;
            }
        }
    }
    
    if (topological_idx != registry->count) {
        idcu_mutex_unlock(&registry->lock);
        return IDCU_ERR_CIRCULAR_DEP;
    }
    
    registry->topological_count = topological_idx;
    registry->topological_valid = 1;
    
    idcu_mutex_unlock(&registry->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_module_registry_init_all(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    if (!registry->topological_valid) {
        idcu_mutex_unlock(&registry->lock);
        ret = idcu_module_registry_build_dependency_graph(registry);
        if (ret != IDCU_ERR_SUCCESS) {
            return ret;
        }
        ret = idcu_module_registry_topological_sort(registry);
        if (ret != IDCU_ERR_SUCCESS) {
            return ret;
        }
        ret = idcu_mutex_lock(&registry->lock);
        if (ret != IDCU_ERR_SUCCESS) {
            return ret;
        }
    }
    
    idcu_mutex_unlock(&registry->lock);
    
    for (uint32_t i = 0; i < registry->topological_count; i++) {
        uint32_t module_id = registry->topological_order[i];
        int init_ret = idcu_module_registry_init_module(registry, module_id);
        if (init_ret != IDCU_ERR_SUCCESS) {
            return init_ret;
        }
    }
    
    return IDCU_ERR_SUCCESS;
}

int idcu_module_registry_run_all(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    if (!registry->topological_valid) {
        idcu_mutex_unlock(&registry->lock);
        ret = idcu_module_registry_build_dependency_graph(registry);
        if (ret != IDCU_ERR_SUCCESS) {
            return ret;
        }
        ret = idcu_module_registry_topological_sort(registry);
        if (ret != IDCU_ERR_SUCCESS) {
            return ret;
        }
        ret = idcu_mutex_lock(&registry->lock);
        if (ret != IDCU_ERR_SUCCESS) {
            return ret;
        }
    }
    
    idcu_mutex_unlock(&registry->lock);
    
    for (uint32_t i = 0; i < registry->topological_count; i++) {
        uint32_t module_id = registry->topological_order[i];
        int run_ret = idcu_module_registry_run_module(registry, module_id);
        if (run_ret != IDCU_ERR_SUCCESS) {
            return run_ret;
        }
    }
    
    return IDCU_ERR_SUCCESS;
}

int idcu_module_registry_stop_all(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    if (!registry->topological_valid) {
        idcu_mutex_unlock(&registry->lock);
        ret = idcu_module_registry_build_dependency_graph(registry);
        if (ret != IDCU_ERR_SUCCESS) {
            return ret;
        }
        ret = idcu_module_registry_topological_sort(registry);
        if (ret != IDCU_ERR_SUCCESS) {
            return ret;
        }
        ret = idcu_mutex_lock(&registry->lock);
        if (ret != IDCU_ERR_SUCCESS) {
            return ret;
        }
    }
    
    idcu_mutex_unlock(&registry->lock);
    
    for (int i = (int)registry->topological_count - 1; i >= 0; i--) {
        uint32_t module_id = registry->topological_order[i];
        idcu_module_registry_stop_module(registry, module_id);
    }
    
    return IDCU_ERR_SUCCESS;
}
