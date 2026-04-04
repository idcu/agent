#include "module_registry.h"
#include "module_version.h"
#include "log.h"
#include "error_code.h"
#include <string.h>

#if defined(__GNUC__) && !defined(__MINGW32__)
extern const idcu_ModuleInterface __start_modules;
extern const idcu_ModuleInterface __stop_modules;
#endif

extern const idcu_ModuleInterface __idcu_module_log_module;
extern const idcu_ModuleInterface __idcu_module_collect_module;
extern const idcu_ModuleInterface __idcu_module_core_module;
extern const idcu_ModuleInterface __idcu_module_heartbeat_module;
extern const idcu_ModuleInterface __idcu_module_healthcheck_module;
extern const idcu_ModuleInterface __idcu_module_metrics_module;
extern const idcu_ModuleInterface __idcu_module_alert_module;

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
    registry->use_config = 0;
    
    ret = idcu_module_category_manager_init(&registry->category_mgr);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to init category manager");
        idcu_mutex_destroy(&registry->lock);
        return ret;
    }
    
    return IDCU_ERR_SUCCESS;
}

void idcu_module_registry_destroy(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        return;
    }
    idcu_module_registry_stop_all(registry);
    idcu_module_category_manager_destroy(&registry->category_mgr);
    idcu_mutex_destroy(&registry->lock);
    memset(registry, 0, sizeof(idcu_ModuleRegistry));
}

