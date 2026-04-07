# Basic Libraries Integration

## 功能说明

本模块提供统一的基础库集成层，将多个常用的 IDCU 基础库整合到一个模块中，通过编译选项或配置控制启用哪些库。

### 主要功能
- 统一的初始化和销毁接口
- 可配置的库启用/禁用
- 配置管理集成
- 日志系统集成
- JSON 处理集成
- 指标监控集成
- 健康检查集成
- HTTP 服务端/客户端集成
- 网络层集成

## 编译选项

可以通过以下 CMake 选项控制集成的库：

```cmake
option(ENABLE_CONFIG_INTEGRATION "Enable config library integration" ON)
option(ENABLE_LOG_INTEGRATION "Enable log library integration" ON)
option(ENABLE_JSON_INTEGRATION "Enable JSON library integration" ON)
option(ENABLE_METRICS_INTEGRATION "Enable metrics library integration" ON)
option(ENABLE_HEALTHCHECK_INTEGRATION "Enable healthcheck library integration" ON)
option(ENABLE_HTTP_SERVER_INTEGRATION "Enable HTTP server integration" ON)
option(ENABLE_HTTP_CLIENT_INTEGRATION "Enable HTTP client integration" ON)
option(ENABLE_NETWORK_INTEGRATION "Enable network layer integration" ON)
```

## 使用方式

### 1. 初始化集成库

```c
#include "basic_libs.h"

idcu_BasicLibs libs;
int ret = idcu_basic_libs_init(&libs, "config/agent.cfg", "app.log", IDCU_LOG_INFO);
if (ret != IDCU_ERR_OK) {
    // 处理错误
}
```

### 2. 运行时控制库的启用

```c
// 禁用某些库（需要在初始化前调用）
idcu_basic_libs_enable_metrics(0);  // 禁用指标监控
idcu_basic_libs_enable_http_server(0);  // 禁用HTTP服务端
```

### 3. 配置热更新

```c
// 重新加载配置
idcu_basic_libs_config_reload();

// 保存配置
idcu_basic_libs_config_save("new_config.cfg");
```

### 4. 日志级别控制

```c
// 设置日志级别
idcu_basic_libs_log_set_level(IDCU_LOG_DEBUG);

// 获取当前日志级别
idcu_LogLevel level = idcu_basic_libs_log_get_level();
```

### 5. 销毁集成库

```c
idcu_basic_libs_destroy(&libs);
```

## 配置文件

可以通过配置文件控制各库的启用：

```ini
[basic_libs]
enable_config = true
enable_log = true
enable_json = true
enable_metrics = true
enable_healthcheck = true
enable_http_server = true
enable_http_client = true
enable_network = true
```

## 依赖关系

- `idcu-common`: 通用工具和错误码
- `idcu-log`: 日志系统
- `idcu-config`: 配置管理
- `idcu-json`: JSON 处理
- `idcu-metrics`: 指标监控
- `idcu-healthcheck`: 健康检查
- `idcu-http-server`: HTTP 服务端
- `idcu-http-client`: HTTP 客户端
- `idcu-network`: 网络层
- `idcu-module-system`: 模块系统
