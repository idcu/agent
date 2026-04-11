# idcu-device-collector

IDCU Agent 的设备采集库。

## 功能特性
- 设备信息采集
- 设备状态监控
- 设备数据上报
- 线程安全实现

## 使用方法

```c
#include <idcu/device-collector/device-collector.h>

idcu_DeviceCollector* collector;
int ret = idcu_device_collector_init(&collector);
if (ret == IDCU_ERR_OK) {
    // 采集设备信息
    idcu_DeviceInfo info;
    idcu_device_collector_collect(collector, &info);
    
    // 获取设备状态
    idcu_DeviceStatus status;
    idcu_device_collector_get_status(collector, &status);
    
    // 清理
    idcu_device_collector_destroy(collector);
}
```

## API 参考
- `idcu_device_collector_init()` - 初始化设备采集器
- `idcu_device_collector_destroy()` - 销毁设备采集器
- `idcu_device_collector_is_initialized()` - 检查是否已初始化
- `idcu_device_collector_collect()` - 采集设备信息
- `idcu_device_collector_get_status()` - 获取设备状态
- `idcu_device_collector_get_operation_count()` - 获取操作计数
- `idcu_device_collector_get_error_count()` - 获取错误计数

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
