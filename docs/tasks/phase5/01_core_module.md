# 任务 5.1: core-module - 核心基础模块

## 目标

创建核心基础模块，支持：
- 模块初始化和清理
- 配置加载和管理
- 日志初始化
- 模块生命周期管理
- 模块依赖管理
- 健康检查集成
- 指标收集集成
- 优雅关闭
- 信号处理

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/core-module/include/idcu/core_module
mkdir -p modules/core-module/src/idcu/core_module
mkdir -p modules/core-module/tests
mkdir -p modules/core-module/examples
```

### 2. 创建核心模块头文件 (core_module.h)

创建 `modules/core-module/include/idcu/core_module/core_module.h`：

```c
#ifndef IDCU_CORE_MODULE_CORE_MODULE_H
#define IDCU_CORE_MODULE_CORE_MODULE_H

#include "idcu/common/error_code.h"
#include "idcu/log/log.h"
#include "idcu/config/config.h"
#include "idcu/healthcheck/healthcheck.h"
#include "idcu/metrics/metrics.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_CoreModuleId;

typedef enum
{
    IDCU_CORE_MODULE_STATE_UNINITIALIZED = 0,
    IDCU_CORE_MODULE_STATE_INITIALIZING,
    IDCU_CORE_MODULE_STATE_INITIALIZED,
    IDCU_CORE_MODULE_STATE_STARTING,
    IDCU_CORE_MODULE_STATE_RUNNING,
    IDCU_CORE_MODULE_STATE_STOPPING,
    IDCU_CORE_MODULE_STATE_STOPPED,
    IDCU_CORE_MODULE_STATE_ERROR
} idcu_CoreModuleState;

typedef enum
{
    IDCU_CORE_MODULE_SIGNAL_TERM = 0,
    IDCU_CORE_MODULE_SIGNAL_INT,
    IDCU_CORE_MODULE_SIGNAL_HUP,
    IDCU_CORE_MODULE_SIGNAL_USR1,
    IDCU_CORE_MODULE_SIGNAL_USR2
} idcu_CoreModuleSignal;

typedef int (*idcu_CoreModuleInitFunc)(void* user_data);
typedef int (*idcu_CoreModuleStartFunc)(void* user_data);
typedef int (*idcu_CoreModuleStopFunc)(void* user_data);
typedef int (*idcu_CoreModuleCleanupFunc)(void* user_data);
typedef void (*idcu_CoreModuleSignalHandler)(idcu_CoreModuleSignal signal, void* user_data);

typedef struct
{
    char name[128];
    char version[64];
    char description[512];
    char config_path[1024];
    char log_path[1024];
    idcu_LogLevel log_level;
    int enable_healthcheck;
    int enable_metrics;
    int enable_signals;
    int graceful_shutdown_timeout_ms;
} idcu_CoreModuleConfig;

typedef struct
{
    idcu_CoreModuleId id;
    char name[128];
    char version[64];
    idcu_CoreModuleState state;
    
    idcu_CoreModuleConfig config;
    
    idcu_ConfigManager config_manager;
    idcu_Logger logger;
    idcu_HealthCheck health_check;
    idcu_MetricsRegistry metrics_registry;
    
    idcu_CoreModuleInitFunc init_func;
    idcu_CoreModuleStartFunc start_func;
    idcu_CoreModuleStopFunc stop_func;
    idcu_CoreModuleCleanupFunc cleanup_func;
    void* module_user_data;
    
    idcu_CoreModuleSignalHandler signal_handler;
    void* signal_user_data;
    
    uint64_t started_at;
    uint64_t uptime_ms;
    
    int initialized;
} idcu_CoreModule;

int  idcu_core_module_config_init(idcu_CoreModuleConfig* config);

int  idcu_core_module_init(idcu_CoreModule* module, const idcu_CoreModuleConfig* config);
void idcu_core_module_destroy(idcu_CoreModule* module);

int  idcu_core_module_set_init_func(idcu_CoreModule* module, idcu_CoreModuleInitFunc func, void* user_data);
int  idcu_core_module_set_start_func(idcu_CoreModule* module, idcu_CoreModuleStartFunc func, void* user_data);
int  idcu_core_module_set_stop_func(idcu_CoreModule* module, idcu_CoreModuleStopFunc func, void* user_data);
int  idcu_core_module_set_cleanup_func(idcu_CoreModule* module, idcu_CoreModuleCleanupFunc func, void* user_data);
int  idcu_core_module_set_signal_handler(idcu_CoreModule* module, idcu_CoreModuleSignalHandler handler, void* user_data);

