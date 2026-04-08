# 任务 4.7: basic-libs（基础库统一集成）

## 目标

创建基础库统一集成模块，将所有基础库整合到模块系统中，包括：
- 所有基础库的集成
- 统一的初始化和清理
- 统一的配置管理
- 统一的日志管理

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/integrations/basic-libs/include
mkdir -p modules/integrations/basic-libs/src
```

### 2. 创建集成头文件 (basic_libs.h)

```c
#ifndef IDCU_BASIC_LIBS_H
#define IDCU_BASIC_LIBS_H

#include "sdk.h"
#include "idcu/common/error_code.h"

#ifdef __cplusplus
extern "C" {
#endif

// 基础库初始化
int idcu_basic_libs_init(idcu_SdkContext* ctx);
void idcu_basic_libs_destroy(idcu_SdkContext* ctx);

// 获取集成的库
void* idcu_basic_libs_get_logger(void);
void* idcu_basic_libs_get_config(void);
void* idcu_basic_libs_get_json_parser(void);
void* idcu_basic_libs_get_yaml_parser(void);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-basic-libs-integration C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-basic-libs-integration STATIC
    src/basic_libs.c
)

target_include_directories(idcu-basic-libs-integration PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-basic-libs-integration PUBLIC
    idcu::sdk
    idcu::log
    idcu::config
    idcu::json
    idcu::yaml
)
```

### 4. 创建模块配置文件 (module.yaml)

```yaml
name: basic-libs-integration
version: 1.0.0
description: Basic libraries integration module
author: IDCU Team
license: MIT

dependencies:
  - idcu-sdk
  - idcu-log
  - idcu-config
  - idcu-json
  - idcu-yaml

build:
  type: cmake
  targets:
    - idcu-basic-libs-integration

headers:
  - basic_libs.h

features:
  - unified_init: Unified initialization of all basic libraries
  - unified_cleanup: Unified cleanup of all basic libraries
  - library_access: Centralized access to all basic libraries

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
- [ ] 基础库集成功能正常
- [ ] 安全检查清单已通过

## Git 提交

```bash
git add modules/integrations/basic-libs/
git commit -m "feat: add basic-libs integration module

- Add unified basic libraries integration
- Add centralized library access
- Add CMake build configuration
- Add module.yaml metadata with YAML format"
```
