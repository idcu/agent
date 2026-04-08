#include "json_integration.h"
#include "idcu/log/log.h"
#include "module_def.h"

static idcu_JsonIntegration *g_json_integration = NULL;

int idcu_json_integration_init(idcu_JsonIntegration *integration) {
    if (!integration) {
        return IDCU_ERR_INVALID_PARAM;
    }

    integration->initialized = 1;
    g_json_integration = integration;
    IDCU_LOG_INFO("JSON integration initialized");
    return IDCU_ERR_OK;
}

void idcu_json_integration_destroy(idcu_JsonIntegration *integration) {
    if (!integration || !integration->initialized) {
        return;
    }

    integration->initialized = 0;
    g_json_integration = NULL;
    IDCU_LOG_INFO("JSON integration destroyed");
}

static idcu_JsonIntegration s_json_integration;

static int json_integration_module_init(void) {
    return idcu_json_integration_init(&s_json_integration);
}

static int json_integration_module_run(void) { return 0; }

static int json_integration_module_stop(void) {
    idcu_json_integration_destroy(&s_json_integration);
    return 0;
}

IDCU_REGISTER_MODULE(json_integration, IDCU_MODULE_VERSION(1, 0, 0), json_integration_module_init,
                     json_integration_module_run, json_integration_module_stop);
