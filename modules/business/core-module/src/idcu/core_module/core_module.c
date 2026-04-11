#include <idcu/core_module/core_module.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

static idcu_CoreModule* g_core_module = NULL;

#ifdef _WIN32
static BOOL WINAPI windows_signal_handler(DWORD signal) {
    if (g_core_module) {
        idcu_core_module_request_shutdown(g_core_module);
    }
    return TRUE;
}
#else
static void unix_signal_handler(int signal) {
    if (g_core_module) {
        idcu_core_module_request_shutdown(g_core_module);
    }
}
#endif

int idcu_core_module_config_init(idcu_CoreModuleConfig* config) {
    if (!config) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(config, 0, sizeof(idcu_CoreModuleConfig));
    strncpy(config->name, "idcu-core-module", sizeof(config->name) - 1);
    strncpy(config->version, "1.0.0", sizeof(config->version) - 1);
    strncpy(config->config_path, "config/app.yaml", sizeof(config->config_path) - 1);
    strncpy(config->log_path, "logs/idcu.log", sizeof(config->log_path) - 1);
    config->log_level = IDCU_LOG_LEVEL_INFO;
    config->enable_healthcheck = 1;
    config->enable_metrics = 1;
    config->enable_signals = 1;
    config->graceful_shutdown_timeout_ms = 30000;

    return IDCU_ERR_OK;
}

int idcu_core_module_init(idcu_CoreModule* module, const idcu_CoreModuleConfig* config) {
    if (!module || !config) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(module, 0, sizeof(idcu_CoreModule));
    memcpy(&module->config, config, sizeof(idcu_CoreModuleConfig));
    module->state = IDCU_CORE_MODULE_STATE_UNINITIALIZED;
    module->should_shutdown = 0;

    if (module->config.enable_signals) {
        g_core_module = module;
#ifdef _WIN32
        SetConsoleCtrlHandler(windows_signal_handler, TRUE);
#else
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = unix_signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);
#endif
    }

    module->state = IDCU_CORE_MODULE_STATE_INITIALIZED;

    return IDCU_ERR_OK;
}

int idcu_core_module_start(idcu_CoreModule* module) {
    if (!module) {
        return IDCU_ERR_INVALID_ARG;
    }

    if (module->state != IDCU_CORE_MODULE_STATE_INITIALIZED &&
        module->state != IDCU_CORE_MODULE_STATE_STOPPED) {
        return IDCU_ERR_INVALID_STATE;
    }

    module->state = IDCU_CORE_MODULE_STATE_STARTING;

    int ret = idcu_config_manager_create(&module->config_manager);
    if (ret != IDCU_ERR_OK) {
        module->state = IDCU_CORE_MODULE_STATE_ERROR;
        return ret;
    }

    idcu_LoggerConfig logger_config = {
        .level = module->config.log_level,
        .output_path = module->config.log_path,
        .enable_console = 1
    };

    ret = idcu_logger_create(&logger_config, &module->logger);
    if (ret != IDCU_ERR_OK) {
        idcu_config_manager_destroy(module->config_manager);
        module->state = IDCU_CORE_MODULE_STATE_ERROR;
        return ret;
    }

    if (module->config.enable_healthcheck) {
        idcu_HealthCheckConfig hc_config = {
            .check_interval_ms = 5000,
            .timeout_ms = 10000
        };
        ret = idcu_healthcheck_create(&hc_config, &module->health_check);
        if (ret != IDCU_ERR_OK) {
            idcu_logger_destroy(module->logger);
            idcu_config_manager_destroy(module->config_manager);
            module->state = IDCU_CORE_MODULE_STATE_ERROR;
            return ret;
        }
    }

    if (module->config.enable_metrics) {
        ret = idcu_metrics_registry_create(&module->metrics_registry);
        if (ret != IDCU_ERR_OK) {
            if (module->health_check) {
                idcu_healthcheck_destroy(module->health_check);
            }
            idcu_logger_destroy(module->logger);
            idcu_config_manager_destroy(module->config_manager);
            module->state = IDCU_CORE_MODULE_STATE_ERROR;
            return ret;
        }
    }

    if (module->config.config_path[0] != '\0') {
        ret = idcu_core_module_load_config(module, module->config.config_path);
    }

    module->state = IDCU_CORE_MODULE_STATE_RUNNING;

    idcu_logger_log(module->logger, IDCU_LOG_LEVEL_INFO, "Core module started successfully");

    return IDCU_ERR_OK;
}

int idcu_core_module_stop(idcu_CoreModule* module) {
    if (!module) {
        return IDCU_ERR_INVALID_ARG;
    }

    if (module->state != IDCU_CORE_MODULE_STATE_RUNNING) {
        return IDCU_ERR_INVALID_STATE;
    }

    module->state = IDCU_CORE_MODULE_STATE_STOPPING;

    idcu_logger_log(module->logger, IDCU_LOG_LEVEL_INFO, "Stopping core module...");

    if (module->metrics_registry) {
        idcu_metrics_registry_destroy(module->metrics_registry);
        module->metrics_registry = NULL;
    }

    if (module->health_check) {
        idcu_healthcheck_destroy(module->health_check);
        module->health_check = NULL;
    }

    if (module->logger) {
        idcu_logger_destroy(module->logger);
        module->logger = NULL;
    }

    if (module->config_manager) {
        idcu_config_manager_destroy(module->config_manager);
        module->config_manager = NULL;
    }

    module->state = IDCU_CORE_MODULE_STATE_STOPPED;

    return IDCU_ERR_OK;
}

void idcu_core_module_destroy(idcu_CoreModule* module) {
    if (!module) {
        return;
    }

    if (module->state == IDCU_CORE_MODULE_STATE_RUNNING) {
        idcu_core_module_stop(module);
    }

    g_core_module = NULL;

    memset(module, 0, sizeof(idcu_CoreModule));
}

int idcu_core_module_load_config(idcu_CoreModule* module, const char* config_path) {
    if (!module || !config_path) {
        return IDCU_ERR_INVALID_ARG;
    }

    return idcu_config_manager_load_from_file(module->config_manager, config_path);
}

idcu_Logger* idcu_core_module_get_logger(idcu_CoreModule* module) {
    if (!module) {
        return NULL;
    }
    return module->logger;
}

idcu_ConfigManager* idcu_core_module_get_config_manager(idcu_CoreModule* module) {
    if (!module) {
        return NULL;
    }
    return module->config_manager;
}

idcu_HealthCheck* idcu_core_module_get_health_check(idcu_CoreModule* module) {
    if (!module) {
        return NULL;
    }
    return module->health_check;
}

idcu_MetricsRegistry* idcu_core_module_get_metrics_registry(idcu_CoreModule* module) {
    if (!module) {
        return NULL;
    }
    return module->metrics_registry;
}

int idcu_core_module_request_shutdown(idcu_CoreModule* module) {
    if (!module) {
        return IDCU_ERR_INVALID_ARG;
    }
    module->should_shutdown = 1;
    return IDCU_ERR_OK;
}

int idcu_core_module_should_shutdown(idcu_CoreModule* module) {
    if (!module) {
        return 0;
    }
    return module->should_shutdown;
}
