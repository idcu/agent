
#include "module/module_def.h"
#include "common/error_code.h"
#include "utils/log.h"
#include "monitor/prometheus_exporter.h"
#include "monitor/metrics.h"

static idcu_PrometheusExporter g_exporter;
static int g_initialized = 0;

static int prometheus_exporter_module_init(void) {
    idcu_log_info("[prometheus_exporter] Initializing Prometheus exporter module");
    
    idcu_MetricsCollector* collector = idcu_global_metrics_collector();
    if (!collector) {
        idcu_log_error("[prometheus_exporter] Metrics collector not initialized");
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    int ret = idcu_prometheus_exporter_init(&g_exporter, collector, NULL, 0);
    if (ret != IDCU_ERR_OK) {
        idcu_log_error("[prometheus_exporter] Failed to initialize exporter");
        return ret;
    }
    
    g_initialized = 1;
    return IDCU_ERR_OK;
}

static int prometheus_exporter_module_start(void) {
    idcu_log_info("[prometheus_exporter] Starting Prometheus exporter module");
    
    if (!g_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    int ret = idcu_prometheus_exporter_start(&g_exporter);
    if (ret != IDCU_ERR_OK) {
        idcu_log_error("[prometheus_exporter] Failed to start exporter");
        return ret;
    }
    
    return IDCU_ERR_OK;
}

static int prometheus_exporter_module_stop(void) {
    idcu_log_info("[prometheus_exporter] Stopping Prometheus exporter module");
    
    if (!g_initialized) {
        return IDCU_ERR_OK;
    }
    
    return idcu_prometheus_exporter_stop(&g_exporter);
}

static void prometheus_exporter_module_destroy(void) {
    idcu_log_info("[prometheus_exporter] Destroying Prometheus exporter module");
    
    if (g_initialized) {
        idcu_prometheus_exporter_destroy(&g_exporter);
        g_initialized = 0;
    }
}

IDCU_MODULE_DEFINE(
    "prometheus_exporter",
    "1.0.0",
    "Prometheus metrics exporter module",
    prometheus_exporter_module_init,
    prometheus_exporter_module_start,
    prometheus_exporter_module_stop,
    prometheus_exporter_module_destroy
);
