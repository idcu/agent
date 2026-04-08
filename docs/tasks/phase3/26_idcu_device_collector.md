# 任务 3.26: idcu-device-collector - 设备数据采集

## 目标

创建设备数据采集库，支持：
- 多种设备类型支持（服务器、网络设备、IoT设备）
- 多种采集协议（SNMP、HTTP、Modbus、自定义）
- 数据收集和聚合
- 数据过滤和转换
- 定时采集
- 数据缓存
- 数据导出

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-device-collector/include/idcu/device_collector
mkdir -p libs/idcu-device-collector/src/idcu/device_collector
mkdir -p libs/idcu-device-collector/tests
mkdir -p libs/idcu-device-collector/examples
```

### 2. 创建设备采集头文件 (device_collector.h)

创建 `libs/idcu-device-collector/include/idcu/device_collector/device_collector.h`：

```c
#ifndef IDCU_DEVICE_COLLECTOR_DEVICE_COLLECTOR_H
#define IDCU_DEVICE_COLLECTOR_DEVICE_COLLECTOR_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/scheduler/scheduler.h"
#include "idcu/storage/storage.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_DeviceId;
typedef uint64_t idcu_MetricId;

typedef enum
{
    IDCU_DEVICE_TYPE_SERVER = 0,
    IDCU_DEVICE_TYPE_NETWORK,
    IDCU_DEVICE_TYPE_IOT,
    IDCU_DEVICE_TYPE_CUSTOM
} idcu_DeviceType;

typedef enum
{
    IDCU_PROTOCOL_SNMP = 0,
    IDCU_PROTOCOL_HTTP,
    IDCU_PROTOCOL_MODBUS,
    IDCU_PROTOCOL_TCP,
    IDCU_PROTOCOL_UDP,
    IDCU_PROTOCOL_CUSTOM
} idcu_ProtocolType;

typedef enum
{
    IDCU_METRIC_TYPE_GAUGE = 0,
    IDCU_METRIC_TYPE_COUNTER,
    IDCU_METRIC_TYPE_HISTOGRAM,
    IDCU_METRIC_TYPE_STRING,
    IDCU_METRIC_TYPE_BOOLEAN
} idcu_MetricType;

typedef struct
{
    char name[128];
    char description[512];
    idcu_MetricType type;
    char unit[64];
    double min_value;
    double max_value;
} idcu_MetricDef;

typedef struct
{
    idcu_MetricId id;
    char name[128];
    idcu_MetricType type;
    double value_double;
    int64_t value_int;
    char value_str[256];
    int value_bool;
    uint64_t timestamp;
} idcu_MetricValue;

typedef struct
{
    char key[128];
    char value[512];
} idcu_DeviceProperty;

typedef struct
{
    idcu_DeviceId id;
    char name[128];
    char description[512];
    idcu_DeviceType type;
    idcu_ProtocolType protocol;
    
    char address[256];
    uint16_t port;
    char credentials[1024];
    
    idcu_Vector properties;
    idcu_Vector metrics;
    
    uint64_t collect_interval_ms;
    uint64_t last_collect_time;
    int enabled;
    
    int connected;
    uint64_t connect_timeout_ms;
    uint64_t retry_count;
} idcu_Device;

typedef int (*idcu_DeviceCollectFunc)(idcu_Device* device, idcu_Vector* values, void* user_data);
typedef int (*idcu_DeviceConnectFunc)(idcu_Device* device, void* user_data);
typedef int (*idcu_DeviceDisconnectFunc)(idcu_Device* device, void* user_data);
typedef int (*idcu_MetricTransformFunc)(idcu_MetricValue* value, void* user_data);
typedef int (*idcu_MetricFilterFunc)(const idcu_MetricValue* value, void* user_data);

typedef struct
{
    char config_path[1024];
    uint64_t default_collect_interval_ms;
    uint64_t default_connect_timeout_ms;
    int enable_cache;
    size_t cache_max_entries;
    uint64_t cache_ttl_ms;
    int enable_persistence;
    char storage_path[1024];
} idcu_DeviceCollectorConfig;

