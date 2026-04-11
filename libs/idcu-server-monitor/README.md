# idcu-server-monitor

IDCU Agent 的服务器监控库。

## 功能特性
- 服务器性能监控
- 资源使用统计
- 健康状态检查
- 线程安全实现

## 使用方法

```c
#include <idcu/server-monitor/server-monitor.h>

idcu_ServerMonitor* monitor;
int ret = idcu_server_monitor_init(&monitor);
if (ret == IDCU_ERR_OK) {
    // 获取服务器信息
    idcu_ServerInfo info;
    idcu_server_monitor_get_info(monitor, &info);
    
    // 获取性能统计
    idcu_ServerStats stats;
    idcu_server_monitor_get_stats(monitor, &stats);
    
    // 清理
    idcu_server_monitor_destroy(monitor);
}
```

## API 参考
- `idcu_server_monitor_init()` - 初始化服务器监控
- `idcu_server_monitor_destroy()` - 销毁服务器监控
- `idcu_server_monitor_is_initialized()` - 检查是否已初始化
- `idcu_server_monitor_get_info()` - 获取服务器信息
- `idcu_server_monitor_get_stats()` - 获取性能统计
- `idcu_server_monitor_get_operation_count()` - 获取操作计数
- `idcu_server_monitor_get_error_count()` - 获取错误计数

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
