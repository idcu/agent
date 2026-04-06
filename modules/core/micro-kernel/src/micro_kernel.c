#include "micro_kernel.h"
#include "module_registry.h"
#include "idcu/log/log.h"
#include "idcu/common/error_code.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

static idcu_MicroKernel *g_kernel = NULL;
static idcu_ModuleRegistry g_module_registry;

static int idcu_kernel_resize_tracked(idcu_MicroKernel *k, uint32_t new_capacity)
{
    if (new_capacity <= k->tracked_capacity) {
        return IDCU_ERR_SUCCESS;
    }
    
    idcu_TrackedModule* new_tracked = (idcu_TrackedModule*)realloc(k->tracked_modules, new_capacity * sizeof(idcu_TrackedModule));
    if (!new_tracked) {
        IDCU_LOG_ERROR("Failed to resize tracked modules array");
        return IDCU_ERR_NO_MEMORY;
    }
    
    memset(&new_tracked[k->tracked_capacity], 0, (new_capacity - k->tracked_capacity) * sizeof(idcu_TrackedModule));
    k->tracked_modules = new_tracked;
    k->tracked_capacity = new_capacity;
    
    return IDCU_ERR_SUCCESS;
}

#ifdef _WIN32
static BOOL WINAPI windows_signal_handler(DWORD fdwCtrlType)
{
    if (fdwCtrlType == CTRL_C_EVENT) {
        IDCU_LOG_INFO("received Ctrl+C, initiating graceful shutdown...");
        if (g_kernel) {
            g_kernel->should_exit = 1;
        }
        return TRUE;
    }
    return FALSE;
}
#else
static void unix_signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM) {
        IDCU_LOG_INFO("received signal %d, initiating graceful shutdown...", sig);
        if (g_kernel) {
            g_kernel->should_exit = 1;
        }
    }
}
#endif

void idcu_kernel_init(idcu_MicroKernel *k)
{
    IDCU_LOG_INFO("initializing micro kernel...");
    memset(k, 0, sizeof(idcu_MicroKernel));
    
    k->tracked_capacity = IDCU_DEFAULT_MODULE_CAPACITY;
    k->tracked_modules = (idcu_TrackedModule*)calloc(k->tracked_capacity, sizeof(idcu_TrackedModule));
    if (!k->tracked_modules) {
        IDCU_LOG_ERROR("Failed to allocate tracked modules array");
        return;
    }
    
    strncpy(k->config_file, "config/agent.cfg", IDCU_CONFIG_FILE_MAX - 1);
    idcu_msg_bus_init(&k->msg);
    idcu_ctx_init(&k->global, 0, 0);
    
    int ret = idcu_dynamic_loader_init(&k->dynamic_loader, NULL);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to init dynamic loader, error code: %d (%s)", ret, idcu_err_to_str(ret));
        free(k->tracked_modules);
        k->tracked_modules = NULL;
        return;
    }
    
    ret = idcu_module_registry_init(&g_module_registry);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to init module registry, error code: %d (%s)", ret, idcu_err_to_str(ret));
        idcu_dynamic_loader_destroy(&k->dynamic_loader);
        free(k->tracked_modules);
        k->tracked_modules = NULL;
        return;
    }
    
    k->registry = &g_module_registry;
    
    ret = idcu_module_registry_discover_modules(&g_module_registry);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to discover modules, error code: %d (%s)", ret, idcu_err_to_str(ret));
    }
    
    g_kernel = k;
    IDCU_LOG_INFO("micro kernel initialized successfully");
}

int idcu_kernel_set_config_file(idcu_MicroKernel *k, const char* config_file)
{
    if (!k || !config_file) {
        return IDCU_ERR_INVALID_PARAM;
    }
    strncpy(k->config_file, config_file, sizeof(k->config_file) - 1);
    IDCU_LOG_INFO("config file set to: %s", config_file);
    return IDCU_ERR_SUCCESS;
}

