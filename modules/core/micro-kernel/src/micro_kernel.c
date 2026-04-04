#include "micro_kernel.h"
#include "module_registry.h"
#include "log.h"
#include "error_code.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

static idcu_MicroKernel *g_kernel = NULL;
static idcu_ModuleRegistry g_module_registry;

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
    strncpy(k->config_file, "config/agent.cfg", sizeof(k->config_file) - 1);
    idcu_msg_bus_init(&k->msg);
    idcu_ctx_init(&k->global, 0, 0);
    
    int ret = idcu_health_monitor_init(&k->health_monitor);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to init health monitor, error code: %d (%s)", ret, idcu_err_to_str(ret));
        return;
    }
    
    k->health_check_interval_ms = 5000;
    k->last_health_check_ms = idcu_health_get_uptime_ms();
    
    ret = idcu_dynamic_loader_init(&k->dynamic_loader, NULL);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to init dynamic loader, error code: %d (%s)", ret, idcu_err_to_str(ret));
        return;
    }
    
    ret = idcu_module_registry_init(&g_module_registry);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to init module registry, error code: %d (%s)", ret, idcu_err_to_str(ret));
        return;
    }
    
    k->registry = &g_module_registry;
    
    ret = idcu_module_registry_discover_modules(&g_module_registry);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to discover modules, error code: %d (%s)", ret, idcu_err_to_str(ret));
        return;
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
    for (int i = 0; i < module_count && cnt < 16; i++) {
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
        
        uint32_t health_module_id = cnt;
        ret = idcu_health_register_module(&k->health_monitor, health_module_id);
        if (ret != IDCU_ERR_SUCCESS) {
            IDCU_LOG_WARN("failed to register module %s with health monitor: %d", mod->name, ret);
        }
        
        k->sandbox[cnt].module_id = cnt;
        k->sandbox[cnt].perm = IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN;
        
        k->tracked_modules[cnt].iface = mod;
        k->tracked_modules[cnt].state = IDCU_MOD_STATE_RUNNING;
        k->tracked_modules[cnt].health_module_id = health_module_id;
        
        cnt++;
    }
    k->sb_cnt = cnt;
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
        idcu_health_unregister_module(&k->health_monitor, tracked->health_module_id);
    }
    
    idcu_dynamic_loader_destroy(&k->dynamic_loader);
    idcu_health_monitor_destroy(&k->health_monitor);
    
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
    
    if (k->tracked_cnt >= 16) {
        IDCU_LOG_WARN("No space left in tracked modules");
        return IDCU_ERR_QUEUE_FULL;
    }
    
    uint32_t idx = k->tracked_cnt;
    k->tracked_modules[idx].iface = mod->iface;
    k->tracked_modules[idx].state = mod->state;
    k->tracked_modules[idx].is_dynamic = 1;
    
    k->sandbox[idx].module_id = idx;
    k->sandbox[idx].perm = IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN;
    
    k->tracked_cnt++;
    k->sb_cnt++;
    
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
            memmove(&k->sandbox[found_idx], &k->sandbox[found_idx + 1],
                    (k->sb_cnt - found_idx - 1) * sizeof(idcu_Sandbox));
        }
        k->tracked_cnt--;
        k->sb_cnt--;
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

        idcu_Message msg;
        for (uint32_t i = 0; i < k->sb_cnt; ++i) {
            idcu_Sandbox *sb = &k->sandbox[i];
            if (idcu_sandbox_perm_check(sb, IDCU_PERM_RECV) == 0) {
                idcu_msg_recv(&k->msg, i, &msg);
            }
        }

        for (uint32_t i = 0; i < k->tracked_cnt; ++i) {
            idcu_TrackedModule *tracked = &k->tracked_modules[i];
            if (tracked->state == IDCU_MOD_STATE_RUNNING && tracked->iface->run) {
                tracked->iface->run();
                idcu_health_update_heartbeat(&k->health_monitor, tracked->health_module_id);
            }
        }

        uint64_t now = idcu_health_get_uptime_ms();
        if (now - k->last_health_check_ms >= k->health_check_interval_ms) {
            idcu_health_check_all(&k->health_monitor);
            
            idcu_HealthSummary summary;
            idcu_health_get_summary(&k->health_monitor, &summary);
            IDCU_LOG_INFO("Health check - Total: %u, Healthy: %u, Warning: %u, Critical: %u, Dead: %u",
                          summary.total_count, summary.healthy_count, summary.warning_count,
                          summary.critical_count, summary.dead_count);
            
            k->last_health_check_ms = now;
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

idcu_HealthMonitor* idcu_kernel_get_health_monitor(idcu_MicroKernel *k)
{
    if (!k) {
        return NULL;
    }
    return &k->health_monitor;
}

idcu_HealthStatus idcu_kernel_get_overall_health(idcu_MicroKernel *k)
{
    if (!k) {
        return IDCU_HEALTH_UNKNOWN;
    }
    return idcu_health_get_overall_status(&k->health_monitor);
}

void idcu_kernel_get_health_summary(idcu_MicroKernel *k, idcu_HealthSummary* summary)
{
    if (!k || !summary) {
        return;
    }
    idcu_health_get_summary(&k->health_monitor, summary);
}

int idcu_kernel_update_module_heartbeat(idcu_MicroKernel *k, uint32_t module_idx)
{
    if (!k || module_idx >= k->tracked_cnt) {
        return IDCU_ERR_INVALID_PARAM;
    }
    return idcu_health_update_heartbeat(&k->health_monitor, k->tracked_modules[module_idx].health_module_id);
}

int idcu_kernel_report_module_error(idcu_MicroKernel *k, uint32_t module_idx)
{
    if (!k || module_idx >= k->tracked_cnt) {
        return IDCU_ERR_INVALID_PARAM;
    }
    return idcu_health_report_error(&k->health_monitor, k->tracked_modules[module_idx].health_module_id);
}

void idcu_kernel_set_health_check_interval(idcu_MicroKernel *k, uint64_t interval_ms)
{
    if (k) {
        k->health_check_interval_ms = interval_ms;
    }
}