int idcu_module_registry_register(idcu_ModuleRegistry* registry, const idcu_ModuleInterface* iface, idcu_ModulePrio priority)
{
    if (!registry || !iface || !iface->name) {
        IDCU_LOG_ERROR("module_registry_register failed: invalid parameters");
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("module_registry_register failed: lock error, code=%d (%s)", ret, idcu_err_to_str(ret));
        return ret;
    }
    
    for (uint32_t i = 0; i < registry->count; i++) {
        if (strcmp(registry->modules[i].iface->name, iface->name) == 0) {
            idcu_mutex_unlock(&registry->lock);
            IDCU_LOG_WARN("module_registry_register: module %s already exists", iface->name);
            return IDCU_ERR_ALREADY_EXISTS;
        }
    }
    
    if (registry->count >= IDCU_MAX_REGISTERED_MODULES) {
        idcu_mutex_unlock(&registry->lock);
        IDCU_LOG_ERROR("module_registry_register failed: module registry full (max=%d)", IDCU_MAX_REGISTERED_MODULES);
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
    mod->enabled = 1;
    registry->count++;
    registry->topological_valid = 0;
    idcu_mutex_unlock(&registry->lock);
    
    IDCU_LOG_INFO("module registered: %s (id=%u)", iface->name, mod->module_id);
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
        IDCU_LOG_ERROR("module_registry_init_module failed: invalid parameters");
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("module_registry_init_module failed: lock error, code=%d (%s)", ret, idcu_err_to_str(ret));
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
        IDCU_LOG_ERROR("module_registry_init_module failed: module id=%u not found", module_id);
        return IDCU_ERR_NOT_FOUND;
    }
    
    if (mod->state >= IDCU_MOD_STATE_INITED) {
        idcu_mutex_unlock(&registry->lock);
        IDCU_LOG_DEBUG("module %s already initialized", mod->iface->name);
        return IDCU_ERR_SUCCESS;
    }
    
    int result = IDCU_ERR_SUCCESS;
    if (mod->iface->init) {
        IDCU_LOG_INFO("initializing module %s...", mod->iface->name);
        result = mod->iface->init();
        if (result != IDCU_ERR_SUCCESS) {
            mod->state = IDCU_MOD_STATE_ERROR;
            idcu_mutex_unlock(&registry->lock);
            IDCU_LOG_ERROR("module %s init failed, code=%d (%s)", mod->iface->name, result, idcu_err_to_str(result));
            return result;
        }
        IDCU_LOG_INFO("module %s initialized successfully", mod->iface->name);
    }
    
    mod->state = IDCU_MOD_STATE_INITED;
    idcu_mutex_unlock(&registry->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_module_registry_run_module(idcu_ModuleRegistry* registry, uint32_t module_id)
{
    if (!registry) {
        IDCU_LOG_ERROR("module_registry_run_module failed: invalid parameters");
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("module_registry_run_module failed: lock error, code=%d (%s)", ret, idcu_err_to_str(ret));
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
        IDCU_LOG_ERROR("module_registry_run_module failed: module id=%u not found", module_id);
        return IDCU_ERR_NOT_FOUND;
    }
    
    if (mod->state == IDCU_MOD_STATE_RUNNING) {
        idcu_mutex_unlock(&registry->lock);
        IDCU_LOG_DEBUG("module %s already running", mod->iface->name);
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
            IDCU_LOG_ERROR("module_registry_run_module failed: lock error after init, code=%d (%s)", ret, idcu_err_to_str(ret));
            return ret;
        }
    }
    
    int result = IDCU_ERR_SUCCESS;
    if (mod->iface->run) {
        IDCU_LOG_INFO("running module %s...", mod->iface->name);
        result = mod->iface->run();
        if (result != IDCU_ERR_SUCCESS) {
            mod->state = IDCU_MOD_STATE_ERROR;
            idcu_mutex_unlock(&registry->lock);
            IDCU_LOG_ERROR("module %s run failed, code=%d (%s)", mod->iface->name, result, idcu_err_to_str(result));
            return result;
        }
        IDCU_LOG_INFO("module %s running successfully", mod->iface->name);
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
    ret = idcu_module_registry_register(registry, &__idcu_module_log_module, IDCU_MOD_PRIO_NORMAL);
    if (ret != IDCU_ERR_SUCCESS && ret != IDCU_ERR_ALREADY_EXISTS) {
        return ret;
    }
    ret = idcu_module_registry_register(registry, &__idcu_module_collect_module, IDCU_MOD_PRIO_NORMAL);
    if (ret != IDCU_ERR_SUCCESS && ret != IDCU_ERR_ALREADY_EXISTS) {
        return ret;
    }
    ret = idcu_module_registry_register(registry, &__idcu_module_core_module, IDCU_MOD_PRIO_NORMAL);
    if (ret != IDCU_ERR_SUCCESS && ret != IDCU_ERR_ALREADY_EXISTS) {
        return ret;
    }
    ret = idcu_module_registry_register(registry, &__idcu_module_heartbeat_module, IDCU_MOD_PRIO_NORMAL);
    if (ret != IDCU_ERR_SUCCESS && ret != IDCU_ERR_ALREADY_EXISTS) {
        return ret;
    }
    ret = idcu_module_registry_register(registry, &__idcu_module_healthcheck_module, IDCU_MOD_PRIO_NORMAL);
    if (ret != IDCU_ERR_SUCCESS && ret != IDCU_ERR_ALREADY_EXISTS) {
        return ret;
    }
    ret = idcu_module_registry_register(registry, &__idcu_module_metrics_module, IDCU_MOD_PRIO_NORMAL);
    if (ret != IDCU_ERR_SUCCESS && ret != IDCU_ERR_ALREADY_EXISTS) {
        return ret;
    }
    ret = idcu_module_registry_register(registry, &__idcu_module_alert_module, IDCU_MOD_PRIO_NORMAL);
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

int idcu_module_registry_build_dependency_graph(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        IDCU_LOG_ERROR("module_registry_build_dependency_graph failed: invalid parameters");
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("module_registry_build_dependency_graph failed: lock error, code=%d (%s)", ret, idcu_err_to_str(ret));
        return ret;
    }
    
    IDCU_LOG_INFO("building dependency graph for %u modules...", registry->count);
    
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
                    IDCU_LOG_DEBUG("dependency: %s -> %s", mod->iface->name, dep_name);
                    break;
                }
            }
            
            if (!dep_found) {
                idcu_mutex_unlock(&registry->lock);
                IDCU_LOG_ERROR("module_registry_build_dependency_graph failed: dependency %s not found for module %s", dep_name, mod->iface->name);
                return IDCU_ERR_NOT_FOUND;
            }
        }
    }
    
    idcu_mutex_unlock(&registry->lock);
    IDCU_LOG_INFO("dependency graph built successfully");
    return IDCU_ERR_SUCCESS;
}

