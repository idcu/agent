#include "micro_kernel.h"
#include "idcu/common/error_code.h"
#include "idcu/log/log.h"
#include "module_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <time.h>
#include <unistd.h>
#endif

static uint64_t get_current_time_ms(void) {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

static idcu_MicroKernel *g_kernel = NULL;
static idcu_ModuleRegistry g_module_registry;

static int idcu_kernel_resize_tracked(idcu_MicroKernel *k, uint32_t new_capacity) {
    if (new_capacity <= k->tracked_capacity) {
        return IDCU_ERR_SUCCESS;
    }

    idcu_TrackedModule *new_tracked = (idcu_TrackedModule *)realloc(
        k->tracked_modules, new_capacity * sizeof(idcu_TrackedModule));
    if (!new_tracked) {
        IDCU_LOG_ERROR("Failed to resize tracked modules array");
        return IDCU_ERR_NO_MEMORY;
    }

    memset(&new_tracked[k->tracked_capacity], 0,
           (new_capacity - k->tracked_capacity) * sizeof(idcu_TrackedModule));
    k->tracked_modules = new_tracked;
    k->tracked_capacity = new_capacity;

    return IDCU_ERR_SUCCESS;
}

static void idcu_sandbox_violation_callback(idcu_ModuleSandboxContext *ctx,
                                            idcu_SandboxResourceType resource_type,
                                            const char *violation_details, void *user_data) {
    (void)ctx;
    (void)user_data;
    IDCU_LOG_ERROR("Sandbox violation: type=%d, details=%s", resource_type, violation_details);
}

#ifdef _WIN32
static BOOL WINAPI windows_signal_handler(DWORD fdwCtrlType) {
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
static void unix_signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        IDCU_LOG_INFO("received signal %d, initiating graceful shutdown...", sig);
        if (g_kernel) {
            g_kernel->should_exit = 1;
        }
    }
}
#endif

void idcu_kernel_init(idcu_MicroKernel *k) {
    IDCU_LOG_INFO("initializing micro kernel...");
    memset(k, 0, sizeof(idcu_MicroKernel));

    k->tracked_capacity = IDCU_DEFAULT_MODULE_CAPACITY;
    k->tracked_modules =
        (idcu_TrackedModule *)calloc(k->tracked_capacity, sizeof(idcu_TrackedModule));
    if (!k->tracked_modules) {
        IDCU_LOG_ERROR("Failed to allocate tracked modules array");
        return;
    }

    k->dep_graph = idcu_module_dependency_graph_create();
    if (!k->dep_graph) {
        IDCU_LOG_ERROR("Failed to create dependency graph");
        free(k->tracked_modules);
        k->tracked_modules = NULL;
        return;
    }

    strncpy(k->config_file, "config/agent.cfg", IDCU_CONFIG_FILE_MAX - 1);
    idcu_msg_bus_init(&k->msg);
    idcu_ctx_init(&k->global, 0, 0);

    int ret = idcu_dynamic_loader_init(&k->dynamic_loader, NULL);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to init dynamic loader, error code: %d (%s)", ret,
                       idcu_err_to_str(ret));
        idcu_module_dependency_graph_destroy(k->dep_graph);
        k->dep_graph = NULL;
        free(k->tracked_modules);
        k->tracked_modules = NULL;
        return;
    }

    ret = idcu_module_registry_init(&g_module_registry);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to init module registry, error code: %d (%s)", ret,
                       idcu_err_to_str(ret));
        idcu_dynamic_loader_destroy(&k->dynamic_loader);
        idcu_module_dependency_graph_destroy(k->dep_graph);
        k->dep_graph = NULL;
        free(k->tracked_modules);
        k->tracked_modules = NULL;
        return;
    }

    k->registry = &g_module_registry;

    ret = idcu_module_registry_discover_modules(&g_module_registry);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("failed to discover modules, error code: %d (%s)", ret,
                       idcu_err_to_str(ret));
    }

    ret = idcu_watchdog_init(&k->watchdog, IDCU_WATCHDOG_DEFAULT_TIMEOUT_MS,
                             IDCU_WATCHDOG_MAX_RESTARTS, IDCU_WATCHDOG_RESTART_WINDOW_MS);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_WARN("Failed to initialize watchdog, continuing without it");
    }

    k->last_health_check = get_current_time_ms();

    g_kernel = k;
    IDCU_LOG_INFO("micro kernel initialized successfully");
}

