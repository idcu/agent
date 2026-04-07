#ifndef IDCU_KERNEL_MICRO_KERNEL_H
#define IDCU_KERNEL_MICRO_KERNEL_H

#include "coroutine.h"
#include "msg_bus.h"
#include "module_def.h"
#include "module_registry.h"
#include "dynamic_module.h"
#include "module_dependency.h"
#include "module_sandbox.h"
#include "idcu/watchdog/watchdog.h"
#include <stdint.h>

#define IDCU_DEFAULT_MODULE_CAPACITY 16
#define IDCU_CONFIG_FILE_MAX 256
#define IDCU_MODULE_HEALTH_CHECK_INTERVAL_MS 1000
#define IDCU_MODULE_MAX_RESTARTS 5
#define IDCU_MODULE_RESTART_WINDOW_MS 60000

typedef enum {
    IDCU_MODULE_HEALTH_UNKNOWN = 0,
    IDCU_MODULE_HEALTH_HEALTHY,
    IDCU_MODULE_HEALTH_DEGRADED,
    IDCU_MODULE_HEALTH_CRITICAL
} idcu_ModuleHealthStatus;

typedef struct {
    uint64_t last_checkin;
    uint32_t restart_count;
    uint64_t restart_window_start;
    idcu_ModuleHealthStatus health;
    int enable_auto_restart;
    uint32_t health_check_interval_ms;
} idcu_ModuleHealthMonitor;

typedef struct {
    const idcu_ModuleInterface* iface;
    idcu_ModuleState state;
    int is_dynamic;
    idcu_ModuleSandboxContext* sandbox;
    idcu_SandboxResourceLimits sandbox_limits;
    idcu_ModuleHealthMonitor health_monitor;
} idcu_TrackedModule;

typedef void (*idcu_ModuleHealthCallback)(const char* module_name, idcu_ModuleHealthStatus health, void* user_data);

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
    idcu_ModuleDependencyGraph* dep_graph;
    char config_file[IDCU_CONFIG_FILE_MAX];
    idcu_Watchdog      watchdog;
    uint64_t           last_health_check;
    idcu_ModuleHealthCallback health_callback;
    void*              health_callback_data;
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

idcu_ModuleDependencyGraph* idcu_kernel_get_dependency_graph(idcu_MicroKernel *k);
int idcu_kernel_resolve_dependencies(idcu_MicroKernel *k, const char* module_name,
                                      idcu_ModuleDependencyNode** out_order, int* out_count);

int idcu_kernel_set_module_sandbox_limits(idcu_MicroKernel *k, const char* module_name,
                                           const idcu_SandboxResourceLimits* limits);
idcu_ModuleSandboxContext* idcu_kernel_get_module_sandbox(idcu_MicroKernel *k, const char* module_name);

idcu_MicroKernel* idcu_get_kernel(void);

int idcu_kernel_module_checkin(idcu_MicroKernel *k, const char* module_name);
int idcu_kernel_set_module_auto_restart(idcu_MicroKernel *k, const char* module_name, int enable);
idcu_ModuleHealthStatus idcu_kernel_get_module_health(idcu_MicroKernel *k, const char* module_name);
int idcu_kernel_restart_module(idcu_MicroKernel *k, const char* module_name);
int idcu_kernel_set_health_callback(idcu_MicroKernel *k, idcu_ModuleHealthCallback callback, void* user_data);
void idcu_kernel_health_check(idcu_MicroKernel *k);

#endif // IDCU_KERNEL_MICRO_KERNEL_H