int idcu_module_registry_topological_sort(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        IDCU_LOG_ERROR("module_registry_topological_sort failed: invalid parameters");
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("module_registry_topological_sort failed: lock error, code=%d (%s)", ret, idcu_err_to_str(ret));
        return ret;
    }
    
    IDCU_LOG_INFO("performing topological sort...");
    
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
        IDCU_LOG_ERROR("module_registry_topological_sort failed: circular dependency detected");
        return IDCU_ERR_CIRCULAR_DEP;
    }
    
    registry->topological_count = topological_idx;
    registry->topological_valid = 1;
    
    idcu_mutex_unlock(&registry->lock);
    IDCU_LOG_INFO("topological sort completed, order count=%u", topological_idx);
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
        
        const idcu_RegisteredModule* reg_mod = idcu_module_registry_find_by_id(registry, module_id);
        if (reg_mod && !reg_mod->enabled) {
            IDCU_LOG_INFO("skipping disabled module: %s", reg_mod->iface->name);
            continue;
        }
        
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
        
        const idcu_RegisteredModule* reg_mod = idcu_module_registry_find_by_id(registry, module_id);
        if (reg_mod && !reg_mod->enabled) {
            IDCU_LOG_INFO("skipping disabled module: %s", reg_mod->iface->name);
            continue;
        }
        
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

int idcu_module_registry_check_dependency_versions(idcu_ModuleRegistry* registry)
{
    if (!registry) {
        IDCU_LOG_ERROR("module_registry_check_dependency_versions failed: invalid parameters");
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("module_registry_check_dependency_versions failed: lock error, code=%d (%s)", ret, idcu_err_to_str(ret));
        return ret;
    }
    
    IDCU_LOG_INFO("checking dependency versions for %u modules...", registry->count);
    
    for (uint32_t i = 0; i < registry->count; i++) {
        idcu_RegisteredModule* mod = &registry->modules[i];
        
        if (!mod->iface->dependencies || mod->iface->dependency_count == 0) {
            continue;
        }
        
        for (int d = 0; d < mod->iface->dependency_count; d++) {
            const idcu_ModuleDependency* dep = &mod->iface->dependencies[d];
            int dep_found = 0;
            
            for (uint32_t j = 0; j < registry->count; j++) {
                if (strcmp(registry->modules[j].iface->name, dep->dependency_name) == 0) {
                    idcu_RegisteredModule* dep_mod = &registry->modules[j];
                    
                    int version_check = idcu_version_check_dependency(&dep_mod->iface->version, dep);
                    if (version_check != IDCU_SUCCESS) {
                        char ver_str[IDCU_VERSION_STR_MAX];
                        char req_min_str[IDCU_VERSION_STR_MAX];
                        idcu_version_format(&dep_mod->iface->version, ver_str, sizeof(ver_str));
                        idcu_version_format(&dep->min_version, req_min_str, sizeof(req_min_str));
                        
                        idcu_mutex_unlock(&registry->lock);
                        IDCU_LOG_ERROR("version check failed for %s -> %s: actual=%s, required min=%s, error=%s",
                                       mod->iface->name, dep->dependency_name, ver_str, req_min_str,
                                       idcu_err_to_str(version_check));
                        return version_check;
                    }
                    
                    dep_found = 1;
                    IDCU_LOG_DEBUG("version check passed: %s -> %s", mod->iface->name, dep->dependency_name);
                    break;
                }
            }
            
            if (!dep_found) {
                idcu_mutex_unlock(&registry->lock);
                IDCU_LOG_ERROR("module_registry_check_dependency_versions failed: dependency %s not found for module %s",
                               dep->dependency_name, mod->iface->name);
                return IDCU_ERR_NOT_FOUND;
            }
        }
    }
    
    idcu_mutex_unlock(&registry->lock);
    IDCU_LOG_INFO("all dependency versions checked successfully");
    return IDCU_SUCCESS;
}