int idcu_kernel_load_config(idcu_MicroKernel *k)
{
    if (!k) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    IDCU_LOG_INFO("loading config from: %s", k->config_file);
    
    int ret = idcu_module_registry_load_config(&g_module_registry, k->config_file);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("failed to load module config, continuing with defaults");
    }
    
    ret = idcu_module_registry_apply_config(&g_module_registry);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("failed to apply module config, continuing with defaults");
    }
    
    return IDCU_ERR_SUCCESS;
}

void idcu_kernel_start_modules(idcu_MicroKernel *k)
{
    uint32_t cnt = 0;
    
    IDCU_LOG_INFO("loading module configuration...");
    int ret = idcu_kernel_load_config(k);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("failed to load config, using defaults");
    }
    
    IDCU_LOG_INFO("starting to initialize modules...");
    ret = idcu_module_registry_init_all(&g_module_registry);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to init modules, error code: %d (%s)", ret, idcu_err_to_str(ret));
        return;
    }
    IDCU_LOG_INFO("modules initialized successfully");
    
    IDCU_LOG_INFO("starting to run modules...");
    ret = idcu_module_registry_run_all(&g_module_registry);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to run modules, error code: %d (%s)", ret, idcu_err_to_str(ret));
        return;
    }
    IDCU_LOG_INFO("modules running successfully");
    
    int module_count = idcu_module_registry_get_count(&g_module_registry);
    
    if ((uint32_t)module_count > k->tracked_capacity) {
        ret = idcu_kernel_resize_tracked(k, (uint32_t)module_count);
        if (ret != IDCU_ERR_SUCCESS) {
            IDCU_LOG_ERROR("Failed to resize tracked modules");
            return;
        }
    }
    
    for (int i = 0; i < module_count; i++) {
        const idcu_RegisteredModule* reg_mod = idcu_module_registry_get_at(&g_module_registry, i);
        if (!reg_mod) {
            IDCU_LOG_WARN("skipping invalid module at index %d", i);
            continue;
        }
        
        if (!reg_mod->enabled) {
            IDCU_LOG_INFO("skipping disabled module: %s", reg_mod->iface->name);
            continue;
        }
        
        const idcu_ModuleInterface* mod = reg_mod->iface;
        IDCU_LOG_INFO("module %s started", mod->name);
        
        k->tracked_modules[cnt].iface = mod;
        k->tracked_modules[cnt].state = IDCU_MOD_STATE_RUNNING;
        cnt++;
    }
    k->tracked_cnt = cnt;
    IDCU_LOG_INFO("tracked %d modules", cnt);
}

void idcu_kernel_stop(idcu_MicroKernel *k)
{
    IDCU_LOG_INFO("stopping all modules...");
    
    int ret = idcu_module_registry_stop_all(&g_module_registry);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("some modules failed to stop, error code: %d (%s)", ret, idcu_err_to_str(ret));
    }
    
    for (uint32_t i = 0; i < k->tracked_cnt; i++) {
        idcu_TrackedModule *tracked = &k->tracked_modules[i];
        tracked->state = IDCU_MOD_STATE_STOPPED;
    }
    
    idcu_dynamic_loader_destroy(&k->dynamic_loader);
    
    if (k->tracked_modules) {
        free(k->tracked_modules);
        k->tracked_modules = NULL;
        k->tracked_capacity = 0;
    }
    
    IDCU_LOG_INFO("all modules stopped");
}

