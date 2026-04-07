# IDCU Device Collector Module

设备数据采集模块，用于管理和采集物联网设备数据。

## 功能特性

- 设备注册和管理
- 设备状态监控
- 数据点采集和存储
- 回调机制（数据变化、状态变化）
- JSON 格式导出

## 使用示例

```c
#include "idcu/device_collector/device_collector.h"

idcu_DeviceCollector collector;
idcu_device_collector_init(&collector);

idcu_DeviceInfo device;
strcpy(device.name, "sensor-001");
strcpy(device.device_type, "temperature");
strcpy(device.location, "room-101");
device.status = IDCU_DEVICE_STATUS_ONLINE;
device.tag_count = 2;
strcpy(device.tags[0], "indoor");
strcpy(device.tags[1], "floor-1");

idcu_device_collector_register_device(&collector, &device);

idcu_DeviceDataPoint data;
strcpy(data.key, "temperature");
data.type = IDCU_DATA_TYPE_FLOAT;
data.value.float_val = 25.5;
data.timestamp_ms = get_current_time_ms();

idcu_device_collector_add_data_point(&collector, "sensor-001", &data);

char buffer[4096];
size_t size;
idcu_device_collector_export_json(&collector, buffer, sizeof(buffer), &size);

idcu_device_collector_destroy(&collector);
```

## 配置

暂无特殊配置。

## 依赖

- idcu-common
- idcu-log
