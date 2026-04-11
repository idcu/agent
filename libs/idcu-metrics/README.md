# idcu-metrics

IDCU Agent 的指标库。

## 功能特性
- 多种指标类型（Counter、Gauge、Histogram、Summary）
- 标签支持
- Prometheus 格式导出
- 指标持久化
- 线程安全实现

## 使用方法

```c
#include <idcu/metrics/metrics.h>

idcu_MetricsRegistry* registry;
int ret = idcu_metrics_registry_init(&registry);
if (ret == IDCU_ERR_OK) {
    // 创建指标
    idcu_Metric* counter = idcu_metrics_counter_create(registry, "requests_total", "总请求数");
    
    // 使用指标
    idcu_metrics_counter_inc(counter);
    
    // 导出指标
    char* prometheus = idcu_metrics_export_prometheus(registry);
    printf("%s", prometheus);
    free(prometheus);
    
    // 清理
    idcu_metrics_registry_destroy(registry);
}
```

## API 参考
- `idcu_metrics_registry_init()` - 初始化指标注册表
- `idcu_metrics_registry_destroy()` - 销毁指标注册表
- `idcu_metrics_registry_is_initialized()` - 检查注册表是否已初始化
- `idcu_metrics_counter_create()` - 创建计数器
- `idcu_metrics_gauge_create()` - 创建仪表盘
- `idcu_metrics_histogram_create()` - 创建直方图
- `idcu_metrics_summary_create()` - 创建摘要
- `idcu_metrics_export_prometheus()` - 导出为 Prometheus 格式

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