typedef struct
{
    idcu_DeviceCollectorConfig config;
    idcu_Vector devices;
    idcu_HashMap device_map;
    idcu_Mutex lock;
    
    idcu_DeviceConnectFunc connect_func;
    idcu_DeviceDisconnectFunc disconnect_func;
    idcu_DeviceCollectFunc collect_func;
    void* protocol_user_data;
    
    idcu_MetricTransformFunc transform_func;
    idcu_MetricFilterFunc filter_func;
    void* transform_user_data;
    
    idcu_Scheduler* scheduler;
    idcu_Storage* storage;
    
    int initialized;
    int running;
} idcu_DeviceCollector;

int  idcu_device_collector_config_init(idcu_DeviceCollectorConfig* config);

int  idcu_device_collector_init(idcu_DeviceCollector* collector, 
                                  const idcu_DeviceCollectorConfig* config);
void idcu_device_collector_destroy(idcu_DeviceCollector* collector);

int  idcu_device_collector_start(idcu_DeviceCollector* collector);
int  idcu_device_collector_stop(idcu_DeviceCollector* collector);

int  idcu_device_collector_load_config(idcu_DeviceCollector* collector, const char* path);
int  idcu_device_collector_save_config(idcu_DeviceCollector* collector, const char* path);

int  idcu_device_collector_register_device(idcu_DeviceCollector* collector, const idcu_Device* device);
int  idcu_device_collector_unregister_device(idcu_DeviceCollector* collector, idcu_DeviceId device_id);
idcu_Device* idcu_device_collector_get_device(idcu_DeviceCollector* collector, idcu_DeviceId device_id);
idcu_Device* idcu_device_collector_get_device_by_name(idcu_DeviceCollector* collector, const char* name);

size_t idcu_device_collector_get_device_count(idcu_DeviceCollector* collector);
int  idcu_device_collector_get_all_devices(idcu_DeviceCollector* collector, idcu_Vector* devices);

int  idcu_device_collector_enable_device(idcu_DeviceCollector* collector, idcu_DeviceId device_id);
int  idcu_device_collector_disable_device(idcu_DeviceCollector* collector, idcu_DeviceId device_id);

int  idcu_device_collector_collect_now(idcu_DeviceCollector* collector, idcu_DeviceId device_id);
int  idcu_device_collector_collect_all_now(idcu_DeviceCollector* collector);

int  idcu_device_collector_set_protocol_handlers(idcu_DeviceCollector* collector,
                                                  idcu_DeviceConnectFunc connect,
                                                  idcu_DeviceDisconnectFunc disconnect,
                                                  idcu_DeviceCollectFunc collect,
                                                  void* user_data);
int  idcu_device_collector_set_transform(idcu_DeviceCollector* collector,
                                          idcu_MetricTransformFunc transform,
                                          idcu_MetricFilterFunc filter,
                                          void* user_data);

int  idcu_device_collector_get_metrics(idcu_DeviceCollector* collector, idcu_DeviceId device_id,
                                         idcu_Vector* values);
int  idcu_device_collector_get_latest_metric(idcu_DeviceCollector* collector, idcu_DeviceId device_id,
                                              const char* metric_name, idcu_MetricValue* value);

int  idcu_device_collector_export_metrics(idcu_DeviceCollector* collector, char* buffer, 
                                            size_t buffer_size, const char* format);

int  idcu_device_init(idcu_Device* device, const char* name, idcu_DeviceType type);
void idcu_device_destroy(idcu_Device* device);

int  idcu_device_add_property(idcu_Device* device, const char* key, const char* value);
int  idcu_device_get_property(idcu_Device* device, const char* key, char* value, size_t value_size);

int  idcu_device_add_metric(idcu_Device* device, const idcu_MetricDef* metric);
int  idcu_device_remove_metric(idcu_Device* device, const char* metric_name);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-device-collector/module.yaml`：

```yaml
name: idcu-device-collector
version: 1.0.0
description: Device data collection library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-scheduler
  - idcu-storage
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-device-collector

headers:
  - idcu/device_collector/device_collector.h

features:
  - multi_device: Multiple device types support (server, network, IoT)
  - multi_protocol: Multiple collection protocols (SNMP, HTTP, Modbus, custom)
  - data_collection: Data collection and aggregation
  - data_transform: Data filtering and transformation
  - scheduled: Scheduled collection
  - caching: Data caching
  - export: Data export

testing:
  enabled: true
  framework: internal
```

### 4. 创建 README.md

创建 `libs/idcu-device-collector/README.md`：

```markdown
# idcu-device-collector

