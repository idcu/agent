# Monitor - 监控模块

本模块提供系统健康检查和性能指标收集功能。

## 模块内容

### health_check.h/c - 健康监控
监控模块和系统的健康状态：
- 模块心跳检测
- 健康状态管理 (UNKNOWN, HEALTHY, WARNING, CRITICAL, DEAD)
- 错误计数和重启计数
- 健康状态变化回调
- 可配置的超时和阈值
- 最大支持 64 个模块的健康监控

### metrics.h/c - 指标收集
收集和导出系统性能指标：
- 多种指标类型 (COUNTER, GAUGE, HISTOGRAM)
- 指标注册和更新
- 最小值、最大值、总和、计数统计
- 文本格式导出
- Prometheus 格式导出
- 最大支持 64 个指标

## 使用示例

### 使用健康监控
```c
#include "monitor/health_check.h"

HealthMonitor monitor;
health_monitor_init(&monitor);

// 注册模块
health_register_module(&monitor, module_id);

// 更新心跳
health_update_heartbeat(&monitor, module_id);

// 报告错误
health_report_error(&monitor, module_id);

// 检查所有模块健康状态
health_check_all(&monitor);

// 获取模块状态
HealthStatus status = health_get_status(&monitor, module_id);

health_monitor_destroy(&monitor);
```

### 使用指标收集
```c
#include "monitor/metrics.h"

MetricsCollector collector;
metrics_init(&collector);

// 注册指标
metrics_register(&collector, "requests_total", "总请求数", METRIC_COUNTER);
metrics_register(&collector, "memory_usage", "内存使用量", METRIC_GAUGE);

// 更新指标
metrics_inc(&collector, "requests_total", 1);
metrics_set(&collector, "memory_usage", 1024000);
metrics_observe(&collector, "response_time", 42);

// 导出指标
char buffer[4096];
metrics_export_text(&collector, buffer, sizeof(buffer));
metrics_export_prometheus(&collector, buffer, sizeof(buffer));

metrics_destroy(&collector);
```
