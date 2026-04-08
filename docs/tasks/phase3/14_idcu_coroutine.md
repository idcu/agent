# 任务 3.14: idcu-coroutine（独立库）

## 目标

创建独立的协程库，为其他模块提供协程支持，包括：
- 协程创建和管理
- 协程上下文切换
- 协程调度
- 协程同步原语

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-coroutine/include/idcu/coroutine
mkdir -p libs/idcu-coroutine/src/idcu/coroutine
mkdir -p libs/idcu-coroutine/tests
mkdir -p libs/idcu-coroutine/examples
```

### 2. 创建协程头文件 (coroutine.h)

```c
#ifndef IDCU_COROUTINE_COROUTINE_H
#define IDCU_COROUTINE_COROUTINE_H

#include "idcu/common/error_code.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_COROUTINE_STACK_SIZE (64 * 1024)
#define IDCU_COROUTINE_MAX_POOL_SIZE 128

typedef enum {
    IDCU_CORO_READY,
    IDCU_CORO_RUNNING,
    IDCU_CORO_WAITING,
    IDCU_CORO_SUSPENDED,
    IDCU_CORO_FINISHED
} idcu_CoroutineState;

typedef struct idcu_Coroutine idcu_Coroutine;
typedef struct idcu_CoroutineScheduler idcu_CoroutineScheduler;

typedef void (*idcu_CoroutineFunc)(void* arg);

// 协程调度器
int  idcu_coro_scheduler_init(idcu_CoroutineScheduler** scheduler);
void idcu_coro_scheduler_destroy(idcu_CoroutineScheduler* scheduler);
int  idcu_coro_scheduler_run(idcu_CoroutineScheduler* scheduler);
int  idcu_coro_scheduler_stop(idcu_CoroutineScheduler* scheduler);

// 协程创建
int  idcu_coro_create(idcu_CoroutineScheduler* scheduler, idcu_Coroutine** coro,
                      idcu_CoroutineFunc func, void* arg);
void idcu_coro_destroy(idcu_Coroutine* coro);

// 协程控制
int  idcu_coro_yield(void);
int  idcu_coro_suspend(idcu_Coroutine* coro);
int  idcu_coro_resume(idcu_Coroutine* coro);

// 协程状态
idcu_CoroutineState idcu_coro_get_state(const idcu_Coroutine* coro);
const char* idcu_coro_state_to_str(idcu_CoroutineState state);

// 当前协程
idcu_Coroutine* idcu_coro_current(void);

// 睡眠
int  idcu_coro_sleep_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-coroutine C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-coroutine STATIC
    src/idcu/coroutine/coroutine.c
)

target_include_directories(idcu-coroutine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-coroutine PUBLIC
    idcu::common
)

if(WIN32)
    target_compile_definitions(idcu-coroutine PRIVATE _WIN32_WINNT=0x0601)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(idcu-coroutine PRIVATE Threads::Threads)
endif()

add_library(idcu::coroutine ALIAS idcu-coroutine)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

```yaml
name: idcu-coroutine
version: 1.0.0
description: Standalone coroutine library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common

build:
  type: cmake
  targets:
    - idcu-coroutine

headers:
  - idcu/coroutine/coroutine.h

features:
  - coroutine: Lightweight coroutine implementation
  - scheduler: Simple coroutine scheduler
  - context_switch: Fast context switching

testing:
  enabled: true
  framework: internal

security:
  notes: |
    - Stack size limits prevent stack overflow
    - Proper cleanup of coroutine resources
    - No unsafe memory operations
```

## 验证检查清单

- [ ] 头文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建（YAML 默认格式）
- [ ] README.md 已创建
- [ ] 代码可以成功编译
- [ ] 基本协程功能正常
- [ ] 安全检查清单已通过

## Git 提交

```bash
git add libs/idcu-coroutine/
git commit -m "feat: add idcu-coroutine standalone library

- Add standalone coroutine implementation
- Add coroutine scheduler
- Add CMake build configuration
- Add module.yaml metadata with YAML format"
```
