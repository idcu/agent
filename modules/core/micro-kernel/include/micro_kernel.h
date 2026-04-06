#ifndef IDCU_KERNEL_MICRO_KERNEL_H
#define IDCU_KERNEL_MICRO_KERNEL_H

#include "coroutine.h"
#include "msg_bus.h"
#include "module_def.h"
#include "module_registry.h"
#include "dynamic_module.h"

#define IDCU_DEFAULT_MODULE_CAPACITY 16
#define IDCU_CONFIG_FILE_MAX 256

typedef struct {
    const idcu_ModuleInterface* iface;
    idcu_ModuleState state;
    int is_dynamic;
} idcu_TrackedModule;

typedef struct {
    idcu_CoroScheduler coro;
    idcu_MessageBus    msg;
    idcu_StackContext  global;
    volatile int       should_exit;
    idcu_TrackedModule* tracked_modules;
    uint32_t           tracked_cnt;
    uint32_t           tracked_capacity;
    idcu_DynamicLoader dynamic_loader;
    idcu_ModuleRegistry* registry;
    char config_file[IDCU_CONFIG_FILE_MAX];
} idcu_MicroKernel;

void idcu_kernel_init(idcu_MicroKernel *k);
void idcu_kernel_start_modules(idcu_MicroKernel *k);
void idcu_kernel_run(idcu_MicroKernel *k);
void idcu_kernel_stop(idcu_MicroKernel *k);
void idcu_kernel_set_signal_handler(idcu_MicroKernel *k);

int idcu_kernel_hotplug_load(idcu_MicroKernel *k, const char* name, const char* path);
int idcu_kernel_hotplug_unload(idcu_MicroKernel *k, const char* name);
int idcu_kernel_hotplug_restart(idcu_MicroKernel *k, const char* name);

int idcu_kernel_set_config_file(idcu_MicroKernel *k, const char* config_file);
int idcu_kernel_load_config(idcu_MicroKernel *k);

idcu_MicroKernel* idcu_get_kernel(void);

#endif // IDCU_KERNEL_MICRO_KERNEL_H
