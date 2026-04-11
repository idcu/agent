# idcu-metrics-module

Metrics business module for IDCU Agent, providing:
- Metrics collection and aggregation
- Metrics registration and management
- Metrics export (Prometheus format)
- Metrics alert rules
- Performance metrics: metrics update latency ≤ 10ms, supports 10000 metrics/sec

## Usage

```c
#include <idcu/metrics_module/metrics_module.h>

int main() {
    idcu_MetricsModule mm;
    int ret = idcu_metrics_module_init(&mm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_metrics_module_start(&mm);
    if (ret != IDCU_ERR_OK) {
        idcu_metrics_module_destroy(&mm);
        return 1;
    }
    
    idcu_metrics_module_register_counter(&mm, "http_requests_total", "Total HTTP requests");
    idcu_metrics_module_inc(&mm, "http_requests_total", 1);
    
    char buffer[4096];
    idcu_metrics_module_export_prometheus(&mm, buffer, sizeof(buffer));
    printf("%s\n", buffer);
    
    idcu_metrics_module_stop(&mm);
    idcu_metrics_module_destroy(&mm);
    
    return 0;
}
```