int idcu_kernel_set_config_file(idcu_MicroKernel *k, const char *config_file) {
    if (!k || !config_file) {
        return IDCU_ERR_INVALID_PARAM;
    }
    strncpy(k->config_file, config_file, sizeof(k->config_file) - 1);
    IDCU_LOG_INFO("config file set to: %s", config_file);
    return IDCU_ERR_SUCCESS;
}

int idcu_kernel_load_config(idcu_MicroKernel *k) {
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

void idcu_kernel_start_modules(idcu_MicroKernel *k) {
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
        const idcu_RegisteredModule *reg_mod = idcu_module_registry_get_at(&g_module_registry, i);
        if (!reg_mod) {
            IDCU_LOG_WARN("skipping invalid module at index %d", i);
            continue;
        }

        if (!reg_mod->enabled) {
            IDCU_LOG_INFO("skipping disabled module: %s", reg_mod->iface->name);
            continue;
        }

        const idcu_ModuleInterface *mod = reg_mod->iface;
        IDCU_LOG_INFO("module %s started", mod->name);

        k->tracked_modules[cnt].iface = mod;
        k->tracked_modules[cnt].state = IDCU_MOD_STATE_RUNNING;
        k->tracked_modules[cnt].is_dynamic = 0;

        idcu_module_dependency_graph_add_module(k->dep_graph, mod->name, &mod->version,
                                                mod->dependencies, mod->dependency_count);

        k->tracked_modules[cnt].sandbox = idcu_module_sandbox_create_context();
        if (k->tracked_modules[cnt].sandbox) {
            idcu_module_sandbox_set_module_info(k->tracked_modules[cnt].sandbox, mod->name);
            idcu_module_sandbox_set_violation_callback(k->tracked_modules[cnt].sandbox,
                                                       idcu_sandbox_violation_callback, NULL);
        }

        k->tracked_modules[cnt].health_monitor.last_checkin = get_current_time_ms();
        k->tracked_modules[cnt].health_monitor.restart_count = 0;
        k->tracked_modules[cnt].health_monitor.restart_window_start = get_current_time_ms();
        k->tracked_modules[cnt].health_monitor.health = IDCU_MODULE_HEALTH_HEALTHY;
        k->tracked_modules[cnt].health_monitor.enable_auto_restart = 1;
        k->tracked_modules[cnt].health_monitor.health_check_interval_ms =
            IDCU_MODULE_HEALTH_CHECK_INTERVAL_MS;

        cnt++;
    }
    k->tracked_cnt = cnt;

    ret = idcu_module_dependency_graph_check_cycles(k->dep_graph);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_WARN("Dependency cycle detected: %d", ret);
    }

    ret = idcu_module_dependency_graph_check_versions(k->dep_graph);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_WARN("Dependency version issues detected: %d", ret);
    }

    IDCU_LOG_INFO("tracked %d modules", cnt);
}

void idcu_kernel_stop(idcu_MicroKernel *k) {
    IDCU_LOG_INFO("stopping all modules...");

    idcu_watchdog_stop(&k->watchdog);
    idcu_watchdog_destroy(&k->watchdog);

    int ret = idcu_module_registry_stop_all(&g_module_registry);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("some modules failed to stop, error code: %d (%s)", ret,
                      idcu_err_to_str(ret));
    }

    for (uint32_t i = 0; i < k->tracked_cnt; i++) {
        idcu_TrackedModule *tracked = &k->tracked_modules[i];
        tracked->state = IDCU_MOD_STATE_STOPPED;
        if (tracked->sandbox) {
            idcu_module_sandbox_destroy_context(tracked->sandbox);
            tracked->sandbox = NULL;
        }
    }

    if (k->dep_graph) {
        idcu_module_dependency_graph_destroy(k->dep_graph);
        k->dep_graph = NULL;
    }

    idcu_dynamic_loader_destroy(&k->dynamic_loader);

    if (k->tracked_modules) {
        free(k->tracked_modules);
        k->tracked_modules = NULL;
        k->tracked_capacity = 0;
    }

    IDCU_LOG_INFO("all modules stopped");
}

idcu_ModuleDependencyGraph *idcu_kernel_get_dependency_graph(idcu_MicroKernel *k) {
    if (!k)
        return NULL;
    return k->dep_graph;
}

int idcu_kernel_resolve_dependencies(idcu_MicroKernel *k, const char *module_name,
                                     idcu_ModuleDependencyNode **out_order, int *out_count) {
    if (!k || !module_name || !out_order || !out_count) {
        return IDCU_ERR_INVALID_PARAM;
    }

    return idcu_module_dependency_graph_resolve(k->dep_graph, module_name, out_order, out_count);
}

