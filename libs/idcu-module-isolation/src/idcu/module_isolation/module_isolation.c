#include "idcu/module_isolation/module_isolation.h"
#include "idcu/log/log.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#endif

static idcu_IsolationCallback g_on_crash = NULL;
static void *g_on_crash_data = NULL;
static idcu_IsolationCallback g_on_restart = NULL;
static void *g_on_restart_data = NULL;

static uint64_t get_current_time_ms(void) {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

static void sleep_ms(uint32_t ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

static idcu_IsolatedModule *find_module(idcu_ModuleIsolator *isolator, const char *module_name) {
    if (!isolator || !module_name) {
        return NULL;
    }

    for (uint32_t i = 0; i < isolator->module_count; i++) {
        if (strcmp(isolator->modules[i].module_name, module_name) == 0) {
            return &isolator->modules[i];
        }
    }
    return NULL;
}

#ifdef _WIN32
static DWORD WINAPI monitor_thread(LPVOID param) {
#else
static void *monitor_thread(void *param) {
#endif
    idcu_ModuleIsolator *isolator = (idcu_ModuleIsolator *)param;
    uint64_t current_time;

    idcu_log_info("Module isolation monitor started");

    while (!isolator->should_exit) {
        current_time = get_current_time_ms();

        for (uint32_t i = 0; i < isolator->module_count; i++) {
            idcu_IsolatedModule *mod = &isolator->modules[i];

            if (mod->state == IDCU_MODULE_STATE_CRASHED) {
                uint64_t time_since_crash = current_time - mod->last_crash_time_ms;

                if (time_since_crash >= mod->restart_delay_ms) {
                    if (mod->crash_count < mod->max_restarts) {
                        idcu_log_info("Attempting to restart module: %s (crash %u/%u)",
                                      mod->module_name, mod->crash_count + 1, mod->max_restarts);

                        mod->state = IDCU_MODULE_STATE_RESTARTING;

                        if (g_on_restart) {
                            g_on_restart(mod->module_name, g_on_restart_data);
                        }

                        mod->state = IDCU_MODULE_STATE_RUNNING;
                        mod->last_restart_time_ms = current_time;
                        idcu_log_info("Module restarted: %s", mod->module_name);
                    } else {
                        idcu_log_error("Module %s has crashed %u times, marking as failed",
                                       mod->module_name, mod->crash_count);
                        mod->state = IDCU_MODULE_STATE_FAILED;
                    }
                }
            }
        }

        sleep_ms(100);
    }

    idcu_log_info("Module isolation monitor stopped");
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int idcu_isolator_init(idcu_ModuleIsolator *isolator) {
    if (!isolator) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(isolator, 0, sizeof(idcu_ModuleIsolator));
    isolator->should_exit = 0;

    idcu_log_info("Module isolator initialized");
    return IDCU_ERR_OK;
}

void idcu_isolator_destroy(idcu_ModuleIsolator *isolator) {
    if (!isolator) {
        return;
    }

    idcu_isolator_stop_monitor(isolator);
    memset(isolator, 0, sizeof(idcu_ModuleIsolator));
}

int idcu_isolator_register_module(idcu_ModuleIsolator *isolator, const char *module_name,
                                  void *module_handle, uint32_t max_restarts,
                                  uint32_t restart_delay_ms) {
    if (!isolator || !module_name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (isolator->module_count >= IDCU_ISOLATION_MAX_MODULES) {
        idcu_log_error("Maximum module count reached");
        return IDCU_ERR_NO_MEMORY;
    }

    if (find_module(isolator, module_name)) {
        idcu_log_error("Module already registered: %s", module_name);
        return IDCU_ERR_ALREADY_EXISTS;
    }

    idcu_IsolatedModule *mod = &isolator->modules[isolator->module_count];
    strncpy(mod->module_name, module_name, sizeof(mod->module_name) - 1);
    mod->module_name[sizeof(mod->module_name) - 1] = '\0';
    mod->state = IDCU_MODULE_STATE_STOPPED;
    mod->crash_count = 0;
    mod->last_crash_time_ms = 0;
    mod->last_restart_time_ms = 0;
    mod->max_restarts = max_restarts > 0 ? max_restarts : IDCU_ISOLATION_DEFAULT_MAX_RESTARTS;
    mod->restart_delay_ms =
        restart_delay_ms > 0 ? restart_delay_ms : IDCU_ISOLATION_DEFAULT_RESTART_DELAY_MS;
    mod->module_handle = module_handle;
    mod->crash_log[0] = '\0';

    isolator->module_count++;

    idcu_log_info("Module registered for isolation: %s", module_name);
    return IDCU_ERR_OK;
}

int idcu_isolator_unregister_module(idcu_ModuleIsolator *isolator, const char *module_name) {
    if (!isolator || !module_name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (uint32_t i = 0; i < isolator->module_count; i++) {
        if (strcmp(isolator->modules[i].module_name, module_name) == 0) {
            for (uint32_t j = i; j < isolator->module_count - 1; j++) {
                memcpy(&isolator->modules[j], &isolator->modules[j + 1],
                       sizeof(idcu_IsolatedModule));
            }
            isolator->module_count--;

            idcu_log_info("Module unregistered from isolation: %s", module_name);
            return IDCU_ERR_OK;
        }
    }

    return IDCU_ERR_NOT_FOUND;
}

int idcu_isolator_start_monitor(idcu_ModuleIsolator *isolator) {
    if (!isolator) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (isolator->monitor_thread) {
        return IDCU_ERR_OK;
    }

    isolator->should_exit = 0;

#ifdef _WIN32
    isolator->monitor_thread = CreateThread(NULL, 0, monitor_thread, isolator, 0, NULL);
    if (!isolator->monitor_thread) {
        idcu_log_error("Failed to create monitor thread");
        return IDCU_ERR_SYSTEM;
    }
#else
    if (pthread_create((pthread_t *)&isolator->monitor_thread, NULL, monitor_thread, isolator) !=
        0) {
        idcu_log_error("Failed to create monitor thread");
        return IDCU_ERR_SYSTEM;
    }
#endif

    idcu_log_info("Module isolation monitor started");
    return IDCU_ERR_OK;
}

int idcu_isolator_stop_monitor(idcu_ModuleIsolator *isolator) {
    if (!isolator) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (!isolator->monitor_thread) {
        return IDCU_ERR_OK;
    }

    isolator->should_exit = 1;

#ifdef _WIN32
    WaitForSingleObject(isolator->monitor_thread, INFINITE);
    CloseHandle(isolator->monitor_thread);
#else
    pthread_join(*(pthread_t *)&isolator->monitor_thread, NULL);
#endif

    isolator->monitor_thread = NULL;

    idcu_log_info("Module isolation monitor stopped");
    return IDCU_ERR_OK;
}

int idcu_isolator_report_crash(idcu_ModuleIsolator *isolator, const char *module_name,
                               const char *crash_info) {
    if (!isolator || !module_name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_IsolatedModule *mod = find_module(isolator, module_name);
    if (!mod) {
        return IDCU_ERR_NOT_FOUND;
    }

    mod->crash_count++;
    mod->last_crash_time_ms = get_current_time_ms();
    mod->state = IDCU_MODULE_STATE_CRASHED;

    if (crash_info) {
        strncpy(mod->crash_log, crash_info, sizeof(mod->crash_log) - 1);
        mod->crash_log[sizeof(mod->crash_log) - 1] = '\0';
    }

    idcu_log_error("Module crash reported: %s (crash #%u)", module_name, mod->crash_count);

    if (g_on_crash) {
        g_on_crash(module_name, g_on_crash_data);
    }

    return IDCU_ERR_OK;
}

int idcu_isolator_handle_crash(idcu_ModuleIsolator *isolator, const char *module_name) {
    if (!isolator || !module_name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    return idcu_isolator_report_crash(isolator, module_name, "Crash detected");
}

int idcu_isolator_get_module_state(idcu_ModuleIsolator *isolator, const char *module_name,
                                   idcu_ModuleState *state) {
    if (!isolator || !module_name || !state) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_IsolatedModule *mod = find_module(isolator, module_name);
    if (!mod) {
        return IDCU_ERR_NOT_FOUND;
    }

    *state = mod->state;
    return IDCU_ERR_OK;
}

int idcu_isolator_get_crash_count(idcu_ModuleIsolator *isolator, const char *module_name,
                                  uint32_t *count) {
    if (!isolator || !module_name || !count) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_IsolatedModule *mod = find_module(isolator, module_name);
    if (!mod) {
        return IDCU_ERR_NOT_FOUND;
    }

    *count = mod->crash_count;
    return IDCU_ERR_OK;
}

int idcu_isolator_get_crash_log(idcu_ModuleIsolator *isolator, const char *module_name,
                                char *buffer, size_t buffer_size) {
    if (!isolator || !module_name || !buffer || buffer_size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_IsolatedModule *mod = find_module(isolator, module_name);
    if (!mod) {
        return IDCU_ERR_NOT_FOUND;
    }

    strncpy(buffer, mod->crash_log, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return IDCU_ERR_OK;
}

int idcu_isolator_set_on_crash(idcu_ModuleIsolator *isolator, idcu_IsolationCallback callback,
                               void *user_data) {
    (void)isolator;
    g_on_crash = callback;
    g_on_crash_data = user_data;
    return IDCU_ERR_OK;
}

int idcu_isolator_set_on_restart(idcu_ModuleIsolator *isolator, idcu_IsolationCallback callback,
                                 void *user_data) {
    (void)isolator;
    g_on_restart = callback;
    g_on_restart_data = user_data;
    return IDCU_ERR_OK;
}

int idcu_isolator_restart_module(idcu_ModuleIsolator *isolator, const char *module_name) {
    if (!isolator || !module_name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_IsolatedModule *mod = find_module(isolator, module_name);
    if (!mod) {
        return IDCU_ERR_NOT_FOUND;
    }

    idcu_log_info("Manual restart requested for module: %s", module_name);

    mod->state = IDCU_MODULE_STATE_RESTARTING;

    if (g_on_restart) {
        g_on_restart(module_name, g_on_restart_data);
    }

    mod->state = IDCU_MODULE_STATE_RUNNING;
    mod->last_restart_time_ms = get_current_time_ms();

    idcu_log_info("Module manually restarted: %s", module_name);
    return IDCU_ERR_OK;
}

int idcu_isolator_unload_module(idcu_ModuleIsolator *isolator, const char *module_name) {
    if (!isolator || !module_name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_IsolatedModule *mod = find_module(isolator, module_name);
    if (!mod) {
        return IDCU_ERR_NOT_FOUND;
    }

    idcu_log_info("Unloading module: %s", module_name);
    mod->state = IDCU_MODULE_STATE_STOPPED;

    return IDCU_ERR_OK;
}