int  idcu_core_module_load_config(idcu_CoreModule* module, const char* path);
int  idcu_core_module_reload_config(idcu_CoreModule* module);
int  idcu_core_module_save_config(idcu_CoreModule* module);

int  idcu_core_module_start(idcu_CoreModule* module);
int  idcu_core_module_stop(idcu_CoreModule* module);
int  idcu_core_module_restart(idcu_CoreModule* module);

idcu_CoreModuleState idcu_core_module_get_state(idcu_CoreModule* module);
const char* idcu_core_module_get_state_string(idcu_CoreModuleState state);

idcu_ConfigManager* idcu_core_module_get_config(idcu_CoreModule* module);
idcu_Logger* idcu_core_module_get_logger(idcu_CoreModule* module);
idcu_HealthCheck* idcu_core_module_get_healthcheck(idcu_CoreModule* module);
idcu_MetricsRegistry* idcu_core_module_get_metrics(idcu_CoreModule* module);

int  idcu_core_module_add_health_check(idcu_CoreModule* module, const char* name, idcu_HealthCheckFunc func, void* user_data);
int  idcu_core_module_register_metric(idcu_CoreModule* module, idcu_Metric* metric);

int  idcu_core_module_get_uptime(idcu_CoreModule* module, uint64_t* uptime_ms);
int  idcu_core_module_get_start_time(idcu_CoreModule* module, uint64_t* timestamp_ms);

int  idcu_core_module_register_signal_handlers(idcu_CoreModule* module);
int  idcu_core_module_unregister_signal_handlers(idcu_CoreModule* module);
int  idcu_core_module_send_signal(idcu_CoreModule* module, idcu_CoreModuleSignal signal);

int  idcu_core_module_enable_healthcheck(idcu_CoreModule* module);
int  idcu_core_module_disable_healthcheck(idcu_CoreModule* module);
int  idcu_core_module_get_health_status(idcu_CoreModule* module, idcu_HealthStatus* status);

int  idcu_core_module_enable_metrics(idcu_CoreModule* module);
int  idcu_core_module_disable_metrics(idcu_CoreModule* module);
int  idcu_core_module_export_metrics(idcu_CoreModule* module, char* buffer, size_t buffer_size);

int  idcu_core_module_set_log_level(idcu_CoreModule* module, idcu_LogLevel level);
idcu_LogLevel idcu_core_module_get_log_level(idcu_CoreModule* module);

int  idcu_core_module_graceful_shutdown(idcu_CoreModule* module, uint64_t timeout_ms);
int  idcu_core_module_force_shutdown(idcu_CoreModule* module);

int  idcu_core_module_get_info(idcu_CoreModule* module, char* buffer, size_t buffer_size);
int  idcu_core_module_get_info_json(idcu_CoreModule* module, char* buffer, size_t buffer_size);

typedef struct
{
    idcu_Vector dependencies;
    idcu_Vector modules;
    idcu_Mutex lock;
    int initialized;
} idcu_ModuleManager;

int  idcu_module_manager_init(idcu_ModuleManager* manager);
void idcu_module_manager_destroy(idcu_ModuleManager* manager);

int  idcu_module_manager_register(idcu_ModuleManager* manager, idcu_CoreModule* module);
int  idcu_module_manager_unregister(idcu_ModuleManager* manager, idcu_CoreModuleId id);
idcu_CoreModule* idcu_module_manager_get(idcu_ModuleManager* manager, idcu_CoreModuleId id);
idcu_CoreModule* idcu_module_manager_get_by_name(idcu_ModuleManager* manager, const char* name);

int  idcu_module_manager_add_dependency(idcu_ModuleManager* manager, idcu_CoreModuleId module_id, idcu_CoreModuleId dependency_id);
int  idcu_module_manager_remove_dependency(idcu_ModuleManager* manager, idcu_CoreModuleId module_id, idcu_CoreModuleId dependency_id);

