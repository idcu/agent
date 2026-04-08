#include "network_monitor_integration.h"
#include <string.h>

int idcu_network_monitor_integration_init(idcu_NetworkMonitorIntegration *integration) {
    if (!integration) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(integration, 0, sizeof(idcu_NetworkMonitorIntegration));

    int ret = idcu_metrics_init(&integration->metrics);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to initialize metrics collector");
        return ret;
    }

    integration->initialized = 1;
    IDCU_LOG_INFO("Network-Monitor integration initialized");
    return IDCU_ERR_OK;
}

void idcu_network_monitor_integration_destroy(idcu_NetworkMonitorIntegration *integration) {
    if (!integration || !integration->initialized) {
        return;
    }

    idcu_network_monitor_integration_stop(integration);
    idcu_metrics_destroy(&integration->metrics);
    integration->initialized = 0;

    IDCU_LOG_INFO("Network-Monitor integration destroyed");
}

int idcu_network_monitor_integration_start(idcu_NetworkMonitorIntegration *integration,
                                           const char *bind_address, uint16_t port) {
    if (!integration || !integration->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    int ret = idcu_prometheus_exporter_init(&integration->exporter, &integration->metrics,
                                            bind_address, port);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to initialize Prometheus exporter");
        return ret;
    }

    ret = idcu_prometheus_exporter_start(&integration->exporter);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to start Prometheus exporter");
        idcu_prometheus_exporter_destroy(&integration->exporter);
        return ret;
    }

    IDCU_LOG_INFO("Network-Monitor integration started");
    return IDCU_ERR_OK;
}

int idcu_network_monitor_integration_stop(idcu_NetworkMonitorIntegration *integration) {
    if (!integration || !integration->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_prometheus_exporter_destroy(&integration->exporter);
    IDCU_LOG_INFO("Network-Monitor integration stopped");
    return IDCU_ERR_OK;
}

int idcu_network_monitor_integration_poll(idcu_NetworkMonitorIntegration *integration,
                                          int timeout_ms) {
    if (!integration || !integration->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    return idcu_prometheus_exporter_poll(&integration->exporter, timeout_ms);
}

idcu_MetricsCollector *
idcu_network_monitor_integration_get_metrics(idcu_NetworkMonitorIntegration *integration) {
    if (!integration || !integration->initialized) {
        return NULL;
    }
    return &integration->metrics;
}
