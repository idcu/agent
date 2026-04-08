# 任务 3.27: idcu-server-monitor - 服务器监控

## 目标

创建服务器监控库，支持：
- CPU 使用率监控
- 内存使用率监控
- 磁盘使用率监控
- 网络流量监控
- 进程监控
- 系统负载监控
- 告警规则
- 数据记录和历史查询

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-server-monitor/include/idcu/server_monitor
mkdir -p libs/idcu-server-monitor/src/idcu/server_monitor
mkdir -p libs/idcu-server-monitor/tests
mkdir -p libs/idcu-server-monitor/examples
```

### 2. 创建服务器监控头文件 (server_monitor.h)

创建 `libs/idcu-server-monitor/include/idcu/server_monitor/server_monitor.h`：

```c
#ifndef IDCU_SERVER_MONITOR_SERVER_MONITOR_H
#define IDCU_SERVER_MONITOR_SERVER_MONITOR_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/lock.h"
#include "idcu/metrics/metrics.h"
#include "idcu/alert/alert.h"
#include "idcu/storage/storage.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_ALERT_LEVEL_INFO = 0,
    IDCU_ALERT_LEVEL_WARNING,
    IDCU_ALERT_LEVEL_CRITICAL,
    IDCU_ALERT_LEVEL_FATAL
} idcu_AlertLevel;

typedef struct
{
    double user;
    double system;
    double nice;
    double idle;
    double iowait;
    double irq;
    double softirq;
    uint64_t timestamp;
} idcu_CpuStats;

typedef struct
{
    uint64_t total;
    uint64_t used;
    uint64_t free;
    uint64_t available;
    uint64_t buffers;
    uint64_t cached;
    uint64_t swap_total;
    uint64_t swap_used;
    uint64_t swap_free;
    uint64_t timestamp;
} idcu_MemoryStats;

typedef struct
{
    char mount_point[256];
    char filesystem[256];
    uint64_t total;
    uint64_t used;
    uint64_t free;
    double usage_percent;
    uint64_t timestamp;
} idcu_DiskStats;

typedef struct
{
    char interface[64];
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
    uint64_t timestamp;
} idcu_NetworkStats;

typedef struct
{
    int pid;
    char name[256];
    char command[1024];
    char user[128];
    double cpu_usage;
    uint64_t memory_usage;
    uint64_t start_time;
    int running;
    uint64_t timestamp;
} idcu_ProcessStats;

typedef struct
{
    double load_1m;
    double load_5m;
    double load_15m;
    int running_processes;
    int total_processes;
    uint64_t timestamp;
} idcu_LoadStats;

typedef struct
{
    char metric_name[128];
    idcu_AlertLevel level;
    double threshold;
    int duration_seconds;
    int consecutive_violations;
    char message[512];
} idcu_AlertRule;

typedef struct
{
    char id[128];
    char metric_name[128];
    idcu_AlertLevel level;
    double current_value;
    double threshold;
    char message[512];
    uint64_t start_time;
    uint64_t last_update;
    int active;
} idcu_AlertRecord;

typedef struct
{
    uint64_t collect_interval_ms;
    uint64_t history_retention_seconds;
    int enable_alerts;
    int enable_history;
    char storage_path[1024];
} idcu_ServerMonitorConfig;

typedef struct
{
    idcu_ServerMonitorConfig config;
    
    idcu_CpuStats cpu_stats;
    idcu_MemoryStats memory_stats;
    idcu_Vector disk_stats;
    idcu_Vector network_stats;
    idcu_Vector process_stats;
    idcu_LoadStats load_stats;
    
    idcu_Vector alert_rules;
    idcu_Vector active_alerts;
    
    idcu_MetricsRegistry* metrics;
    idcu_Storage* storage;
    idcu_Mutex lock;
    
    int initialized;
    int running;
    uint64_t last_collect_time;
} idcu_ServerMonitor;

int  idcu_server_monitor_config_init(idcu_ServerMonitorConfig* config);

int  idcu_server_monitor_init(idcu_ServerMonitor* monitor, const idcu_ServerMonitorConfig* config);
void idcu_server_monitor_destroy(idcu_ServerMonitor* monitor);

int  idcu_server_monitor_start(idcu_ServerMonitor* monitor);
int  idcu_server_monitor_stop(idcu_ServerMonitor* monitor);

int  idcu_server_monitor_collect_now(idcu_ServerMonitor* monitor);

int  idcu_server_monitor_get_cpu(idcu_ServerMonitor* monitor, idcu_CpuStats* stats);
int  idcu_server_monitor_get_memory(idcu_ServerMonitor* monitor, idcu_MemoryStats* stats);
int  idcu_server_monitor_get_disks(idcu_ServerMonitor* monitor, idcu_Vector* disks);
int  idcu_server_monitor_get_networks(idcu_ServerMonitor* monitor, idcu_Vector* networks);
int  idcu_server_monitor_get_processes(idcu_ServerMonitor* monitor, idcu_Vector* processes);
int  idcu_server_monitor_get_load(idcu_ServerMonitor* monitor, idcu_LoadStats* stats);

int  idcu_server_monitor_add_alert_rule(idcu_ServerMonitor* monitor, const idcu_AlertRule* rule);
int  idcu_server_monitor_remove_alert_rule(idcu_ServerMonitor* monitor, const char* metric_name);
int  idcu_server_monitor_get_alert_rules(idcu_ServerMonitor* monitor, idcu_Vector* rules);

int  idcu_server_monitor_get_active_alerts(idcu_ServerMonitor* monitor, idcu_Vector* alerts);
int  idcu_server_monitor_acknowledge_alert(idcu_ServerMonitor* monitor, const char* alert_id);