int idcu_kernel_set_module_sandbox_limits(idcu_MicroKernel *k, const char *module_name,
                                          const idcu_SandboxResourceLimits *limits) {
    if (!k || !module_name || !limits) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (uint32_t i = 0; i < k->tracked_cnt; i++) {
        idcu_TrackedModule *tracked = &k->tracked_modules[i];
        if (strcmp(tracked->iface->name, module_name) == 0) {
            memcpy(&tracked->sandbox_limits, limits, sizeof(idcu_SandboxResourceLimits));
            if (tracked->sandbox) {
                idcu_module_sandbox_set_limits(tracked->sandbox, limits);
            }
            return IDCU_ERR_SUCCESS;
        }
    }

    return IDCU_MOD_ERR_MODULE_NOT_FOUND;
}

idcu_ModuleSandboxContext *idcu_kernel_get_module_sandbox(idcu_MicroKernel *k,
                                                          const char *module_name) {
    if (!k || !module_name)
        return NULL;

    for (uint32_t i = 0; i < k->tracked_cnt; i++) {
        idcu_TrackedModule *tracked = &k->tracked_modules[i];
        if (strcmp(tracked->iface->name, module_name) == 0) {
            return tracked->sandbox;
        }
    }

    return NULL;
}

int idcu_kernel_hotplug_load(idcu_MicroKernel *k, const char *name, const char *path) {
    if (!k || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    IDCU_LOG_INFO("Kernel hotplug loading module: %s", name);

    int ret = idcu_dynamic_loader_hotplug_load(&k->dynamic_loader, name, path);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    idcu_DynamicModule *mod = idcu_dynamic_loader_find_module(&k->dynamic_loader, name);
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

int idcu_kernel_hotplug_unload(idcu_MicroKernel *k, const char *name) {
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

int idcu_kernel_hotplug_restart(idcu_MicroKernel *k, const char *name) {
    if (!k || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    IDCU_LOG_INFO("Kernel hotplug restarting module: %s", name);

    int ret = idcu_dynamic_module_restart(&k->dynamic_loader, name);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    idcu_DynamicModule *mod = idcu_dynamic_loader_find_module(&k->dynamic_loader, name);
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

void idcu_kernel_run(idcu_MicroKernel *k) {
    idcu_watchdog_start(&k->watchdog);

    while (!k->should_exit) {
        idcu_coro_sched_run(&k->coro);

        for (uint32_t i = 0; i < k->tracked_cnt; ++i) {
            idcu_TrackedModule *tracked = &k->tracked_modules[i];
            if (tracked->state == IDCU_MOD_STATE_RUNNING && tracked->iface->run) {
                tracked->iface->run();
            }
        }

        idcu_watchdog_heartbeat(&k->watchdog);
        idcu_kernel_health_check(k);
    }

    idcu_kernel_stop(k);
}

void idcu_kernel_set_signal_handler(idcu_MicroKernel *k) {
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

idcu_MicroKernel *idcu_get_kernel(void) { return g_kernel; }

int idcu_kernel_module_checkin(idcu_MicroKernel *k, const char *module_name) {
    if (!k || !module_name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (uint32_t i = 0; i < k->tracked_cnt; i++) {
        idcu_TrackedModule *tracked = &k->tracked_modules[i];
        if (strcmp(tracked->iface->name, module_name) == 0) {
            tracked->health_monitor.last_checkin = get_current_time_ms();
            if (tracked->health_monitor.health != IDCU_MODULE_HEALTH_HEALTHY) {
                tracked->health_monitor.health = IDCU_MODULE_HEALTH_HEALTHY;
                if (k->health_callback) {
                    k->health_callback(module_name, IDCU_MODULE_HEALTH_HEALTHY,
                                       k->health_callback_data);
                }
            }
            return IDCU_ERR_SUCCESS;
        }
    }

    return IDCU_MOD_ERR_MODULE_NOT_FOUND;
}

int idcu_kernel_set_module_auto_restart(idcu_MicroKernel *k, const char *module_name, int enable) {
    if (!k || !module_name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (uint32_t i = 0; i < k->tracked_cnt; i++) {
        idcu_TrackedModule *tracked = &k->tracked_modules[i];
        if (strcmp(tracked->iface->name, module_name) == 0) {
            tracked->health_monitor.enable_auto_restart = enable;
            IDCU_LOG_INFO("Module %s auto-restart %s", module_name,
                          enable ? "enabled" : "disabled");
            return IDCU_ERR_SUCCESS;
        }
    }

    return IDCU_MOD_ERR_MODULE_NOT_FOUND;
}

idcu_ModuleHealthStatus idcu_kernel_get_module_health(idcu_MicroKernel *k,
                                                      const char *module_name) {
    if (!k || !module_name) {
        return IDCU_MODULE_HEALTH_UNKNOWN;
    }

    for (uint32_t i = 0; i < k->tracked_cnt; i++) {
        idcu_TrackedModule *tracked = &k->tracked_modules[i];
        if (strcmp(tracked->iface->name, module_name) == 0) {
            return tracked->health_monitor.health;
        }
    }

    return IDCU_MODULE_HEALTH_UNKNOWN;
}

int idcu_kernel_restart_module(idcu_MicroKernel *k, const char *module_name) {
    if (!k || !module_name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    IDCU_LOG_INFO("Restarting module: %s", module_name);

    for (uint32_t i = 0; i < k->tracked_cnt; i++) {
        idcu_TrackedModule *tracked = &k->tracked_modules[i];
        if (strcmp(tracked->iface->name, module_name) == 0) {
            if (tracked->is_dynamic) {
                return idcu_kernel_hotplug_restart(k, module_name);
            } else {
                IDCU_LOG_WARN("Static module %s cannot be restarted at runtime", module_name);
                return IDCU_ERR_NOT_SUPPORTED;
            }
        }
    }

    return IDCU_MOD_ERR_MODULE_NOT_FOUND;
}

int idcu_kernel_set_health_callback(idcu_MicroKernel *k, idcu_ModuleHealthCallback callback,
                                    void *user_data) {
    if (!k) {
        return IDCU_ERR_INVALID_PARAM;
    }

    k->health_callback = callback;
    k->health_callback_data = user_data;
    return IDCU_ERR_SUCCESS;
}

void idcu_kernel_health_check(idcu_MicroKernel *k) {
    if (!k) {
        return;
    }

    uint64_t current_time = get_current_time_ms();

    if (current_time - k->last_health_check < IDCU_MODULE_HEALTH_CHECK_INTERVAL_MS) {
        return;
    }

    k->last_health_check = current_time;

    for (uint32_t i = 0; i < k->tracked_cnt; i++) {
        idcu_TrackedModule *tracked = &k->tracked_modules[i];
        idcu_ModuleHealthMonitor *health = &tracked->health_monitor;

        if (tracked->state != IDCU_MOD_STATE_RUNNING) {
            continue;
        }

        uint64_t time_since_checkin = current_time - health->last_checkin;

        if (time_since_checkin > health->health_check_interval_ms * 3) {
            if (health->health != IDCU_MODULE_HEALTH_CRITICAL) {
                health->health = IDCU_MODULE_HEALTH_CRITICAL;
                IDCU_LOG_ERROR("Module %s health: CRITICAL (no checkin for %llu ms)",
                               tracked->iface->name, (unsigned long long)time_since_checkin);

                if (k->health_callback) {
                    k->health_callback(tracked->iface->name, IDCU_MODULE_HEALTH_CRITICAL,
                                       k->health_callback_data);
                }
            }

            if (health->enable_auto_restart) {
                uint64_t window_elapsed = current_time - health->restart_window_start;
                if (window_elapsed > IDCU_MODULE_RESTART_WINDOW_MS) {
                    health->restart_count = 0;
                    health->restart_window_start = current_time;
                }

                if (health->restart_count < IDCU_MODULE_MAX_RESTARTS) {
                    health->restart_count++;
                    IDCU_LOG_WARN("Attempting to restart module %s (attempt %u/%u)",
                                  tracked->iface->name, health->restart_count,
                                  IDCU_MODULE_MAX_RESTARTS);
                    idcu_kernel_restart_module(k, tracked->iface->name);
                } else {
                    IDCU_LOG_ERROR("Max restart attempts (%u) reached for module %s. Giving up.",
                                   IDCU_MODULE_MAX_RESTARTS, tracked->iface->name);
                }
            }
        } else if (time_since_checkin > health->health_check_interval_ms) {
            if (health->health != IDCU_MODULE_HEALTH_DEGRADED) {
                health->health = IDCU_MODULE_HEALTH_DEGRADED;
                IDCU_LOG_WARN("Module %s health: DEGRADED (no checkin for %llu ms)",
                              tracked->iface->name, (unsigned long long)time_since_checkin);

                if (k->health_callback) {
                    k->health_callback(tracked->iface->name, IDCU_MODULE_HEALTH_DEGRADED,
                                       k->health_callback_data);
                }
            }
        }
    }
}
