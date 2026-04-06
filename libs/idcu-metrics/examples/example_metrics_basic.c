/*
 * 简单指标示例
 */

#include "idcu/metrics/metrics.h"
#include "idcu/common/string_buf.h"
#include <stdio.h>

int main() {
    printf("=== idcu-metrics 简单示例 ===\n\n");

    // 初始化指标收集器
    idcu_MetricsCollector collector;
    idcu_metrics_init(&collector);
    printf("✓ Metrics collector initialized\n\n");

    // 注册各种类型的指标
    printf("Registering metrics...\n");
    idcu_metrics_register(&collector, "http_requests_total", "Total HTTP requests", IDCU_METRIC_COUNTER);
    idcu_metrics_register(&collector, "memory_usage_bytes", "Current memory usage", IDCU_METRIC_GAUGE);
    idcu_metrics_register(&collector, "request_duration_ms", "Request duration", IDCU_METRIC_HISTOGRAM);
    printf("✓ Metrics registered\n\n");

    // 更新指标
    printf("Updating metrics...\n");
    idcu_metrics_inc(&collector, "http_requests_total", 100);
    idcu_metrics_set(&collector, "memory_usage_bytes", 1024000);
    idcu_metrics_observe(&collector, "request_duration_ms", 50);
    idcu_metrics_observe(&collector, "request_duration_ms", 100);
    idcu_metrics_observe(&collector, "request_duration_ms", 75);
    printf("✓ Metrics updated\n\n");

    // 导出为文本格式
    printf("=== Text Export ===\n");
    idcu_StringBuf text_buf;
    idcu_strbuf_init(&text_buf, 1024);
    idcu_metrics_export_text(&collector, &text_buf);
    printf("%s\n", idcu_strbuf_data(&text_buf));
    idcu_strbuf_destroy(&text_buf);

    // 导出为 Prometheus 格式
    printf("=== Prometheus Export ===\n");
    idcu_StringBuf prom_buf;
    idcu_strbuf_init(&prom_buf, 1024);
    idcu_metrics_export_prometheus(&collector, &prom_buf);
    printf("%s\n", idcu_strbuf_data(&prom_buf));
    idcu_strbuf_destroy(&prom_buf);

    // 清理
    idcu_metrics_destroy(&collector);
    printf("✓ Metrics collector destroyed\n\n");

    printf("=== 示例完成 ===\n");
    return 0;
}
