#ifndef IDCU_MODULE_DYNAMIC_MODULE_H
#define IDCU_MODULE_DYNAMIC_MODULE_H

#include "module_def.h"
#include "idcu/common/error_code.h"
#include "idcu/common/lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // IDCU_MODULE_DYNAMIC_MODULE_H

#define IDCU_MAX_DYNAMIC_MODULES 32
#define IDCU_MODULE_PATH_MAX 256

#ifdef _WIN32
#include <windows.h>
typedef HMODULE idcu_ModuleHandle;
#else
#include <dlfcn.h>
typedef void* idcu_ModuleHandle;
#endif // IDCU_MODULE_DYNAMIC_MODULE_H

typedef struct {
    char name[64];
    char path[IDCU_MODULE_PATH_MAX];
    idcu_ModuleHandle handle;
    idcu_ModuleInterface* iface;
    idcu_ModuleState state;
    uint32_t ref_count;
} idcu_DynamicModule;

typedef struct {
    idcu_DynamicModule modules[IDCU_MAX_DYNAMIC_MODULES];
    uint32_t count;
    idcu_Mutex lock;
    char module_path[IDCU_MODULE_PATH_MAX];
} idcu_DynamicLoader;

int idcu_dynamic_loader_init(idcu_DynamicLoader* loader, const char* module_path);
void idcu_dynamic_loader_destroy(idcu_DynamicLoader* loader);
int idcu_dynamic_loader_load_module(idcu_DynamicLoader* loader, const char* name, const char* path);
int idcu_dynamic_loader_unload_module(idcu_DynamicLoader* loader, const char* name);
idcu_DynamicModule* idcu_dynamic_loader_find_module(idcu_DynamicLoader* loader, const char* name);
idcu_DynamicModule* idcu_dynamic_loader_get_at(idcu_DynamicLoader* loader, uint32_t index);
int idcu_dynamic_loader_get_count(idcu_DynamicLoader* loader);
int idcu_dynamic_module_init(idcu_DynamicModule* mod);
int idcu_dynamic_module_run(idcu_DynamicModule* mod);
int idcu_dynamic_module_stop(idcu_DynamicModule* mod);
int idcu_dynamic_module_restart(idcu_DynamicLoader* loader, const char* name);
int idcu_dynamic_module_reload(idcu_DynamicLoader* loader, const char* name, const char* path);
int idcu_dynamic_loader_hotplug_load(idcu_DynamicLoader* loader, const char* name, const char* path);
int idcu_dynamic_loader_hotplug_unload(idcu_DynamicLoader* loader, const char* name);

#ifdef __cplusplus
}
#endif // IDCU_MODULE_DYNAMIC_MODULE_H

#endif // IDCU_MODULE_DYNAMIC_MODULE_H
