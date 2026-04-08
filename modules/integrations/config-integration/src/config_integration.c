#include "config_integration.h"
#include "idcu/log/log.h"
#include "module_def.h"
#include <string.h>

static idcu_ConfigIntegration *g_config_integration = NULL;

int idcu_config_integration_init(idcu_ConfigIntegration *integration, const char *config_file) {
    if (!integration) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(integration, 0, sizeof(idcu_ConfigIntegration));

    if (config_file) {
        strncpy(integration->config_file, config_file, sizeof(integration->config_file) - 1);
    }

    int ret = idcu_config_init(config_file);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    integration->initialized = 1;
    g_config_integration = integration;
    IDCU_LOG_INFO("Config integration initialized");
    return IDCU_ERR_OK;
}

void idcu_config_integration_destroy(idcu_ConfigIntegration *integration) {
    if (!integration || !integration->initialized) {
        return;
    }

    idcu_config_shutdown();
    integration->initialized = 0;
    g_config_integration = NULL;
    IDCU_LOG_INFO("Config integration destroyed");
}

int idcu_config_integration_reload(idcu_ConfigIntegration *integration) {
    if (!integration || !integration->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    return idcu_config_reload();
}

int idcu_config_integration_save(idcu_ConfigIntegration *integration, const char *file_path) {
    if (!integration || !integration->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    return idcu_config_save(file_path);
}

static idcu_ConfigIntegration s_config_integration;

static int config_integration_module_init(void) {
    return idcu_config_integration_init(&s_config_integration, "config/agent.cfg");
}

static int config_integration_module_run(void) { return 0; }

static int config_integration_module_stop(void) {
    idcu_config_integration_destroy(&s_config_integration);
    return 0;
}

IDCU_REGISTER_MODULE(config_integration, IDCU_MODULE_VERSION(1, 0, 0),
                     config_integration_module_init, config_integration_module_run,
                     config_integration_module_stop);
