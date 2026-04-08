# 任务 2.7: SDK 基础

## 目标

创建 SDK（软件开发工具包）基础，简化模块开发，包括：
- 模块开发接口
- 模块生命周期管理
- 日志接口封装
- 消息发送和接收封装
- 配置访问封装

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/core/sdk/include
mkdir -p modules/core/sdk/src
mkdir -p modules/core/sdk/tests
```

### 2. 创建 SDK 头文件 (sdk.h)

创建 `modules/core/sdk/include/sdk.h`：

```c
#ifndef IDCU_SDK_H
#define IDCU_SDK_H

#include "idcu/common/error_code.h"
#include "idcu/log/log.h"
#include "idcu/msgbus/msgbus.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct idcu_SdkContext idcu_SdkContext;

// 模块生命周期函数
typedef int (*idcu_SdkInitFunc)(idcu_SdkContext* ctx);
typedef int (*idcu_SdkStartFunc)(idcu_SdkContext* ctx);
typedef int (*idcu_SdkRunFunc)(idcu_SdkContext* ctx);
typedef int (*idcu_SdkStopFunc)(idcu_SdkContext* ctx);
typedef void (*idcu_SdkDestroyFunc)(idcu_SdkContext* ctx);

// 模块定义
typedef struct {
    const char* name;
    const char* version;
    const char* description;
    idcu_SdkInitFunc init;
    idcu_SdkStartFunc start;
    idcu_SdkRunFunc run;
    idcu_SdkStopFunc stop;
    idcu_SdkDestroyFunc destroy;
} idcu_SdkModuleDef;

// SDK 日志接口
void idcu_sdk_log(idcu_SdkContext* ctx, idcu_LogLevel level, const char* fmt, ...);
void idcu_sdk_log_debug(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_info(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_warn(idcu_SdkContext* ctx, const char* fmt, ...);
void idcu_sdk_log_error(idcu_SdkContext* ctx, const char* fmt, ...);

// SDK 消息接口
int idcu_sdk_publish_message(idcu_SdkContext* ctx, idcu_MsgTopic topic,
                             const void* data, size_t data_size,
                             idcu_MsgPriority priority);
int idcu_sdk_subscribe_message(idcu_SdkContext* ctx, idcu_MsgTopic topic,
                                idcu_MsgHandler handler, void* user_data);

// SDK 配置接口
int idcu_sdk_get_config_string(idcu_SdkContext* ctx, const char* key,
                               char* out_value, size_t out_value_size);
int idcu_sdk_get_config_int(idcu_SdkContext* ctx, const char* key, int* out_value);
int idcu_sdk_get_config_bool(idcu_SdkContext* ctx, const char* key, bool* out_value);

// SDK 用户数据
void* idcu_sdk_get_user_data(idcu_SdkContext* ctx);
void idcu_sdk_set_user_data(idcu_SdkContext* ctx, void* data);

// 模块定义宏
#define IDCU_SDK_MODULE_DEFINE(name, ver, desc, init_fn, start_fn, run_fn, stop_fn, destroy_fn) \
    static idcu_SdkModuleDef _module_def_##name = { \
        #name, \
        ver, \
        desc, \
        init_fn, \
        start_fn, \
        run_fn, \
        stop_fn, \
        destroy_fn \
    }; \
    idcu_SdkModuleDef* idcu_get_module_def_##name(void) { \
        return &_module_def_##name; \
    }

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/core/sdk/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-sdk C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-sdk STATIC
    src/sdk.c
)

target_include_directories(idcu-sdk PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-sdk PUBLIC
    idcu::common
    idcu::log
    idcu::msgbus
)

add_library(idcu::sdk ALIAS idcu-sdk)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/core/sdk/module.yaml`：

```yaml
name: idcu-sdk
version: 1.0.0
description: Software Development Kit for IDCU Agent modules
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-log
  - idcu-msgbus

build:
  type: cmake
  targets:
    - idcu-sdk

headers:
  - sdk.h

features:
  - module_lifecycle: Simplified module lifecycle management
  - logging: Easy-to-use logging interface
  - messaging: Simplified message publishing and subscription
  - config: Configuration access interface
  - user_data: User data storage

testing:
  enabled: true
  framework: internal

security:
  notes: |
    - Module definition macros are type-safe
    - All SDK functions validate input parameters
    - Logging functions do not expose sensitive information
```

## 验证检查清单

- [ ] 头文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建（YAML 默认格式）
- [ ] README.md 已创建
- [ ] 代码可以成功编译
- [ ] 基本 SDK 功能正常
- [ ] 模块定义宏工作正常
- [ ] 安全检查清单已通过

## Git 提交

```bash
git add modules/core/sdk/
git commit -m "feat: add SDK base

- Add module lifecycle management
- Add logging interface
- Add messaging interface
- Add configuration access interface
- Add CMake build configuration
- Add module.yaml metadata with YAML format"
```
