# idcu-metrics-module

IDCU Agent 的指标业务模块，提供：
- 指标收集和聚合
- 指标注册和管理
- 指标导出（Prometheus 格式）
- 指标告警规则
- 性能指标：指标更新延迟 ≤ 10ms，支持 10000 指标/秒

## 使用方法

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
    
    idcu_metrics_module_register_counter(&mm, "http_requests_total", "总 HTTP 请求数");
    idcu_metrics_module_inc(&mm, "http_requests_total", 1);
    
    char buffer[4096];
    idcu_metrics_module_export_prometheus(&mm, buffer, sizeof(buffer));
    printf("%s\n", buffer);
    
    idcu_metrics_module_stop(&mm);
    idcu_metrics_module_destroy(&mm);
    
    return 0;
}
```

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
