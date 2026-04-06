#include "metrics_integration.h"
#include "module_def.h"
#include "idcu/log/log.h"

static idcu_MetricsIntegration* g_metrics_integration = NULL;

int idcu_metrics_integration_init(idcu_MetricsIntegration* integration)
{
    if (!integration) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_metrics_init(&integration->collector);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to initialize metrics collector");
        return ret;
    }

    integration->initialized = 1;
    g_metrics_integration = integration;
    IDCU_LOG_INFO("Metrics integration initialized");
    return IDCU_ERR_OK;
}

void idcu_metrics_integration_destroy(idcu_MetricsIntegration* integration)
{
    if (!integration || !integration->initialized) {
        return;
    }

    idcu_metrics_destroy(&integration->collector);
    integration->initialized = 0;
    g_metrics_integration = NULL;
    IDCU_LOG_INFO("Metrics integration destroyed");
}

idcu_MetricsCollector* idcu_metrics_integration_get_collector(idcu_MetricsIntegration* integration)
{
    if (!integration || !integration->initialized) {
        return NULL;
    }
    return &integration->collector;
}

static idcu_MetricsIntegration s_metrics_integration;

static int metrics_integration_module_init(void)
{
    return idcu_metrics_integration_init(&s_metrics_integration);
}

static int metrics_integration_module_run(void)
{
    return 0;
}

static int metrics_integration_module_stop(void)
{
    idcu_metrics_integration_destroy(&s_metrics_integration);
    return 0;
}

IDCU_REGISTER_MODULE(metrics_integration, IDCU_MODULE_VERSION(1, 0, 0), 
                      metrics_integration_module_init, 
                      metrics_integration_module_run, 
                      metrics_integration_module_stop);
