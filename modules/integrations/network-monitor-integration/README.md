# Network-Monitor Integration Module

## 概述

这个模块负责在 IDCU Agent 中连接 Network 和 Monitor 服务模块，提供集成的 Prometheus 指标导出功能。

## 功能

- 初始化和管理 Metrics Collector
- 初始化和管理 Prometheus Exporter
- 提供统一的启动/停止/轮询接口
- 暴露 Metrics Collector 供其他模块使用

## 依赖

- `idcu::common` - 通用工具库
- `idcu::log` - 日志库
- `idcu::network` - 网络层库
- `idcu_services_monitor` - Monitor 服务模块
- `idcu_services_network` - Network 服务模块

## 使用方法

```c
#include "network_monitor_integration.h"

idcu_NetworkMonitorIntegration integration;

// 初始化集成
idcu_network_monitor_integration_init(&integration);

// 启动 Prometheus 导出服务
idcu_network_monitor_integration_start(&integration, "0.0.0.0", 9090);

// 在主循环中轮询
while (running) {
    idcu_network_monitor_integration_poll(&integration, 100);
}

// 停止服务
idcu_network_monitor_integration_stop(&integration);

// 销毁集成
idcu_network_monitor_integration_destroy(&integration);
```

## API 参考

### `idcu_network_monitor_integration_init`

初始化 Network-Monitor 集成模块。

**参数**:
- `integration` - 集成模块实例指针

**返回值**:
- `IDCU_ERR_OK` - 成功
- 其他错误码 - 失败

### `idcu_network_monitor_integration_destroy`

销毁 Network-Monitor 集成模块。

**参数**:
- `integration` - 集成模块实例指针

### `idcu_network_monitor_integration_start`

启动 Prometheus 导出服务。

**参数**:
- `integration` - 集成模块实例指针
- `bind_address` - 绑定地址
- `port` - 绑定端口

**返回值**:
- `IDCU_ERR_OK` - 成功
- 其他错误码 - 失败

### `idcu_network_monitor_integration_stop`

停止 Prometheus 导出服务。

**参数**:
- `integration` - 集成模块实例指针

**返回值**:
- `IDCU_ERR_OK` - 成功
- 其他错误码 - 失败

### `idcu_network_monitor_integration_poll`

轮询处理 Prometheus 导出请求。

**参数**:
- `integration` - 集成模块实例指针
- `timeout_ms` - 超时时间（毫秒）

**返回值**:
- `IDCU_ERR_OK` - 成功
- 其他错误码 - 失败

### `idcu_network_monitor_integration_get_metrics`

获取 Metrics Collector 实例。

**参数**:
- `integration` - 集成模块实例指针

**返回值**:
- Metrics Collector 实例指针，失败时返回 NULL
