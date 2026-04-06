#include "healthcheck_integration.h"
#include "module_def.h"
#include "idcu/log/log.h"

static idcu_HealthcheckIntegration* g_healthcheck_integration = NULL;

int idcu_healthcheck_integration_init(idcu_HealthcheckIntegration* integration)
{
    if (!integration) {
        return IDCU_ERR_INVALID_PARAM;
    }

    integration->initialized = 1;
    g_healthcheck_integration = integration;
    IDCU_LOG_INFO("Healthcheck integration initialized");
    return IDCU_ERR_OK;
}

void idcu_healthcheck_integration_destroy(idcu_HealthcheckIntegration* integration)
{
    if (!integration || !integration->initialized) {
        return;
    }

    integration->initialized = 0;
    g_healthcheck_integration = NULL;
    IDCU_LOG_INFO("Healthcheck integration destroyed");
}

static idcu_HealthcheckIntegration s_healthcheck_integration;

static int healthcheck_integration_module_init(void)
{
    return idcu_healthcheck_integration_init(&s_healthcheck_integration);
}

static int healthcheck_integration_module_run(void)
{
    return 0;
}

static int healthcheck_integration_module_stop(void)
{
    idcu_healthcheck_integration_destroy(&s_healthcheck_integration);
    return 0;
}

IDCU_REGISTER_MODULE(healthcheck_integration, IDCU_MODULE_VERSION(1, 0, 0), 
                      healthcheck_integration_module_init, 
                      healthcheck_integration_module_run, 
                      healthcheck_integration_module_stop);
