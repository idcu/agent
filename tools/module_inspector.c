#include "module_inspector.h"
#include <idcu/common/vector.h>
#include <idcu/module/module.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static idcu_Vector module_infos;
static int initialized = 0;

const char* idcu_module_state_to_string(idcu_ModuleState state) {
    switch (state) {
        case IDCU_MODULE_STATE_UNKNOWN: return "UNKNOWN";
        case IDCU_MODULE_STATE_UNLOADED: return "UNLOADED";
        case IDCU_MODULE_STATE_LOADED: return "LOADED";
        case IDCU_MODULE_STATE_INITIALIZED: return "INITIALIZED";
        case IDCU_MODULE_STATE_STARTING: return "STARTING";
        case IDCU_MODULE_STATE_RUNNING: return "RUNNING";
        case IDCU_MODULE_STATE_STOPPING: return "STOPPING";
        case IDCU_MODULE_STATE_STOPPED: return "STOPPED";
        case IDCU_MODULE_STATE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

int idcu_module_inspector_init(void) {
    if (initialized) {
        idcu_module_inspector_destroy();
    }
    
    idcu_vector_init(&module_infos, sizeof(idcu_ModuleInfo));
    initialized = 1;
    
    return 0;
}

void idcu_module_inspector_destroy(void) {
    if (!initialized) {
        return;
    }
    
    for (size_t i = 0; i < module_infos.size; i++) {
        idcu_ModuleInfo* info = (idcu_ModuleInfo*)idcu_vector_get(&module_infos, i);
        if (info->dependencies) {
            free((void*)info->dependencies);
        }
    }
    
    idcu_vector_destroy(&module_infos);
    initialized = 0;
}

size_t idcu_module_inspector_get_module_count(void) {
    if (!initialized) {
        return 0;
    }
    return module_infos.size;
}

int idcu_module_inspector_get_module(size_t index, idcu_ModuleInfo* info) {
    if (!initialized || !info || index >= module_infos.size) {
        return -1;
    }
    
    idcu_ModuleInfo* src = (idcu_ModuleInfo*)idcu_vector_get(&module_infos, index);
    *info = *src;
    return 0;
}

int idcu_module_inspector_find_module(const char* name, idcu_ModuleInfo* info) {
    if (!initialized || !name || !info) {
        return -1;
    }
    
    for (size_t i = 0; i < module_infos.size; i++) {
        idcu_ModuleInfo* src = (idcu_ModuleInfo*)idcu_vector_get(&module_infos, i);
        if (strcmp(src->name, name) == 0) {
            *info = *src;
            return 0;
        }
    }
    
    return -1;
}

void idcu_module_inspector_get_stats(idcu_ModuleSystemStats* stats) {
    if (!initialized || !stats) {
        return;
    }
    
    memset(stats, 0, sizeof(*stats));
    stats->total_modules = module_infos.size;
    
    for (size_t i = 0; i < module_infos.size; i++) {
        idcu_ModuleInfo* info = (idcu_ModuleInfo*)idcu_vector_get(&module_infos, i);
        
        switch (info->state) {
            case IDCU_MODULE_STATE_LOADED:
                stats->loaded_modules++;
                break;
            case IDCU_MODULE_STATE_INITIALIZED:
            case IDCU_MODULE_STATE_STARTING:
            case IDCU_MODULE_STATE_RUNNING:
                stats->loaded_modules++;
                stats->running_modules++;
                break;
            case IDCU_MODULE_STATE_ERROR:
                stats->error_modules++;
                break;
            default:
                break;
        }
    }
}

void idcu_module_inspector_print_stats(void) {
    idcu_ModuleSystemStats stats;
    idcu_module_inspector_get_stats(&stats);
    
    printf("=== Module System Statistics ===\n");
    printf("Total modules:     %zu\n", stats.total_modules);
    printf("Loaded modules:    %zu\n", stats.loaded_modules);
    printf("Running modules:   %zu\n", stats.running_modules);
    printf("Error modules:     %zu\n", stats.error_modules);
    printf("================================\n");
}

void idcu_module_inspector_print_all(void) {
    printf("=== All Modules ===\n");
    
    for (size_t i = 0; i < module_infos.size; i++) {
        idcu_ModuleInfo* info = (idcu_ModuleInfo*)idcu_vector_get(&module_infos, i);
        
        printf("\nModule #%zu:\n", i + 1);
        printf("  Name:        %s\n", info->name ? info->name : "(unknown)");
        printf("  Version:     %s\n", info->version ? info->version : "(unknown)");
        printf("  State:       %s\n", idcu_module_state_to_string(info->state));
        printf("  Category:    %s\n", info->category ? info->category : "(unknown)");
        printf("  Description: %s\n", info->description ? info->description : "(none)");
        
        if (info->uptime_ms > 0) {
            printf("  Uptime:      %" PRIu64 " ms\n", info->uptime_ms);
        }
        
        if (info->dependency_count > 0) {
            printf("  Dependencies:");
            for (size_t j = 0; j < info->dependency_count; j++) {
                printf(" %s", info->dependencies[j]);
            }
            printf("\n");
        }
    }
    
    printf("\n===================\n");
}

void idcu_module_inspector_print_module(const char* name) {
    idcu_ModuleInfo info;
    if (idcu_module_inspector_find_module(name, &info) != 0) {
        printf("Module '%s' not found\n", name);
        return;
    }
    
    printf("=== Module: %s ===\n", name);
    printf("Version:     %s\n", info.version ? info.version : "(unknown)");
    printf("State:       %s\n", idcu_module_state_to_string(info.state));
    printf("Category:    %s\n", info.category ? info.category : "(unknown)");
    printf("Description: %s\n", info.description ? info.description : "(none)");
    
    if (info.uptime_ms > 0) {
        printf("Uptime:      %" PRIu64 " ms\n", info.uptime_ms);
    }
    
    if (info.dependency_count > 0) {
        printf("Dependencies:");
        for (size_t j = 0; j < info.dependency_count; j++) {
            printf(" %s", info.dependencies[j]);
        }
        printf("\n");
    }
    
    printf("====================\n");
}