int idcu_kernel_hotplug_load(idcu_MicroKernel *k, const char* name, const char* path)
{
    if (!k || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    IDCU_LOG_INFO("Kernel hotplug loading module: %s", name);
    
    int ret = idcu_dynamic_loader_hotplug_load(&k->dynamic_loader, name, path);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&k->dynamic_loader, name);
    if (!mod) {
        return IDCU_ERR_GENERAL;
    }
    
    if (k->tracked_cnt >= k->tracked_capacity) {
        uint32_t new_capacity = k->tracked_capacity * 2;
        if (new_capacity < IDCU_DEFAULT_MODULE_CAPACITY) {
            new_capacity = IDCU_DEFAULT_MODULE_CAPACITY;
        }
        ret = idcu_kernel_resize_tracked(k, new_capacity);
        if (ret != IDCU_ERR_SUCCESS) {
            IDCU_LOG_ERROR("Failed to resize tracked modules for hotplug");
            return ret;
        }
    }
    
    uint32_t idx = k->tracked_cnt;
    k->tracked_modules[idx].iface = mod->iface;
    k->tracked_modules[idx].state = mod->state;
    k->tracked_modules[idx].is_dynamic = 1;
    
    k->tracked_cnt++;
    
    IDCU_LOG_INFO("Dynamic module %s added to kernel tracking", name);
    return IDCU_ERR_SUCCESS;
}

int idcu_kernel_hotplug_unload(idcu_MicroKernel *k, const char* name)
{
    if (!k || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    IDCU_LOG_INFO("Kernel hotplug unloading module: %s", name);
    
    int found_idx = -1;
    for (uint32_t i = 0; i < k->tracked_cnt; i++) {
        if (k->tracked_modules[i].is_dynamic && 
            strcmp(k->tracked_modules[i].iface->name, name) == 0) {
            found_idx = (int)i;
            break;
        }
    }
    
    if (found_idx >= 0) {
        k->tracked_modules[found_idx].state = IDCU_MOD_STATE_STOPPED;
        
        if ((uint32_t)found_idx < k->tracked_cnt - 1) {
            memmove(&k->tracked_modules[found_idx], &k->tracked_modules[found_idx + 1],
                    (k->tracked_cnt - found_idx - 1) * sizeof(idcu_TrackedModule));
        }
        k->tracked_cnt--;
    }
    
    return idcu_dynamic_loader_hotplug_unload(&k->dynamic_loader, name);
}

int idcu_kernel_hotplug_restart(idcu_MicroKernel *k, const char* name)
{
    if (!k || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    IDCU_LOG_INFO("Kernel hotplug restarting module: %s", name);
    
    int ret = idcu_dynamic_module_restart(&k->dynamic_loader, name);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&k->dynamic_loader, name);
    if (mod) {
        for (uint32_t i = 0; i < k->tracked_cnt; i++) {
            if (k->tracked_modules[i].is_dynamic && 
                strcmp(k->tracked_modules[i].iface->name, name) == 0) {
                k->tracked_modules[i].state = mod->state;
                break;
            }
        }
    }
    
    return IDCU_ERR_SUCCESS;
}

void idcu_kernel_run(idcu_MicroKernel *k)
{
    while (!k->should_exit) {
        idcu_coro_sched_run(&k->coro);
        
        for (uint32_t i = 0; i < k->tracked_cnt; ++i) {
            idcu_TrackedModule *tracked = &k->tracked_modules[i];
            if (tracked->state == IDCU_MOD_STATE_RUNNING && tracked->iface->run) {
                tracked->iface->run();
            }
        }
    }
    
    idcu_kernel_stop(k);
}

void idcu_kernel_set_signal_handler(idcu_MicroKernel *k)
{
    (void)k;
#ifdef _WIN32
    if (!SetConsoleCtrlHandler(windows_signal_handler, TRUE)) {
        IDCU_LOG_WARN("failed to set Windows console handler");
    }
#else
    struct sigaction sa;
    sa.sa_handler = unix_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGINT, &sa, NULL) != 0) {
        IDCU_LOG_WARN("failed to set SIGINT handler");
    }
    if (sigaction(SIGTERM, &sa, NULL) != 0) {
        IDCU_LOG_WARN("failed to set SIGTERM handler");
    }
#endif
}

idcu_MicroKernel* idcu_get_kernel(void)
{
    return g_kernel;
}
