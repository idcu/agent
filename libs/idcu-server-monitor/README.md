# IDCU Server Monitor Module

服务器监控模块，提供系统资源监控功能。

## 功能特性

- 系统信息收集（主机名、操作系统、运行时间）
- CPU 负载监控
- 内存使用监控
- 磁盘空间监控
- JSON 和 Prometheus 格式导出

## 使用示例

```c
#include "idcu/server_monitor/server_monitor.h"

idcu_ServerMonitor monitor;
idcu_server_monitor_init(&monitor);

idcu_ServerMetrics metrics;
idcu_server_monitor_collect(&monitor, &metrics);

char buffer[4096];
size_t size;
idcu_server_monitor_export_json(&monitor, buffer, sizeof(buffer), &size);

idcu_server_monitor_destroy(&monitor);
```

## 配置

暂无特殊配置。

## 依赖

- idcu-common
- idcu-log
