#include "http_client_integration.h"
#include "idcu/log/log.h"
#include "module_def.h"

static idcu_HttpClientIntegration *g_http_client_integration = NULL;

int idcu_http_client_integration_init(idcu_HttpClientIntegration *integration) {
    if (!integration) {
        return IDCU_ERR_INVALID_PARAM;
    }

    integration->initialized = 1;
    g_http_client_integration = integration;
    IDCU_LOG_INFO("HTTP client integration initialized");
    return IDCU_ERR_OK;
}

void idcu_http_client_integration_destroy(idcu_HttpClientIntegration *integration) {
    if (!integration || !integration->initialized) {
        return;
    }

    integration->initialized = 0;
    g_http_client_integration = NULL;
    IDCU_LOG_INFO("HTTP client integration destroyed");
}

static idcu_HttpClientIntegration s_http_client_integration;

static int http_client_integration_module_init(void) {
    return idcu_http_client_integration_init(&s_http_client_integration);
}

static int http_client_integration_module_run(void) { return 0; }

static int http_client_integration_module_stop(void) {
    idcu_http_client_integration_destroy(&s_http_client_integration);
    return 0;
}

IDCU_REGISTER_MODULE(http_client_integration, IDCU_MODULE_VERSION(1, 0, 0),
                     http_client_integration_module_init, http_client_integration_module_run,
                     http_client_integration_module_stop);
