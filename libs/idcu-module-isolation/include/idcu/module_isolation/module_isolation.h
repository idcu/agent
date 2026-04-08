#ifndef IDCU_MODULE_ISOLATION_MODULE_ISOLATION_H
#define IDCU_MODULE_ISOLATION_MODULE_ISOLATION_H

#include "idcu/common/error_code.h"

#include <stddef.h>
#include <stdint.h>

#define IDCU_ISOLATION_MAX_MODULES              64
#define IDCU_ISOLATION_CRASH_LOG_MAX            4096
#define IDCU_ISOLATION_DEFAULT_MAX_RESTARTS     3
#define IDCU_ISOLATION_DEFAULT_RESTART_DELAY_MS 5000

typedef enum
{
    IDCU_MODULE_STATE_STOPPED = 0,
    IDCU_MODULE_STATE_RUNNING,
    IDCU_MODULE_STATE_CRASHED,
    IDCU_MODULE_STATE_RESTARTING,
    IDCU_MODULE_STATE_FAILED
} idcu_ModuleState;

typedef struct
{
    char             module_name[128];
    idcu_ModuleState state;
    uint32_t         crash_count;
    uint64_t         last_crash_time_ms;
    uint64_t         last_restart_time_ms;
    uint32_t         max_restarts;
    uint32_t         restart_delay_ms;
    char             crash_log[IDCU_ISOLATION_CRASH_LOG_MAX];
    void*            module_handle;
} idcu_IsolatedModule;

typedef struct
{
    idcu_IsolatedModule modules[IDCU_ISOLATION_MAX_MODULES];
    uint32_t            module_count;
    volatile int        should_exit;
    void*               monitor_thread;
} idcu_ModuleIsolator;

typedef void (*idcu_IsolationCallback)(const char* module_name, void* user_data);

int  idcu_isolator_init(idcu_ModuleIsolator* isolator);
void idcu_isolator_destroy(idcu_ModuleIsolator* isolator);

int idcu_isolator_register_module(idcu_ModuleIsolator* isolator, const char* module_name,
                                  void* module_handle, uint32_t max_restarts,
                                  uint32_t restart_delay_ms);
int idcu_isolator_unregister_module(idcu_ModuleIsolator* isolator, const char* module_name);

int idcu_isolator_start_monitor(idcu_ModuleIsolator* isolator);
int idcu_isolator_stop_monitor(idcu_ModuleIsolator* isolator);

int idcu_isolator_report_crash(idcu_ModuleIsolator* isolator, const char* module_name,
                               const char* crash_info);
int idcu_isolator_handle_crash(idcu_ModuleIsolator* isolator, const char* module_name);

int idcu_isolator_get_module_state(idcu_ModuleIsolator* isolator, const char* module_name,
                                   idcu_ModuleState* state);
int idcu_isolator_get_crash_count(idcu_ModuleIsolator* isolator, const char* module_name,
                                  uint32_t* count);
int idcu_isolator_get_crash_log(idcu_ModuleIsolator* isolator, const char* module_name,
                                char* buffer, size_t buffer_size);

int idcu_isolator_set_on_crash(idcu_ModuleIsolator* isolator, idcu_IsolationCallback callback,
                               void* user_data);
int idcu_isolator_set_on_restart(idcu_ModuleIsolator* isolator, idcu_IsolationCallback callback,
                                 void* user_data);

int idcu_isolator_restart_module(idcu_ModuleIsolator* isolator, const char* module_name);
int idcu_isolator_unload_module(idcu_ModuleIsolator* isolator, const char* module_name);

#endif  // IDCU_MODULE_ISOLATION_MODULE_ISOLATION_H
