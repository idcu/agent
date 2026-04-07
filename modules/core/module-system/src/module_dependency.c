#include "module_dependency.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct idcu_module_dependency_graph_node {
    idcu_ModuleDependencyNode node;
    idcu_ModuleDependency* dependencies_copy;
    int visited;
    int in_stack;
} idcu_ModuleDependencyGraphNode;

struct idcu_module_dependency_graph {
    idcu_ModuleDependencyGraphNode nodes[IDCU_MAX_MODULES_GRAPH];
    int node_count;
};

static int find_module_index(idcu_ModuleDependencyGraph* graph, const char* module_name) {
    for (int i = 0; i < graph->node_count; i++) {
        if (strcmp(graph->nodes[i].node.module_name, module_name) == 0) {
            return i;
        }
    }
    return -1;
}

idcu_ModuleDependencyGraph* idcu_module_dependency_graph_create(void) {
    idcu_ModuleDependencyGraph* graph = (idcu_ModuleDependencyGraph*)malloc(sizeof(idcu_ModuleDependencyGraph));
    if (!graph) {
        return NULL;
    }
    memset(graph, 0, sizeof(idcu_ModuleDependencyGraph));
    return graph;
}

void idcu_module_dependency_graph_destroy(idcu_ModuleDependencyGraph* graph) {
    if (!graph) return;
    
    for (int i = 0; i < graph->node_count; i++) {
        if (graph->nodes[i].dependencies_copy) {
            free(graph->nodes[i].dependencies_copy);
        }
    }
    free(graph);
}

int idcu_module_dependency_graph_add_module(idcu_ModuleDependencyGraph* graph,
                                             const char* module_name,
                                             const idcu_ModuleVersion* version,
                                             const idcu_ModuleDependency* dependencies,
                                             int dependency_count) {
    if (!graph || !module_name || !version) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    if (graph->node_count >= IDCU_MAX_MODULES_GRAPH) {
        return IDCU_ERR_NO_RESOURCE;
    }
    
    if (find_module_index(graph, module_name) >= 0) {
        return IDCU_MOD_ERR_MODULE_ALREADY_LOADED;
    }
    
    idcu_ModuleDependencyGraphNode* new_node = &graph->nodes[graph->node_count];
    memset(new_node, 0, sizeof(idcu_ModuleDependencyGraphNode));
    
    strncpy(new_node->node.module_name, module_name, IDCU_MODULE_NAME_MAX - 1);
    new_node->node.module_name[IDCU_MODULE_NAME_MAX - 1] = '\0';
    memcpy(&new_node->node.version, version, sizeof(idcu_ModuleVersion));
    
    if (dependencies && dependency_count > 0) {
        new_node->dependencies_copy = (idcu_ModuleDependency*)malloc(
            sizeof(idcu_ModuleDependency) * dependency_count);
        if (!new_node->dependencies_copy) {
            return IDCU_ERR_NO_MEMORY;
        }
        memcpy(new_node->dependencies_copy, dependencies, 
               sizeof(idcu_ModuleDependency) * dependency_count);
        new_node->node.dependencies = new_node->dependencies_copy;
        new_node->node.dependency_count = dependency_count;
    }
    
    graph->node_count++;
    return IDCU_ERR_OK;
}