IDCU Agent 的设备数据采集库。

## 功能特性

- **多设备类型**: 支持服务器、网络设备、IoT设备等
- **多协议支持**: SNMP、HTTP、Modbus、TCP、UDP、自定义协议
- **数据采集**: 定时采集和手动触发采集
- **数据转换**: 支持数据过滤和转换
- **数据缓存**: 内置数据缓存机制
- **数据导出**: 支持多种格式导出

## 快速开始

### 初始化采集器

```c
#include "idcu/device_collector/device_collector.h"

idcu_DeviceCollectorConfig config;
idcu_device_collector_config_init(&config);

config.default_collect_interval_ms = 5000;
config.default_connect_timeout_ms = 3000;
config.enable_cache = 1;
config.cache_max_entries = 1000;
config.cache_ttl_ms = 60000;

idcu_DeviceCollector collector;
idcu_device_collector_init(&collector, &config);
```

### 创建设备

```c
idcu_Device device;
idcu_device_init(&device, "server-01", IDCU_DEVICE_TYPE_SERVER);

strncpy(device.address, "192.168.1.100", sizeof(device.address));
device.port = 161;
device.protocol = IDCU_PROTOCOL_SNMP;
device.collect_interval_ms = 10000;
device.enabled = 1;

idcu_MetricDef cpu_metric = {
    .name = "cpu_usage",
    .description = "CPU usage percentage",
    .type = IDCU_METRIC_TYPE_GAUGE,
    .unit = "%",
    .min_value = 0,
    .max_value = 100
};
idcu_device_add_metric(&device, &cpu_metric);

idcu_device_collector_register_device(&collector, &device);
```

### 启动采集

```c
idcu_device_collector_start(&collector);
```

### 获取采集数据

```c
idcu_Vector values;
idcu_vector_init(&values, sizeof(idcu_MetricValue));

idcu_device_collector_get_metrics(&collector, device.id, &values);

for (size_t i = 0; i < values.size; i++) {
    idcu_MetricValue* val = (idcu_MetricValue*)idcu_vector_get(&values, i);
    printf("%s: %.2f\n", val->name, val->value_double);
}

idcu_vector_destroy(&values);
```

### 手动触发采集

```c
idcu_device_collector_collect_now(&collector, device.id);
```

### 导出数据

```c
char buffer[8192];
idcu_device_collector_export_metrics(&collector, buffer, sizeof(buffer), "json");
printf("%s\n", buffer);
```

### 停止采集器

```c
idcu_device_collector_stop(&collector);
idcu_device_collector_destroy(&collector);
```

## 设备类型

| 类型 | 说明 |
|-----|------|
| SERVER | 服务器 |
| NETWORK | 网络设备 |
| IOT | IoT设备 |
| CUSTOM | 自定义设备 |

## 协议类型

| 协议 | 说明 |
|-----|------|
| SNMP | SNMP协议 |
| HTTP | HTTP协议 |
| MODBUS | Modbus协议 |
| TCP | TCP协议 |
| UDP | UDP协议 |
| CUSTOM | 自定义协议 |

## 指标类型

| 类型 | 说明 |
|-----|------|
| GAUGE | 仪表盘（可上下浮动） |
| COUNTER | 计数器（只增不减） |
| HISTOGRAM | 直方图 |
| STRING | 字符串 |
| BOOLEAN | 布尔值 |

## API 文档

详见 [include/idcu/device_collector/device_collector.h](include/idcu/device_collector/device_collector.h)
```

## 验证检查清单

- [ ] 设备采集头文件已创建
- [ ] 设备采集实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以注册和管理设备
- [ ] 数据采集功能正常工作
- [ ] 数据导出功能正常工作

## Git 提交

```bash
git add libs/idcu-device-collector/
git commit -m "feat: add idcu-device-collector library

- Add multiple device types support
- Add multiple collection protocols (SNMP, HTTP, Modbus)
- Add data collection and aggregation
- Add data filtering and transformation
- Add scheduled collection
- Add data caching
- Add data export
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 设备连接失败 | 网络问题或认证错误 | 检查网络连接和认证信息 |
| 采集超时 | 设备响应慢或间隔太短 | 增加超时时间或采集间隔 |
| 数据不准确 | 协议实现问题 | 检查协议处理代码 |
