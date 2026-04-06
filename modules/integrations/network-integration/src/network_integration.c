#include "network_integration.h"
#include "module_def.h"
#include "idcu/log/log.h"

static idcu_NetworkIntegration* g_network_integration = NULL;

int idcu_network_integration_init(idcu_NetworkIntegration* integration)
{
    if (!integration) {
        return IDCU_ERR_INVALID_PARAM;
    }

    integration->initialized = 1;
    g_network_integration = integration;
    IDCU_LOG_INFO("Network integration initialized");
    return IDCU_ERR_OK;
}

void idcu_network_integration_destroy(idcu_NetworkIntegration* integration)
{
    if (!integration || !integration->initialized) {
        return;
    }

    integration->initialized = 0;
    g_network_integration = NULL;
    IDCU_LOG_INFO("Network integration destroyed");
}

static idcu_NetworkIntegration s_network_integration;

static int network_integration_module_init(void)
{
    return idcu_network_integration_init(&s_network_integration);
}

static int network_integration_module_run(void)
{
    return 0;
}

static int network_integration_module_stop(void)
{
    idcu_network_integration_destroy(&s_network_integration);
    return 0;
}

IDCU_REGISTER_MODULE(network_integration, IDCU_MODULE_VERSION(1, 0, 0), 
                      network_integration_module_init, 
                      network_integration_module_run, 
                      network_integration_module_stop);
