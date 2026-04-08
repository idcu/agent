# 任务 4.8: SDK 完善

## 目标

完善 SDK，提供完整的模块开发接口，包括：
- 完整的模块生命周期管理
- 完整的消息通信接口
- 完整的配置访问接口
- 完整的日志接口
- 高级功能接口

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/core/sdk-complete/include
mkdir -p modules/core/sdk-complete/src
```

### 2. 创建完善的 SDK 头文件 (sdk_complete.h)

```c
#ifndef IDCU_SDK_COMPLETE_H
#define IDCU_SDK_COMPLETE_H

#include "sdk.h"
#include "idcu/common/error_code.h"
#include "idcu/coroutine/coroutine.h"
#include "idcu/msgbus/msgbus.h"

#ifdef __cplusplus
extern "C" {
#endif

// 扩展的模块生命周期
typedef int (*idcu_SdkPauseFunc)(idcu_SdkContext* ctx);
typedef int (*idcu_SdkResumeFunc)(idcu_SdkContext* ctx);

// 扩展的模块定义
typedef struct {
    const char* name;
    const char* version;
    const char* description;
    const char* author;
    const char* license;
    idcu_SdkInitFunc init;
    idcu_SdkStartFunc start;
    idcu_SdkPauseFunc pause;
    idcu_SdkResumeFunc resume;
    idcu_SdkRunFunc run;
    idcu_SdkStopFunc stop;
    idcu_SdkDestroyFunc destroy;
} idcu_SdkModuleDefEx;

// 扩展的 SDK 功能
int idcu_sdk_create_coroutine(idcu_SdkContext* ctx, idcu_CoroutineFunc func, void* arg);
int idcu_sdk_publish_message_with_reply(idcu_SdkContext* ctx, idcu_MsgTopic topic,
                                         const void* data, size_t data_size,
                                         void* reply_data, size_t* reply_size,
                                         uint32_t timeout_ms);
int idcu_sdk_register_service(idcu_SdkContext* ctx, const char* service_name,
                               void* service_impl);
void* idcu_sdk_get_service(idcu_SdkContext* ctx, const char* service_name);

// 配置管理扩展
int idcu_sdk_reload_config(idcu_SdkContext* ctx);
int idcu_sdk_save_config(idcu_SdkContext* ctx);

// 性能监控
uint64_t idcu_sdk_get_uptime_ms(idcu_SdkContext* ctx);
uint64_t idcu_sdk_get_memory_usage(idcu_SdkContext* ctx);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-sdk-complete C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-sdk-complete STATIC
    src/sdk_complete.c
)

target_include_directories(idcu-sdk-complete PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-sdk-complete PUBLIC
    idcu::sdk
    idcu::coroutine
    idcu::msgbus
)
```

### 4. 创建模块配置文件 (module.yaml)

```yaml
name: sdk-complete
version: 1.0.0
description: Complete SDK with advanced features
author: IDCU Team
license: MIT

dependencies:
  - idcu-sdk
  - idcu-coroutine
  - idcu-msgbus

build:
  type: cmake
  targets:
    - idcu-sdk-complete

headers:
  - sdk_complete.h

features:
  - extended_lifecycle: Extended module lifecycle (pause/resume)
  - coroutine_integration: Coroutine integration in SDK
  - service_registry: Service registry pattern
  - advanced_config: Advanced configuration management
  - performance_monitoring: Performance monitoring APIs

testing:
  enabled: true
  framework: internal
```

## 验证检查清单

- [ ] 头文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建（YAML 默认格式）
- [ ] README.md 已创建
- [ ] 代码可以成功编译
- [ ] 扩展 SDK 功能正常
- [ ] 安全检查清单已通过

## Git 提交

```bash
git add modules/core/sdk-complete/
git commit -m "feat: add complete SDK

- Add extended module lifecycle management
- Add coroutine integration
- Add service registry
- Add advanced configuration management
- Add performance monitoring
- Add CMake build configuration
- Add module.yaml metadata with YAML format"
```
