#ifndef IDCU_KERNEL_MICRO_KERNEL_H
#define IDCU_KERNEL_MICRO_KERNEL_H

#include "scheduler/coroutine.h"
#include "scheduler/msg_bus.h"
#include "security/sandbox.h"
#include "module/module_def.h"
#include "module/dynamic_module.h"

typedef struct {
    const idcu_ModuleInterface* iface;
    idcu_ModuleState state;
    int is_dynamic;
} idcu_TrackedModule;

typedef struct {
    idcu_CoroScheduler coro;
    idcu_MessageBus    msg;
    idcu_Sandbox       sandbox[16];
    uint32_t           sb_cnt;
    idcu_StackContext  global;
    volatile int       should_exit;
    idcu_TrackedModule tracked_modules[16];
    uint32_t           tracked_cnt;
    idcu_DynamicLoader dynamic_loader;
} idcu_MicroKernel;

void idcu_kernel_init(idcu_MicroKernel *k);
void idcu_kernel_start_modules(idcu_MicroKernel *k);
void idcu_kernel_run(idcu_MicroKernel *k);
void idcu_kernel_stop(idcu_MicroKernel *k);
void idcu_kernel_set_signal_handler(idcu_MicroKernel *k);

int idcu_kernel_hotplug_load(idcu_MicroKernel *k, const char* name, const char* path);
int idcu_kernel_hotplug_unload(idcu_MicroKernel *k, const char* name);
int idcu_kernel_hotplug_restart(idcu_MicroKernel *k, const char* name);

#endif // IDCU_KERNEL_MICRO_KERNEL_H