int  idcu_server_monitor_get_history(idcu_ServerMonitor* monitor, const char* metric_name,
                                      uint64_t start_time, uint64_t end_time,
                                      idcu_Vector* values);

int  idcu_server_monitor_export_json(idcu_ServerMonitor* monitor, char* buffer, size_t buffer_size);
int  idcu_server_monitor_export_prometheus(idcu_ServerMonitor* monitor, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-server-monitor/module.yaml`：

```yaml
name: idcu-server-monitor
version: 1.0.0
description: Server monitoring library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-metrics
  - idcu-alert
  - idcu-storage
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-server-monitor

headers:
  - idcu/server_monitor/server_monitor.h

features:
  - cpu: CPU usage monitoring
  - memory: Memory usage monitoring
  - disk: Disk usage monitoring
  - network: Network traffic monitoring
  - process: Process monitoring
  - load: System load monitoring
  - alerts: Alert rules
  - history: Data recording and historical queries

testing:
  enabled: true
  framework: internal
```

### 4. 创建 README.md

创建 `libs/idcu-server-monitor/README.md`：

```markdown
# idcu-server-monitor

IDCU Agent 的服务器监控库。

## 功能特性

- **CPU监控**: CPU使用率监控
- **内存监控**: 内存使用率监控
- **磁盘监控**: 磁盘使用率监控
- **网络监控**: 网络流量监控
- **进程监控**: 进程监控
- **负载监控**: 系统负载监控
- **告警规则**: 灵活的告警规则配置
- **历史数据**: 数据记录和历史查询

## 快速开始

### 初始化监控器

```c
#include "idcu/server_monitor/server_monitor.h"

idcu_ServerMonitorConfig config;
idcu_server_monitor_config_init(&config);

config.collect_interval_ms = 5000;
config.history_retention_seconds = 86400 * 7;
config.enable_alerts = 1;
config.enable_history = 1;

idcu_ServerMonitor monitor;
idcu_server_monitor_init(&monitor, &config);
```

### 添加告警规则

```c
idcu_AlertRule cpu_rule = {
    .metric_name = "cpu_usage",
    .level = IDCU_ALERT_LEVEL_WARNING,
    .threshold = 80.0,
    .duration_seconds = 60,
    .consecutive_violations = 3,
    .message = "CPU usage is high"
};
idcu_server_monitor_add_alert_rule(&monitor, &cpu_rule);

idcu_AlertRule memory_rule = {
    .metric_name = "memory_usage",
    .level = IDCU_ALERT_LEVEL_CRITICAL,
    .threshold = 90.0,
    .duration_seconds = 30,
    .consecutive_violations = 2,
    .message = "Memory usage is critical"
};
idcu_server_monitor_add_alert_rule(&monitor, &memory_rule);
```

### 启动监控

```c
idcu_server_monitor_start(&monitor);
```

### 获取监控数据

```c
idcu_CpuStats cpu;
idcu_server_monitor_get_cpu(&monitor, &cpu);
printf("CPU: user=%.1f%% system=%.1f%% idle=%.1f%%\n", 
       cpu.user, cpu.system, cpu.idle);

idcu_MemoryStats memory;
idcu_server_monitor_get_memory(&monitor, &memory);
printf("Memory: used=%" PRIu64 "MB free=%" PRIu64 "MB\n",
       memory.used / 1024 / 1024, memory.free / 1024 / 1024);

idcu_LoadStats load;
idcu_server_monitor_get_load(&monitor, &load);
printf("Load: %.2f %.2f %.2f\n", load.load_1m, load.load_5m, load.load_15m);
```

### 获取告警

```c
idcu_Vector alerts;
idcu_vector_init(&alerts, sizeof(idcu_AlertRecord));

idcu_server_monitor_get_active_alerts(&monitor, &alerts);

for (size_t i = 0; i < alerts.size; i++) {
    idcu_AlertRecord* alert = (idcu_AlertRecord*)idcu_vector_get(&alerts, i);
    printf("[ALERT] %s: %.2f (threshold: %.2f)\n",
           alert->message, alert->current_value, alert->threshold);
}

idcu_vector_destroy(&alerts);
```

### 导出数据

```c
char buffer[16384];
idcu_server_monitor_export_json(&monitor, buffer, sizeof(buffer));
printf("%s\n", buffer);
```

### 停止监控

```c
idcu_server_monitor_stop(&monitor);
idcu_server_monitor_destroy(&monitor);
```

## 告警级别

| 级别 | 说明 |
|-----|------|
| INFO | 信息 |
| WARNING | 警告 |
| CRITICAL | 严重 |
| FATAL | 致命 |

## API 文档

详见 [include/idcu/server_monitor/server_monitor.h](include/idcu/server_monitor/server_monitor.h)
```

## 验证检查清单

- [ ] 服务器监控头文件已创建
- [ ] 服务器监控实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以获取系统监控数据
- [ ] 告警规则正常工作
- [ ] 数据导出功能正常

## Git 提交

```bash
git add libs/idcu-server-monitor/
git commit -m "feat: add idcu-server-monitor library

- Add CPU usage monitoring
- Add memory usage monitoring
- Add disk usage monitoring
- Add network traffic monitoring
- Add process monitoring
- Add system load monitoring
- Add alert rules
- Add data recording and historical queries
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 无法获取系统信息 | 权限不足 | 确保程序有足够权限 |
| 告警不触发 | 规则配置错误 | 检查告警规则配置 |
| 历史数据丢失 | 存储配置错误 | 检查存储路径配置 |
