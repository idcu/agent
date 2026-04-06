#ifndef IDCU_NETWORK_MONITOR_INTEGRATION_H
#define IDCU_NETWORK_MONITOR_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/log/log.h"
#include "idcu/metrics/metrics.h"
#include "idcu/metrics/prometheus_exporter.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    idcu_MetricsCollector metrics;
    idcu_PrometheusExporter exporter;
    int initialized;
} idcu_NetworkMonitorIntegration;

int idcu_network_monitor_integration_init(idcu_NetworkMonitorIntegration* integration);

void idcu_network_monitor_integration_destroy(idcu_NetworkMonitorIntegration* integration);

int idcu_network_monitor_integration_start(idcu_NetworkMonitorIntegration* integration,
                                            const char* bind_address,
                                            uint16_t port);

int idcu_network_monitor_integration_stop(idcu_NetworkMonitorIntegration* integration);

int idcu_network_monitor_integration_poll(idcu_NetworkMonitorIntegration* integration, int timeout_ms);

idcu_MetricsCollector* idcu_network_monitor_integration_get_metrics(idcu_NetworkMonitorIntegration* integration);

#ifdef __cplusplus
}
#endif

#endif
