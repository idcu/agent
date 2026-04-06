/*
 * Prometheus 导出示例
 */

#include "idcu/metrics/metrics.h"
#include "idcu/metrics/prometheus_exporter.h"
#include "idcu/log/log.h"
#include <stdio.h>

int main() {
    printf("=== idcu-metrics Prometheus 导出示例 ===\n\n");

    // 初始化日志
    idcu_log_init(NULL, IDCU_LOG_INFO);

    // 初始化全局指标收集器
    idcu_global_metrics_init();
    printf("✓ Global metrics initialized\n\n");

    // 注册指标
    printf("Registering metrics...\n");
    idcu_global_metrics_register("http_requests_total", "Total HTTP requests", IDCU_METRIC_COUNTER);
    idcu_global_metrics_register("memory_usage_bytes", "Current memory usage", IDCU_METRIC_GAUGE);
    printf("✓ Metrics registered\n\n");

    // 更新指标
    printf("Updating metrics...\n");
    idcu_global_metrics_inc("http_requests_total", 42);
    idcu_global_metrics_set("memory_usage_bytes", 2048000);
    printf("✓ Metrics updated\n\n");

    // 初始化 Prometheus 导出器
    printf("Initializing Prometheus exporter...\n");
    idcu_PrometheusExporter exporter;
    int ret = idcu_prometheus_exporter_init(&exporter, 
                                           idcu_global_metrics_collector(),
                                           "127.0.0.1", 
                                           9090);
    if (ret != IDCU_ERR_OK) {
        printf("✗ Failed to initialize Prometheus exporter: %d\n", ret);
        idcu_global_metrics_destroy();
        idcu_log_shutdown();
        return 1;
    }
    printf("✓ Prometheus exporter initialized on 127.0.0.1:9090\n\n");

    printf("Prometheus exporter is ready!\n");
    printf("Metrics available at: http://127.0.0.1:9090/metrics\n\n");
    printf("Press Ctrl+C to stop...\n");

    // 注意：在实际使用中，你需要在循环中调用 idcu_prometheus_exporter_poll()
    // 这里为了演示，我们只展示初始化过程

    // 清理
    printf("\nCleaning up...\n");
    idcu_prometheus_exporter_destroy(&exporter);
    idcu_global_metrics_destroy();
    idcu_log_shutdown();

    printf("✓ Cleanup completed\n");
    printf("=== 示例完成 ===\n");
    return 0;
}
