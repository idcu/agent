#include "monitor/metrics.h"
#include "monitor/prometheus_exporter.h"
#include "network/network_layer.h"
#include "utils/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static volatile int g_running = 1;

void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    idcu_network_init();

    signal(SIGINT, signal_handler);

    IDCU_LOG_INFO("IDCU Prometheus Exporter Example");

    idcu_MetricsCollector metrics;
    int ret = idcu_metrics_init(&metrics);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to initialize metrics collector");
        return 1;
    }

    idcu_metrics_register(&metrics, "http_requests_total", "Total number of HTTP requests", IDCU_METRIC_COUNTER);
    idcu_metrics_register(&metrics, "memory_usage_bytes", "Current memory usage in bytes", IDCU_METRIC_GAUGE);
    idcu_metrics_register(&metrics, "active_connections", "Number of active connections", IDCU_METRIC_GAUGE);
    idcu_metrics_register(&metrics, "request_duration_ms", "Request duration in milliseconds", IDCU_METRIC_HISTOGRAM);

    idcu_PrometheusExporter exporter;
    ret = idcu_prometheus_exporter_init(&exporter, &metrics, "0.0.0.0", 9090);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to initialize Prometheus exporter");
        idcu_metrics_destroy(&metrics);
        return 1;
    }

    ret = idcu_prometheus_exporter_start(&exporter);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to start Prometheus exporter");
        idcu_prometheus_exporter_destroy(&exporter);
        idcu_metrics_destroy(&metrics);
        return 1;
    }

    IDCU_LOG_INFO("Prometheus exporter is running on http://0.0.0.0:9090/metrics");
    IDCU_LOG_INFO("Press Ctrl+C to stop...");

    uint64_t request_count = 0;
    while (g_running) {
        idcu_prometheus_exporter_poll(&exporter, 100);
        
        request_count++;
        idcu_metrics_inc(&metrics, "http_requests_total", 1);
        idcu_metrics_set(&metrics, "memory_usage_bytes", 1024000 + (request_count % 1000) * 100);
        idcu_metrics_set(&metrics, "active_connections", request_count % 10);
        idcu_metrics_observe(&metrics, "request_duration_ms", 50 + (request_count % 50));
    }

    IDCU_LOG_INFO("Shutting down...");

    idcu_prometheus_exporter_stop(&exporter);
    idcu_prometheus_exporter_destroy(&exporter);
    idcu_metrics_destroy(&metrics);
    idcu_network_cleanup();

    IDCU_LOG_INFO("Exiting");
    return 0;
}
