#ifndef IDCU_METRICS_PROMETHEUS_EXPORTER_H
#define IDCU_METRICS_PROMETHEUS_EXPORTER_H

#include "idcu/common/error_code.h"
#include "idcu/metrics/metrics.h"
#include "idcu/network/network_layer.h"
#include "idcu/common/lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_PROMETHEUS_DEFAULT_PORT 9090
#define IDCU_PROMETHEUS_DEFAULT_ADDR "0.0.0.0"
#define IDCU_PROMETHEUS_BUFFER_SIZE 8192
#define IDCU_PROMETHEUS_METRICS_PATH "/metrics"

typedef struct {
    idcu_NetworkServer server;
    idcu_MetricsCollector* metrics;
    char bind_address[64];
    uint16_t port;
    int running;
    idcu_Mutex lock;
} idcu_PrometheusExporter;

int idcu_prometheus_exporter_init(idcu_PrometheusExporter* exporter,
                                    idcu_MetricsCollector* metrics,
                                    const char* bind_address,
                                    uint16_t port);

void idcu_prometheus_exporter_destroy(idcu_PrometheusExporter* exporter);

int idcu_prometheus_exporter_start(idcu_PrometheusExporter* exporter);

int idcu_prometheus_exporter_stop(idcu_PrometheusExporter* exporter);

int idcu_prometheus_exporter_handle_request(idcu_PrometheusExporter* exporter,
                                              idcu_NetworkSocket* client);

int idcu_prometheus_exporter_poll(idcu_PrometheusExporter* exporter, int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
