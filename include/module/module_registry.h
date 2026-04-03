#ifndef MODULE_REGISTRY_H
#define MODULE_REGISTRY_H

#include "module/module_def.h"
#include "common/error_code.h"
#include "common/lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_REGISTERED_MODULES 64

typedef struct {
    const ModuleInterface* iface;
    uint32_t module_id;
    ModuleState state;
    ModulePrio priority;
    void* user_data;
} RegisteredModule;

typedef struct {
    RegisteredModule modules[MAX_REGISTERED_MODULES];
    uint32_t count;
    uint32_t next_id;
    Mutex lock;
} ModuleRegistry;

int module_registry_init(ModuleRegistry* registry);
void module_registry_destroy(ModuleRegistry* registry);
int module_registry_register(ModuleRegistry* registry, const ModuleInterface* iface, ModulePrio priority);
int module_registry_unregister(ModuleRegistry* registry, uint32_t module_id);
const RegisteredModule* module_registry_find_by_id(ModuleRegistry* registry, uint32_t module_id);
const RegisteredModule* module_registry_find_by_name(ModuleRegistry* registry, const char* name);
int module_registry_get_count(ModuleRegistry* registry);
const RegisteredModule* module_registry_get_at(ModuleRegistry* registry, uint32_t index);
int module_registry_init_module(ModuleRegistry* registry, uint32_t module_id);
int module_registry_run_module(ModuleRegistry* registry, uint32_t module_id);
int module_registry_stop_module(ModuleRegistry* registry, uint32_t module_id);
int module_registry_init_all(ModuleRegistry* registry);
int module_registry_run_all(ModuleRegistry* registry);
int module_registry_stop_all(ModuleRegistry* registry);

#ifdef __cplusplus
}
#endif

#endif