int  idcu_module_manager_start_all(idcu_ModuleManager* manager);
int  idcu_module_manager_stop_all(idcu_ModuleManager* manager);
int  idcu_module_manager_restart_all(idcu_ModuleManager* manager);

size_t idcu_module_manager_count(idcu_ModuleManager* manager);
int  idcu_module_manager_get_all(idcu_ModuleManager* manager, idcu_Vector* modules);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/core-module/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(core-module VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(core-module STATIC
    src/idcu/core_module/core_module.c
    src/idcu/core_module/module_manager.c
)

target_include_directories(core-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(core-module PRIVATE
    idcu::common
    idcu::log
    idcu::config
    idcu::healthcheck
    idcu::metrics
    idcu::utils
    idcu::json
)

add_library(idcu::core-module ALIAS core-module)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/core-module/module.yaml`：

```yaml
name: core-module
version: 1.0.0
description: Core base module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-log
  - idcu-config
  - idcu-healthcheck
  - idcu-metrics
  - idcu-utils
  - idcu-json

build:
  type: cmake
  targets:
    - core-module

headers:
  - idcu/core_module/core_module.h

features:
  - lifecycle: Module lifecycle management
  - config: Config loading and management
  - logging: Logging initialization
  - dependencies: Module dependency management
  - healthcheck: Health check integration
  - metrics: Metrics collection integration
  - graceful_shutdown: Graceful shutdown
  - signals: Signal handling
  - module_manager: Module manager

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `modules/core-module/README.md`：

```markdown
# core-module

IDCU Agent 的核心基础模块。

## 功能特性

- **模块生命周期**: 模块初始化和清理
- **配置管理**: 配置加载和管理
- **日志初始化**: 日志初始化
- **依赖管理**: 模块生命周期管理
- **依赖管理**: 模块依赖管理
- **健康检查**: 健康检查集成
- **指标收集**: 指标收集集成
- **优雅关闭**: 优雅关闭
- **信号处理**: 信号处理
- **模块管理器**: 模块管理器

## 快速开始

### 初始化核心模块

```c
#include "idcu/core_module/core_module.h"

idcu_CoreModuleConfig config;
idcu_core_module_config_init(&config);

strncpy(config.name, "my-app", sizeof(config.name));
strncpy(config.version, "1.0.0", sizeof(config.version));
strncpy(config.config_path, "./config.yaml", sizeof(config.config_path));
strncpy(config.log_path, "./logs/app.log", sizeof(config.log_path));
config.log_level = IDCU_LOG_LEVEL_INFO;
config.enable_healthcheck = 1;
config.enable_metrics = 1;
config.enable_signals = 1;
config.graceful_shutdown_timeout_ms = 5000;

idcu_CoreModule module;
idcu_core_module_init(&module, &config);
```

### 设置生命周期函数

```c
int my_init(void* user_data)
{
    printf("Initializing...\n");
    return 0;
}

int my_start(void* user_data)
{
    printf("Starting...\n");
    return 0;
}

int my_stop(void* user_data)
{
    printf("Stopping...\n");
    return 0;
}

int my_cleanup(void* user_data)
{
    printf("Cleaning up...\n");
    return 0;
}

idcu_core_module_set_init_func(&module, my_init, NULL);
idcu_core_module_set_start_func(&module, my_start, NULL);
idcu_core_module_set_stop_func(&module, my_stop, NULL);
idcu_core_module_set_cleanup_func(&module, my_cleanup, NULL);
```

### 设置信号处理器

```c
void signal_handler(idcu_CoreModuleSignal signal, void* user_data)
{
    switch (signal) {
        case IDCU_CORE_MODULE_SIGNAL_TERM:
        case IDCU_CORE_MODULE_SIGNAL_INT:
            printf("Received shutdown signal\n");
            break;
        case IDCU_CORE_MODULE_SIGNAL_HUP:
            printf("Received reload signal\n");
            break;
        default:
            break;
    }
}

idcu_core_module_set_signal_handler(&module, signal_handler, NULL);
```

### 加载配置

```c
idcu_core_module_load_config(&module, "./config.yaml");
```

### 启动模块

```c
idcu_core_module_start(&module);
```

### 访问配置

```c
idcu_ConfigManager* config = idcu_core_module_get_config(&module);

char value[256];
idcu_config_get(config, "app.name", value, sizeof(value));
printf("App name: %s\n", value);
```

### 使用日志

```c
idcu_Logger* logger = idcu_core_module_get_logger(&module);

idcu_log_info(logger, "Application started");
idcu_log_warn(logger, "Warning message");
idcu_log_error(logger, "Error message");
```

### 添加健康检查

```c
int db_check(void* user_data)
{
    // Check database connection
    return 0;
}

idcu_core_module_add_health_check(&module, "database", db_check, NULL);
```

### 注册指标

```c
idcu_Counter requests_counter;
idcu_counter_init(&requests_counter, "http_requests_total", "Total HTTP requests");

idcu_core_module_register_metric(&module, &requests_counter.base);

idcu_counter_inc(&requests_counter);
```

### 获取模块状态

```c
idcu_CoreModuleState state = idcu_core_module_get_state(&module);
printf("State: %s\n", idcu_core_module_get_state_string(state));
```

### 获取运行时间

```c
uint64_t uptime;
idcu_core_module_get_uptime(&module, &uptime);
printf("Uptime: %" PRIu64 " ms\n", uptime);
```

### 获取健康状态

```c
idcu_HealthStatus status;
idcu_core_module_get_health_status(&module, &status);
printf("Health: %s\n", idcu_health_status_to_string(status));
```

### 导出指标

```c
char metrics_buffer[4096];
idcu_core_module_export_metrics(&module, metrics_buffer, sizeof(metrics_buffer));
printf("%s\n", metrics_buffer);
```

### 重新加载配置

```c
idcu_core_module_reload_config(&module);
```

### 保存配置

```c
idcu_core_module_save_config(&module);
```

### 设置日志级别

```c
idcu_core_module_set_log_level(&module, IDCU_LOG_LEVEL_DEBUG);
```

### 停止模块

```c
idcu_core_module_graceful_shutdown(&module, 5000);
```

### 强制停止

```c
idcu_core_module_force_shutdown(&module);
```

### 清理

```c
idcu_core_module_destroy(&module);
```

### 使用模块管理器

```c
idcu_ModuleManager manager;
idcu_module_manager_init(&manager);

idcu_module_manager_register(&manager, &module);

idcu_module_manager_add_dependency(&manager, module.id, other_module.id);

idcu_module_manager_start_all(&manager);

idcu_module_manager_stop_all(&manager);

idcu_module_manager_destroy(&manager);
```

### 获取模块信息

```c
char info_buffer[2048];
idcu_core_module_get_info(&module, info_buffer, sizeof(info_buffer));
printf("%s\n", info_buffer);

char json_buffer[4096];
idcu_core_module_get_info_json(&module, json_buffer, sizeof(json_buffer));
printf("%s\n", json_buffer);
```

## 模块状态

| 状态 | 说明 |
|-----|------|
| UNINITIALIZED | 未初始化 |
| INITIALIZING | 初始化中 |
| INITIALIZED | 已初始化 |
| STARTING | 启动中 |
| RUNNING | 运行中 |
| STOPPING | 停止中 |
| STOPPED | 已停止 |
| ERROR | 错误 |

## 信号类型

| 信号 | 说明 |
|-----|------|
| TERM | 终止信号 |
| INT | 中断信号 |
| HUP | 挂起信号 |
| USR1 | 用户信号 1 |
| USR2 | 用户信号 2 |

## API 文档

详见 [include/idcu/core_module/core_module.h](include/idcu/core_module/core_module.h)
```

## 验证检查清单

- [ ] 核心模块头文件已创建
- [ ] 核心模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 模块可以正常初始化和启动
- [ ] 配置加载正常工作
- [ ] 日志正常工作

## Git 提交

```bash
git add modules/core-module/
git commit -m "feat: add core-module module

- Add module initialization and cleanup
- Add config loading and management
- Add logging initialization
- Add module lifecycle management
- Add module dependency management
- Add health check integration
- Add metrics collection integration
- Add graceful shutdown
- Add signal handling
- Add module manager
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 模块启动失败 | 配置错误 | 检查配置文件 |
| 健康检查失败 | 检查函数错误 | 检查健康检查实现 |
| 信号未处理 | 信号处理器未设置 | 确保设置了信号处理器 |
