# 任务 2.6: 微内核核心

## 目标

创建微内核核心，整合所有核心组件，包括：
- 模块系统整合
- 协程调度器整合
- 消息总线整合
- 核心生命周期管理
- 系统启动和关闭流程

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/core/micro-kernel/include
mkdir -p modules/core/micro-kernel/src
mkdir -p modules/core/micro-kernel/tests
```

### 2. 创建微内核头文件 (micro_kernel.h)

创建 `modules/core/micro-kernel/include/micro_kernel.h`：

```c
#ifndef IDCU_MICRO_KERNEL_H
#define IDCU_MICRO_KERNEL_H

#include "idcu/common/error_code.h"
#include "idcu/coroutine/coroutine.h"
#include "idcu/msgbus/msgbus.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct idcu_MicroKernel idcu_MicroKernel;

typedef enum {
    IDCU_KERNEL_STOPPED,
    IDCU_KERNEL_STARTING,
    IDCU_KERNEL_RUNNING,
    IDCU_KERNEL_STOPPING
} idcu_KernelState;

// 内核配置
typedef struct {
    const char* config_path;
    bool enable_logging;
    int log_level;
    size_t max_modules;
} idcu_KernelConfig;

// 内核初始化和生命周期
int  idcu_kernel_init(idcu_MicroKernel** kernel, const idcu_KernelConfig* config);
void idcu_kernel_destroy(idcu_MicroKernel* kernel);
int  idcu_kernel_start(idcu_MicroKernel* kernel);
int  idcu_kernel_stop(idcu_MicroKernel* kernel);
idcu_KernelState idcu_kernel_get_state(const idcu_MicroKernel* kernel);

// 核心组件访问
idcu_CoroutineScheduler* idcu_kernel_get_scheduler(idcu_MicroKernel* kernel);
idcu_MsgBus* idcu_kernel_get_msgbus(idcu_MicroKernel* kernel);

// 运行循环
int  idcu_kernel_run(idcu_MicroKernel* kernel);
int  idcu_kernel_step(idcu_MicroKernel* kernel);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/core/micro-kernel/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-micro-kernel C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-micro-kernel STATIC
    src/micro_kernel.c
)

target_include_directories(idcu-micro-kernel PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-micro-kernel PUBLIC
    idcu::common
    idcu::coroutine
    idcu::msgbus
)

add_library(idcu::micro-kernel ALIAS idcu-micro-kernel)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/core/micro-kernel/module.yaml`：

```yaml
name: idcu-micro-kernel
version: 1.0.0
description: Micro kernel core for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-coroutine
  - idcu-msgbus

build:
  type: cmake
  targets:
    - idcu-micro-kernel

headers:
  - micro_kernel.h

features:
  - kernel_lifecycle: Complete kernel lifecycle management
  - component_integration: Integration of all core components
  - main_loop: Main event loop and step execution

testing:
  enabled: true
  framework: internal

security:
  notes: |
    - Proper cleanup of all kernel resources
    - Validation of all configuration parameters
    - Graceful shutdown handling
```

## 验证检查清单

- [ ] 头文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建（YAML 默认格式）
- [ ] README.md 已创建
- [ ] 代码可以成功编译
- [ ] 内核初始化和启动功能正常
- [ ] 核心组件正确整合
- [ ] 安全检查清单已通过

## Git 提交

```bash
git add modules/core/micro-kernel/
git commit -m "feat: add micro-kernel core

- Add kernel lifecycle management
- Add integration of all core components
- Add main event loop
- Add CMake build configuration
- Add module.yaml metadata with YAML format"
```
