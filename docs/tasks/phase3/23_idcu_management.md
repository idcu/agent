# 任务 3.23: idcu-management

## 目标

创建管理库，提供 CLI 和 API 管理接口，包括：
- 命令行接口
- HTTP 管理 API
- 模块管理接口
- 配置管理接口
- 系统监控接口

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-management/include/idcu/management
mkdir -p libs/idcu-management/src/idcu/management
mkdir -p libs/idcu-management/tests
mkdir -p libs/idcu-management/examples
```

### 2. 创建管理头文件 (management.h)

```c
#ifndef IDCU_MANAGEMENT_MANAGEMENT_H
#define IDCU_MANAGEMENT_MANAGEMENT_H

#include "idcu/common/error_code.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 管理命令
typedef enum {
    IDCU_MGMT_CMD_STATUS,
    IDCU_MGMT_CMD_MODULE_LIST,
    IDCU_MGMT_CMD_MODULE_START,
    IDCU_MGMT_CMD_MODULE_STOP,
    IDCU_MGMT_CMD_CONFIG_GET,
    IDCU_MGMT_CMD_CONFIG_SET,
    IDCU_MGMT_CMD_METRICS_GET,
    IDCU_MGMT_CMD_HEALTH_CHECK
} idcu_MgmtCommand;

// 管理响应
typedef struct {
    int code;
    char* message;
    char* data;
    size_t data_size;
} idcu_MgmtResponse;

// 管理接口
int  idcu_mgmt_init(void);
void idcu_mgmt_destroy(void);

// 命令执行
int  idcu_mgmt_execute(idcu_MgmtCommand cmd, const char* args,
                       idcu_MgmtResponse* response);
void idcu_mgmt_response_destroy(idcu_MgmtResponse* response);

// CLI 接口
int  idcu_mgmt_cli_start(int port);
void idcu_mgmt_cli_stop(void);

// HTTP API 接口
int  idcu_mgmt_http_start(int port);
void idcu_mgmt_http_stop(void);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-management C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-management STATIC
    src/idcu/management/management.c
    src/idcu/management/cli.c
    src/idcu/management/http_api.c
)

target_include_directories(idcu-management PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-management PUBLIC
    idcu::common
    idcu::http-server
    idcu::metrics
)

add_library(idcu::management ALIAS idcu-management)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

```yaml
name: idcu-management
version: 1.0.0
description: Management library with CLI and HTTP API
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-http-server
  - idcu-metrics

build:
  type: cmake
  targets:
    - idcu-management

headers:
  - idcu/management/management.h

features:
  - cli: Command line interface
  - http_api: HTTP management API
  - module_management: Module management interface
  - config_management: Configuration management interface
  - monitoring: System monitoring interface

testing:
  enabled: true
  framework: internal

security:
  notes: |
    - HTTP API should use authentication
    - CLI access should be restricted
    - Sensitive operations require authorization
```

## 验证检查清单

- [ ] 头文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建（YAML 默认格式）
- [ ] README.md 已创建
- [ ] 代码可以成功编译
- [ ] 基本管理功能正常
- [ ] 安全检查清单已通过

## Git 提交

```bash
git add libs/idcu-management/
git commit -m "feat: add idcu-management library

- Add management CLI interface
- Add HTTP management API
- Add module and config management
- Add CMake build configuration
- Add module.yaml metadata with YAML format"
```
