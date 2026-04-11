#ifndef IDCU_CORE_MODULE_CORE_MODULE_H
#define IDCU_CORE_MODULE_CORE_MODULE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/log/log.h>
#include <idcu/config/config.h>
#include <idcu/healthcheck/healthcheck.h>
#include <idcu/metrics/metrics.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_CORE_MODULE_STATE_UNINITIALIZED = 0,
    IDCU_CORE_MODULE_STATE_INITIALIZED,
    IDCU_CORE_MODULE_STATE_STARTING,
    IDCU_CORE_MODULE_STATE_RUNNING,
    IDCU_CORE_MODULE_STATE_STOPPING,
    IDCU_CORE_MODULE_STATE_STOPPED,
    IDCU_CORE_MODULE_STATE_ERROR
} idcu_CoreModuleState;

typedef enum {
    IDCU_CORE_SIGNAL_INT = 0,
    IDCU_CORE_SIGNAL_TERM,
    IDCU_CORE_SIGNAL_HUP,
    IDCU_CORE_SIGNAL_COUNT
} idcu_CoreSignal;

typedef struct {
    char name[128];
    char version[64];
    char config_path[1024];
    char log_path[1024];
    idcu_LogLevel log_level;
    int enable_healthcheck;
    int enable_metrics;
    int enable_signals;
    uint64_t graceful_shutdown_timeout_ms;
} idcu_CoreModuleConfig;

typedef struct {
    idcu_CoreModuleState state;
    idcu_CoreModuleConfig config;
    idcu_ConfigManager* config_manager;
    idcu_Logger* logger;
    idcu_HealthCheck* health_check;
    idcu_MetricsRegistry* metrics_registry;
    int should_shutdown;
    void* user_data;
} idcu_CoreModule;

int idcu_core_module_config_init(idcu_CoreModuleConfig* config);

int idcu_core_module_init(idcu_CoreModule* module, const idcu_CoreModuleConfig* config);

int idcu_core_module_start(idcu_CoreModule* module);

int idcu_core_module_stop(idcu_CoreModule* module);

void idcu_core_module_destroy(idcu_CoreModule* module);

int idcu_core_module_load_config(idcu_CoreModule* module, const char* config_path);

idcu_Logger* idcu_core_module_get_logger(idcu_CoreModule* module);

idcu_ConfigManager* idcu_core_module_get_config_manager(idcu_CoreModule* module);

idcu_HealthCheck* idcu_core_module_get_health_check(idcu_CoreModule* module);

idcu_MetricsRegistry* idcu_core_module_get_metrics_registry(idcu_CoreModule* module);

int idcu_core_module_request_shutdown(idcu_CoreModule* module);

int idcu_core_module_should_shutdown(idcu_CoreModule* module);

#ifdef __cplusplus
}
#endif

#endif