int idcu_module_dependency_graph_remove_module(idcu_ModuleDependencyGraph* graph,
                                                const char* module_name) {
    if (!graph || !module_name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int index = find_module_index(graph, module_name);
    if (index < 0) {
        return IDCU_MOD_ERR_MODULE_NOT_FOUND;
    }
    
    if (graph->nodes[index].dependencies_copy) {
        free(graph->nodes[index].dependencies_copy);
    }
    
    for (int i = index; i < graph->node_count - 1; i++) {
        graph->nodes[i] = graph->nodes[i + 1];
    }
    
    graph->node_count--;
    return IDCU_ERR_OK;
}

static int topological_sort_visit(idcu_ModuleDependencyGraph* graph, 
                                  int node_index,
                                  idcu_ModuleDependencyNode** out_order,
                                  int* out_count) {
    idcu_ModuleDependencyGraphNode* node = &graph->nodes[node_index];
    
    if (node->in_stack) {
        return IDCU_MOD_ERR_DEPENDENCY_CYCLE;
    }
    
    if (node->visited) {
        return IDCU_ERR_OK;
    }
    
    node->visited = 1;
    node->in_stack = 1;
    
    for (int i = 0; i < node->node.dependency_count; i++) {
        const idcu_ModuleDependency* dep = &node->node.dependencies[i];
        int dep_index = find_module_index(graph, dep->module_name);
        
        if (dep_index < 0) {
            if (!dep->optional) {
                return IDCU_MOD_ERR_DEPENDENCY_MISSING;
            }
            continue;
        }
        
        int ret = topological_sort_visit(graph, dep_index, out_order, out_count);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
    }
    
    node->in_stack = 0;
    out_order[*out_count] = &node->node;
    (*out_count)++;
    
    return IDCU_ERR_OK;
}

int idcu_module_dependency_graph_resolve(idcu_ModuleDependencyGraph* graph,
                                          const char* module_name,
                                          idcu_ModuleDependencyNode** out_order,
                                          int* out_count) {
    if (!graph || !module_name || !out_order || !out_count) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int start_index = find_module_index(graph, module_name);
    if (start_index < 0) {
        return IDCU_MOD_ERR_MODULE_NOT_FOUND;
    }
    
    for (int i = 0; i < graph->node_count; i++) {
        graph->nodes[i].visited = 0;
        graph->nodes[i].in_stack = 0;
    }
    
    *out_count = 0;
    return topological_sort_visit(graph, start_index, out_order, out_count);
}

int idcu_module_dependency_graph_check_cycles(idcu_ModuleDependencyGraph* graph) {
    if (!graph) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    for (int i = 0; i < graph->node_count; i++) {
        graph->nodes[i].visited = 0;
        graph->nodes[i].in_stack = 0;
    }
    
    for (int i = 0; i < graph->node_count; i++) {
        if (!graph->nodes[i].visited) {
            idcu_ModuleDependencyNode* temp_order[IDCU_MAX_MODULES_GRAPH];
            int temp_count = 0;
            int ret = topological_sort_visit(graph, i, temp_order, &temp_count);
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        }
    }
    
    return IDCU_ERR_OK;
}

int idcu_module_dependency_graph_check_versions(idcu_ModuleDependencyGraph* graph) {
    if (!graph) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    for (int i = 0; i < graph->node_count; i++) {
        idcu_ModuleDependencyGraphNode* node = &graph->nodes[i];
        
        for (int j = 0; j < node->node.dependency_count; j++) {
            const idcu_ModuleDependency* dep = &node->node.dependencies[j];
            int dep_index = find_module_index(graph, dep->module_name);
            
            if (dep_index < 0) {
                if (!dep->optional) {
                    return IDCU_MOD_ERR_DEPENDENCY_MISSING;
                }
                continue;
            }
            
            idcu_ModuleVersion* current_version = &graph->nodes[dep_index].node.version;
            int ret = idcu_module_version_check_compat(
                current_version,
                &dep->required_version,
                &dep->max_version,
                dep->compat_mode);
            
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        }
    }
    
    return IDCU_ERR_OK;
}

bool idcu_module_version_compare(const idcu_ModuleVersion* a,
                                  const idcu_ModuleVersion* b) {
    if (!a || !b) return false;
    
    if (a->major != b->major) {
        return a->major < b->major;
    }
    if (a->minor != b->minor) {
        return a->minor < b->minor;
    }
    return a->patch < b->patch;
}

int idcu_module_version_check_compat(const idcu_ModuleVersion* current,
                                      const idcu_ModuleVersion* required,
                                      const idcu_ModuleVersion* max_version,
                                      idcu_DependencyCompatMode mode) {
    if (!current || !required) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    bool current_lt_required = idcu_module_version_compare(current, required);
    
    switch (mode) {
        case IDCU_DEP_COMPAT_EXACT:
            if (current->major != required->major || 
                current->minor != required->minor || 
                current->patch != required->patch) {
                return IDCU_MOD_ERR_DEPENDENCY_VERSION_MISMATCH;
            }
            break;
            
        case IDCU_DEP_COMPAT_GTE:
            if (current_lt_required) {
                return IDCU_MOD_ERR_DEPENDENCY_VERSION_MISMATCH;
            }
            break;
            
        case IDCU_DEP_COMPAT_LTE:
            if (!current_lt_required && 
                (current->major != required->major || 
                 current->minor != required->minor || 
                 current->patch != required->patch)) {
                return IDCU_MOD_ERR_DEPENDENCY_VERSION_MISMATCH;
            }
            break;
            
        case IDCU_DEP_COMPAT_RANGE:
            if (current_lt_required) {
                return IDCU_MOD_ERR_DEPENDENCY_VERSION_MISMATCH;
            }
            if (max_version && !idcu_module_version_compare(current, max_version)) {
                return IDCU_MOD_ERR_DEPENDENCY_VERSION_MISMATCH;
            }
            break;
    }
    
    return IDCU_ERR_OK;
}

int idcu_module_dependency_parse_from_string(const char* str,
                                              idcu_ModuleDependency* out_dep) {
    if (!str || !out_dep) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    memset(out_dep, 0, sizeof(idcu_ModuleDependency));
    
    char temp[256];
    strncpy(temp, str, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    
    char* name_end = strchr(temp, '@');
    if (name_end) {
        *name_end = '\0';
        strncpy(out_dep->module_name, temp, IDCU_MODULE_NAME_MAX - 1);
        out_dep->module_name[IDCU_MODULE_NAME_MAX - 1] = '\0';
        
        char* version_str = name_end + 1;
        int major, minor, patch;
        if (sscanf(version_str, "%d.%d.%d", &major, &minor, &patch) == 3) {
            out_dep->required_version.major = (uint16_t)major;
            out_dep->required_version.minor = (uint16_t)minor;
            out_dep->required_version.patch = (uint16_t)patch;
            out_dep->compat_mode = IDCU_DEP_COMPAT_GTE;
        } else {
            return IDCU_MOD_ERR_INVALID_CONFIG;
        }
    } else {
        strncpy(out_dep->module_name, temp, IDCU_MODULE_NAME_MAX - 1);
        out_dep->module_name[IDCU_MODULE_NAME_MAX - 1] = '\0';
        out_dep->required_version.major = 0;
        out_dep->required_version.minor = 0;
        out_dep->required_version.patch = 0;
        out_dep->compat_mode = IDCU_DEP_COMPAT_GTE;
    }
    
    return IDCU_ERR_OK;
}
