#include "log_integration.h"
#include "module_def.h"
#include <string.h>

static idcu_LogIntegration *g_log_integration = NULL;

int idcu_log_integration_init(idcu_LogIntegration *integration, const char *log_file,
                              idcu_LogLevel level) {
    if (!integration) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(integration, 0, sizeof(idcu_LogIntegration));

    if (log_file) {
        strncpy(integration->log_file, log_file, sizeof(integration->log_file) - 1);
    }
    integration->log_level = level;

    int ret = idcu_log_init(log_file, level);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    integration->initialized = 1;
    g_log_integration = integration;
    IDCU_LOG_INFO("Log integration initialized");
    return IDCU_ERR_OK;
}

void idcu_log_integration_destroy(idcu_LogIntegration *integration) {
    if (!integration || !integration->initialized) {
        return;
    }

    idcu_log_shutdown();
    integration->initialized = 0;
    g_log_integration = NULL;
    IDCU_LOG_INFO("Log integration destroyed");
}

int idcu_log_integration_set_level(idcu_LogIntegration *integration, idcu_LogLevel level) {
    if (!integration || !integration->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_log_set_level(level);
    integration->log_level = level;
    return IDCU_ERR_OK;
}

idcu_LogLevel idcu_log_integration_get_level(idcu_LogIntegration *integration) {
    if (!integration || !integration->initialized) {
        return IDCU_LOG_INFO;
    }
    return integration->log_level;
}

static idcu_LogIntegration s_log_integration;

static int log_integration_module_init(void) {
    return idcu_log_integration_init(&s_log_integration, NULL, IDCU_LOG_INFO);
}

static int log_integration_module_run(void) { return 0; }

static int log_integration_module_stop(void) {
    idcu_log_integration_destroy(&s_log_integration);
    return 0;
}

IDCU_REGISTER_MODULE(log_integration, IDCU_MODULE_VERSION(1, 0, 0), log_integration_module_init,
                     log_integration_module_run, log_integration_module_stop);
