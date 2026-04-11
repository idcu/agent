# idcu-discovery

IDCU Agent 的服务发现库。

## 功能特性
- 服务注册和发现
- 心跳机制
- 服务列表维护
- 线程安全实现

## 使用方法

```c
#include <idcu/discovery/discovery.h>

idcu_Discovery* disc;
int ret = idcu_discovery_init(&disc);
if (ret == IDCU_ERR_OK) {
    // 注册服务
    idcu_discovery_register(disc, "my-service", "127.0.0.1", 8080);
    
    // 发现服务
    idcu_ServiceInfo* services;
    size_t count;
    idcu_discovery_discover(disc, "my-service", &services, &count);
    
    // 清理
    idcu_discovery_destroy(disc);
}
```

## API 参考
- `idcu_discovery_init()` - 初始化服务发现
- `idcu_discovery_destroy()` - 销毁服务发现
- `idcu_discovery_is_initialized()` - 检查是否已初始化
- `idcu_discovery_register()` - 注册服务
- `idcu_discovery_unregister()` - 注销服务
- `idcu_discovery_discover()` - 发现服务
- `idcu_discovery_get_operation_count()` - 获取操作计数
- `idcu_discovery_get_error_count()` - 获取错误计数

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
