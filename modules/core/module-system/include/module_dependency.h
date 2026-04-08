#ifndef IDCU_MODULE_MODULE_DEPENDENCY_H
#define IDCU_MODULE_MODULE_DEPENDENCY_H

#include "module_def.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define IDCU_MAX_MODULES_GRAPH 64

    typedef enum
    {
        IDCU_DEP_COMPAT_EXACT = 0,
        IDCU_DEP_COMPAT_GTE   = 1,
        IDCU_DEP_COMPAT_LTE   = 2,
        IDCU_DEP_COMPAT_RANGE = 3
    } idcu_DependencyCompatMode;

    typedef struct
    {
        char                      module_name[IDCU_MODULE_NAME_MAX];
        idcu_ModuleVersion        required_version;
        idcu_ModuleVersion        max_version;
        idcu_DependencyCompatMode compat_mode;
        bool                      optional;
    } idcu_ModuleDependency;

    typedef struct
    {
        char                         module_name[IDCU_MODULE_NAME_MAX];
        idcu_ModuleVersion           version;
        const idcu_ModuleDependency* dependencies;
        int                          dependency_count;
    } idcu_ModuleDependencyNode;

    typedef struct idcu_module_dependency_graph idcu_ModuleDependencyGraph;

    idcu_ModuleDependencyGraph* idcu_module_dependency_graph_create(void);
    void idcu_module_dependency_graph_destroy(idcu_ModuleDependencyGraph* graph);

    int idcu_module_dependency_graph_add_module(idcu_ModuleDependencyGraph*  graph,
                                                const char*                  module_name,
                                                const idcu_ModuleVersion*    version,
                                                const idcu_ModuleDependency* dependencies,
                                                int                          dependency_count);

    int idcu_module_dependency_graph_remove_module(idcu_ModuleDependencyGraph* graph,
                                                   const char*                 module_name);

    int idcu_module_dependency_graph_resolve(idcu_ModuleDependencyGraph* graph,
                                             const char*                 module_name,
                                             idcu_ModuleDependencyNode** out_order, int* out_count);

    int idcu_module_dependency_graph_check_cycles(idcu_ModuleDependencyGraph* graph);

    int idcu_module_dependency_graph_check_versions(idcu_ModuleDependencyGraph* graph);

    bool idcu_module_version_compare(const idcu_ModuleVersion* a, const idcu_ModuleVersion* b);

    int idcu_module_version_check_compat(const idcu_ModuleVersion* current,
                                         const idcu_ModuleVersion* required,
                                         const idcu_ModuleVersion* max_version,
                                         idcu_DependencyCompatMode mode);

    int idcu_module_dependency_parse_from_string(const char* str, idcu_ModuleDependency* out_dep);

#ifdef __cplusplus
}
#endif

#endif
