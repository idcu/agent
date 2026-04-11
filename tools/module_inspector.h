#ifndef IDCU_TOOLS_MODULE_INSPECTOR_H
#define IDCU_TOOLS_MODULE_INSPECTOR_H

#include <idcu/common/config.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_MODULE_STATE_UNKNOWN,
    IDCU_MODULE_STATE_UNLOADED,
    IDCU_MODULE_STATE_LOADED,
    IDCU_MODULE_STATE_INITIALIZED,
    IDCU_MODULE_STATE_STARTING,
    IDCU_MODULE_STATE_RUNNING,
    IDCU_MODULE_STATE_STOPPING,
    IDCU_MODULE_STATE_STOPPED,
    IDCU_MODULE_STATE_ERROR
} idcu_ModuleState;

typedef struct {
    const char* name;
    const char* version;
    const char* description;
    const char* category;
    idcu_ModuleState state;
    uint64_t init_time;
    uint64_t start_time;
    uint64_t uptime_ms;
    size_t dependency_count;
    const char** dependencies;
} idcu_ModuleInfo;

typedef struct {
    size_t total_modules;
    size_t loaded_modules;
    size_t running_modules;
    size_t error_modules;
} idcu_ModuleSystemStats;

int idcu_module_inspector_init(void);
void idcu_module_inspector_destroy(void);

size_t idcu_module_inspector_get_module_count(void);
int idcu_module_inspector_get_module(size_t index, idcu_ModuleInfo* info);
int idcu_module_inspector_find_module(const char* name, idcu_ModuleInfo* info);

void idcu_module_inspector_get_stats(idcu_ModuleSystemStats* stats);
void idcu_module_inspector_print_stats(void);
void idcu_module_inspector_print_all(void);
void idcu_module_inspector_print_module(const char* name);

const char* idcu_module_state_to_string(idcu_ModuleState state);

#ifdef __cplusplus
}
#endif

#endif