int idcu_module_registry_get_module_version(idcu_ModuleRegistry* registry, const char* module_name, idcu_ModuleVersion* out_version)
{
    if (!registry || !module_name || !out_version) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    int found = 0;
    for (uint32_t i = 0; i < registry->count; i++) {
        if (strcmp(registry->modules[i].iface->name, module_name) == 0) {
            *out_version = registry->modules[i].iface->version;
            found = 1;
            break;
        }
    }
    
    idcu_mutex_unlock(&registry->lock);
    return found ? IDCU_SUCCESS : IDCU_ERR_NOT_FOUND;
}

int idcu_module_registry_find_module_by_version(idcu_ModuleRegistry* registry, const char* module_name,
                                                 const idcu_ModuleVersion* min_version, const idcu_ModuleVersion* max_version,
                                                 const idcu_RegisteredModule** out_module)
{
    if (!registry || !module_name || !out_module) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    *out_module = NULL;
    const idcu_RegisteredModule* best_match = NULL;
    
    for (uint32_t i = 0; i < registry->count; i++) {
        if (strcmp(registry->modules[i].iface->name, module_name) == 0) {
            const idcu_ModuleVersion* ver = &registry->modules[i].iface->version;
            int compatible = 1;
            
            if (min_version) {
                if (idcu_version_compare(ver, min_version) == IDCU_VERSION_LESS) {
                    compatible = 0;
                }
            }
            
            if (max_version && compatible) {
                if (idcu_version_compare(ver, max_version) == IDCU_VERSION_GREATER) {
                    compatible = 0;
                }
            }
            
            if (compatible) {
                if (!best_match || idcu_version_compare(ver, &best_match->iface->version) == IDCU_VERSION_GREATER) {
                    best_match = &registry->modules[i];
                }
            }
        }
    }
    
    *out_module = best_match;
    idcu_mutex_unlock(&registry->lock);
    return best_match ? IDCU_SUCCESS : IDCU_ERR_NOT_FOUND;
}

int idcu_module_registry_load_config(idcu_ModuleRegistry* registry, const char* config_file)
{
    if (!registry || !config_file) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    IDCU_LOG_INFO("loading module config from: %s", config_file);
    
    int ret = idcu_module_category_load_config(&registry->category_mgr, config_file);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("failed to load category config, continuing anyway");
    }
    
    registry->use_config = 1;
    return IDCU_ERR_SUCCESS;
}

int idcu_module_registry_apply_config(idcu_ModuleRegistry* registry)
{
    if (!registry || !registry->use_config) {
        return IDCU_ERR_SUCCESS;
    }
    
    IDCU_LOG_INFO("applying module configuration...");
    
    int ret = idcu_mutex_lock(&registry->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    for (uint32_t i = 0; i < registry->count; i++) {
        idcu_RegisteredModule* mod = &registry->modules[i];
        
        int enabled = idcu_module_category_is_module_enabled(&registry->category_mgr, mod->iface->name);
        mod->enabled = enabled;
        
        idcu_ModulePrio prio = idcu_module_category_get_module_priority(&registry->category_mgr, mod->iface->name);
        mod->priority = prio;
        
        IDCU_LOG_INFO("module %s: enabled=%d, priority=%d", mod->iface->name, enabled, prio);
    }
    
    registry->topological_valid = 0;
    idcu_mutex_unlock(&registry->lock);
    
    ret = idcu_module_registry_build_dependency_graph(registry);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    ret = idcu_module_registry_topological_sort(registry);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    IDCU_LOG_INFO("module configuration applied successfully");
    return IDCU_ERR_SUCCESS;
}
