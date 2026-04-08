#ifndef IDCU_MODULE_MODULE_REGISTRY_H
#define IDCU_MODULE_MODULE_REGISTRY_H

#include "idcu/common/error_code.h"
#include "idcu/common/lock.h"
#include "module_category.h"
#include "module_def.h"
#include "module_version.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif  // IDCU_MODULE_MODULE_REGISTRY_H

#define IDCU_MAX_REGISTERED_MODULES 64

    typedef struct
    {
        const idcu_ModuleInterface* iface;
        uint32_t                    module_id;
        idcu_ModuleState            state;
        idcu_ModulePrio             priority;
        void*                       user_data;
        uint32_t                    in_degree;
        uint32_t                    adjacency[IDCU_MAX_REGISTERED_MODULES];
        uint32_t                    adjacency_count;
        int                         enabled;
    } idcu_RegisteredModule;

    typedef struct
    {
        idcu_RegisteredModule      modules[IDCU_MAX_REGISTERED_MODULES];
        uint32_t                   count;
        uint32_t                   next_id;
        idcu_Mutex                 lock;
        uint32_t                   topological_order[IDCU_MAX_REGISTERED_MODULES];
        uint32_t                   topological_count;
        int                        topological_valid;
        idcu_ModuleCategoryManager category_mgr;
        int                        use_config;
    } idcu_ModuleRegistry;

    int  idcu_module_registry_init(idcu_ModuleRegistry* registry);
    void idcu_module_registry_destroy(idcu_ModuleRegistry* registry);
    int  idcu_module_registry_register(idcu_ModuleRegistry*        registry,
                                       const idcu_ModuleInterface* iface, idcu_ModulePrio priority);
    int  idcu_module_registry_unregister(idcu_ModuleRegistry* registry, uint32_t module_id);
    const idcu_RegisteredModule* idcu_module_registry_find_by_id(idcu_ModuleRegistry* registry,
                                                                 uint32_t             module_id);
    const idcu_RegisteredModule* idcu_module_registry_find_by_name(idcu_ModuleRegistry* registry,
                                                                   const char*          name);
    int                          idcu_module_registry_get_count(idcu_ModuleRegistry* registry);
    const idcu_RegisteredModule* idcu_module_registry_get_at(idcu_ModuleRegistry* registry,
                                                             uint32_t             index);
    int idcu_module_registry_init_module(idcu_ModuleRegistry* registry, uint32_t module_id);
    int idcu_module_registry_run_module(idcu_ModuleRegistry* registry, uint32_t module_id);
    int idcu_module_registry_stop_module(idcu_ModuleRegistry* registry, uint32_t module_id);
    int idcu_module_registry_init_all(idcu_ModuleRegistry* registry);
    int idcu_module_registry_run_all(idcu_ModuleRegistry* registry);
    int idcu_module_registry_stop_all(idcu_ModuleRegistry* registry);

    int idcu_module_registry_discover_modules(idcu_ModuleRegistry* registry);
    int idcu_module_registry_build_dependency_graph(idcu_ModuleRegistry* registry);
    int idcu_module_registry_topological_sort(idcu_ModuleRegistry* registry);

    int idcu_module_registry_load_config(idcu_ModuleRegistry* registry, const char* config_file);
    int idcu_module_registry_apply_config(idcu_ModuleRegistry* registry);

    // 版本管理相关函数
    int idcu_module_registry_check_dependency_versions(idcu_ModuleRegistry* registry);
    int idcu_module_registry_get_module_version(idcu_ModuleRegistry* registry,
                                                const char*          module_name,
                                                idcu_ModuleVersion*  out_version);
    int idcu_module_registry_find_module_by_version(idcu_ModuleRegistry*          registry,
                                                    const char*                   module_name,
                                                    const idcu_ModuleVersion*     min_version,
                                                    const idcu_ModuleVersion*     max_version,
                                                    const idcu_RegisteredModule** out_module);

#ifdef __cplusplus
}
#endif  // IDCU_MODULE_MODULE_REGISTRY_H

#endif  // IDCU_MODULE_MODULE_REGISTRY_H
