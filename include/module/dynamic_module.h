#ifndef DYNAMIC_MODULE_H
#define DYNAMIC_MODULE_H

#include "module/module_def.h"
#include "common/error_code.h"
#include "common/lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DYNAMIC_MODULES 32
#define MODULE_PATH_MAX 256

#ifdef _WIN32
#include <windows.h>
typedef HMODULE ModuleHandle;
#else
#include <dlfcn.h>
typedef void* ModuleHandle;
#endif

typedef struct {
    char name[64];
    char path[MODULE_PATH_MAX];
    ModuleHandle handle;
    ModuleInterface* iface;
    ModuleState state;
    uint32_t ref_count;
} DynamicModule;

typedef struct {
    DynamicModule modules[MAX_DYNAMIC_MODULES];
    uint32_t count;
    Mutex lock;
    char module_path[MODULE_PATH_MAX];
} DynamicLoader;

int dynamic_loader_init(DynamicLoader* loader, const char* module_path);
void dynamic_loader_destroy(DynamicLoader* loader);
int dynamic_loader_load_module(DynamicLoader* loader, const char* name, const char* path);
int dynamic_loader_unload_module(DynamicLoader* loader, const char* name);
DynamicModule* dynamic_loader_find_module(DynamicLoader* loader, const char* name);
DynamicModule* dynamic_loader_get_at(DynamicLoader* loader, uint32_t index);
int dynamic_loader_get_count(DynamicLoader* loader);
int dynamic_module_init(DynamicModule* mod);
int dynamic_module_run(DynamicModule* mod);
int dynamic_module_stop(DynamicModule* mod);

#ifdef __cplusplus
}
#endif

#endif
