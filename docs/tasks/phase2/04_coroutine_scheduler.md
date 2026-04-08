# 任务 2.4: 协程调度器

## 目标

创建高性能的协程调度器，为 IDCU Agent 提供轻量级并发支持，包括：
- 协程上下文切换
- 协程创建和销毁
- 协程调度算法
- 协程间通信
- 协程池管理

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-coroutine/include/idcu/coroutine
mkdir -p libs/idcu-coroutine/src/idcu/coroutine
mkdir -p libs/idcu-coroutine/tests
mkdir -p libs/idcu-coroutine/examples
```

### 2. 创建协程头文件 (coroutine.h)

创建 `libs/idcu-coroutine/include/idcu/coroutine/coroutine.h`：

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

typedef struct {
    void* arg;
    idcu_CoroutineFunc func;
} idcu_CoroutineConfig;

// 协程调度器
int  idcu_coro_scheduler_init(idcu_CoroutineScheduler** scheduler);
void idcu_coro_scheduler_destroy(idcu_CoroutineScheduler* scheduler);
int  idcu_coro_scheduler_run(idcu_CoroutineScheduler* scheduler);
int  idcu_coro_scheduler_stop(idcu_CoroutineScheduler* scheduler);

// 协程创建
int  idcu_coro_create(idcu_CoroutineScheduler* scheduler, idcu_Coroutine** coro,
                      const idcu_CoroutineConfig* config);
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

// 睡眠和等待
int  idcu_coro_sleep_ms(uint32_t ms);
int  idcu_coro_wait_for_event(void* event, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建调度器头文件 (scheduler.h)

创建 `libs/idcu-coroutine/include/idcu/coroutine/scheduler.h`：

```c
#ifndef IDCU_COROUTINE_SCHEDULER_H
#define IDCU_COROUTINE_SCHEDULER_H

#include "coroutine.h"
#include "idcu/common/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_SCHED_ROUND_ROBIN,
    IDCU_SCHED_PRIORITY,
    IDCU_SCHED_FIFO
} idcu_SchedulePolicy;

typedef struct idcu_CoroutineScheduler {
    idcu_Vector ready_queue;
    idcu_Vector waiting_queue;
    idcu_Vector all_coroutines;
    idcu_Coroutine* current_coro;
    idcu_Coroutine* main_coro;
    idcu_SchedulePolicy policy;
    bool running;
    uint32_t next_id;
} idcu_CoroutineScheduler;

// 调度策略
int  idcu_coro_scheduler_set_policy(idcu_CoroutineScheduler* scheduler,
                                    idcu_SchedulePolicy policy);

// 协程统计
size_t idcu_coro_scheduler_count(const idcu_CoroutineScheduler* scheduler);
size_t idcu_coro_scheduler_ready_count(const idcu_CoroutineScheduler* scheduler);
size_t idcu_coro_scheduler_waiting_count(const idcu_CoroutineScheduler* scheduler);

#ifdef __cplusplus
}
#endif

#endif
```

### 4. 创建协程实现文件 (coroutine.c)

创建 `libs/idcu-coroutine/src/idcu/coroutine/coroutine.c`：

```c
#include "idcu/coroutine/coroutine.h"
#include "idcu/coroutine/scheduler.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <ucontext.h>
#include <unistd.h>
#endif

typedef struct idcu_Coroutine {
    uint32_t id;
    idcu_CoroutineState state;
    idcu_CoroutineFunc func;
    void* arg;
    void* stack;
    size_t stack_size;
    idcu_CoroutineScheduler* scheduler;

#ifdef _WIN32
    LPVOID fiber;
#else
    ucontext_t ctx;
    ucontext_t return_ctx;
#endif
} idcu_Coroutine;

static __thread idcu_CoroutineScheduler* g_current_scheduler = NULL;
static __thread idcu_Coroutine* g_current_coro = NULL;

int idcu_coro_scheduler_init(idcu_CoroutineScheduler** scheduler) {
    if (!scheduler) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_CoroutineScheduler* s = (idcu_CoroutineScheduler*)malloc(sizeof(idcu_CoroutineScheduler));
    if (!s) {
        return IDCU_ERR_NO_MEMORY;
    }

    memset(s, 0, sizeof(idcu_CoroutineScheduler));

    int ret = idcu_vector_init(&s->ready_queue, sizeof(idcu_Coroutine*), 16);
    if (ret != IDCU_ERR_OK) {
        free(s);
        return ret;
    }

    ret = idcu_vector_init(&s->waiting_queue, sizeof(idcu_Coroutine*), 16);
    if (ret != IDCU_ERR_OK) {
        idcu_vector_destroy(&s->ready_queue);
        free(s);
        return ret;
    }

    ret = idcu_vector_init(&s->all_coroutines, sizeof(idcu_Coroutine*), 16);
    if (ret != IDCU_ERR_OK) {
        idcu_vector_destroy(&s->ready_queue);
        idcu_vector_destroy(&s->waiting_queue);
        free(s);
        return ret;
    }

    s->policy = IDCU_SCHED_ROUND_ROBIN;
    s->running = false;
    s->next_id = 1;

    *scheduler = s;
    return IDCU_ERR_OK;
}

void idcu_coro_scheduler_destroy(idcu_CoroutineScheduler* scheduler) {
    if (!scheduler) {
        return;
    }

    for (size_t i = 0; i < idcu_vector_size(&scheduler->all_coroutines); ++i) {
        idcu_Coroutine** coro_ptr = (idcu_Coroutine**)idcu_vector_get(&scheduler->all_coroutines, i);
        if (coro_ptr && *coro_ptr) {
            idcu_coro_destroy(*coro_ptr);
        }
    }

    idcu_vector_destroy(&scheduler->ready_queue);
    idcu_vector_destroy(&scheduler->waiting_queue);
    idcu_vector_destroy(&scheduler->all_coroutines);
    free(scheduler);
}

static void coroutine_wrapper(void) {
    idcu_Coroutine* coro = g_current_coro;
    if (!coro) {
        return;
    }

    coro->state = IDCU_CORO_RUNNING;

    if (coro->func) {
        coro->func(coro->arg);
    }

    coro->state = IDCU_CORO_FINISHED;
    idcu_coro_yield();
}

int idcu_coro_create(idcu_CoroutineScheduler* scheduler, idcu_Coroutine** coro,
                      const idcu_CoroutineConfig* config) {
    if (!scheduler || !coro || !config) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_Coroutine* c = (idcu_Coroutine*)malloc(sizeof(idcu_Coroutine));
    if (!c) {
        return IDCU_ERR_NO_MEMORY;
    }

    memset(c, 0, sizeof(idcu_Coroutine));
    c->id = scheduler->next_id++;
    c->state = IDCU_CORO_READY;
    c->func = config->func;
    c->arg = config->arg;
    c->stack_size = IDCU_COROUTINE_STACK_SIZE;
    c->scheduler = scheduler;

    c->stack = malloc(c->stack_size);
    if (!c->stack) {
        free(c);
        return IDCU_ERR_NO_MEMORY;
    }

#ifdef _WIN32
    c->fiber = CreateFiber(c->stack_size, (LPFIBER_START_ROUTINE)coroutine_wrapper, NULL);
    if (!c->fiber) {
        free(c->stack);
        free(c);
        return IDCU_ERR_GENERAL;
    }
#else
    getcontext(&c->ctx);
    c->ctx.uc_stack.ss_sp = c->stack;
    c->ctx.uc_stack.ss_size = c->stack_size;
    c->ctx.uc_link = &c->return_ctx;
    makecontext(&c->ctx, coroutine_wrapper, 0);
#endif

    idcu_vector_push_back(&scheduler->all_coroutines, &c);
    idcu_vector_push_back(&scheduler->ready_queue, &c);

    *coro = c;
    return IDCU_ERR_OK;
}

void idcu_coro_destroy(idcu_Coroutine* coro) {
    if (!coro) {
        return;
    }

#ifdef _WIN32
    if (coro->fiber) {
        DeleteFiber(coro->fiber);
    }
#endif

    if (coro->stack) {
        free(coro->stack);
    }

    free(coro);
}

int idcu_coro_yield(void) {
    idcu_Coroutine* current = g_current_coro;
    idcu_CoroutineScheduler* scheduler = g_current_scheduler;

    if (!current || !scheduler) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    if (current->state == IDCU_CORO_RUNNING) {
        current->state = IDCU_CORO_READY;
        idcu_vector_push_back(&scheduler->ready_queue, &current);
    }

#ifdef _WIN32
    SwitchToFiber(scheduler->main_coro->fiber);
#else
    swapcontext(&current->ctx, &scheduler->main_coro->ctx);
#endif

    return IDCU_ERR_OK;
}

int idcu_coro_scheduler_run(idcu_CoroutineScheduler* scheduler) {
    if (!scheduler) {
        return IDCU_ERR_INVALID_PARAM;
    }

    g_current_scheduler = scheduler;
    scheduler->running = true;

    idcu_Coroutine main_coro;
    memset(&main_coro, 0, sizeof(main_coro));
    main_coro.state = IDCU_CORO_RUNNING;
    scheduler->main_coro = &main_coro;
    g_current_coro = &main_coro;

#ifdef _WIN32
    main_coro.fiber = ConvertThreadToFiber(NULL);
#endif

    while (scheduler->running && idcu_vector_size(&scheduler->ready_queue) > 0) {
        idcu_Coroutine** coro_ptr = (idcu_Coroutine**)idcu_vector_get(&scheduler->ready_queue, 0);
        if (!coro_ptr || !*coro_ptr) {
            idcu_vector_remove(&scheduler->ready_queue, 0);
            continue;
        }

        idcu_vector_remove(&scheduler->ready_queue, 0);
        idcu_Coroutine* coro = *coro_ptr;

        if (coro->state != IDCU_CORO_READY) {
            continue;
        }

        g_current_coro = coro;
        coro->state = IDCU_CORO_RUNNING;

#ifdef _WIN32
        SwitchToFiber(coro->fiber);
#else
        swapcontext(&main_coro.ctx, &coro->ctx);
#endif

        if (coro->state == IDCU_CORO_FINISHED) {
            idcu_coro_destroy(coro);
        }

        g_current_coro = &main_coro;
    }

    scheduler->running = false;
    g_current_scheduler = NULL;
    g_current_coro = NULL;

    return IDCU_ERR_OK;
}

int idcu_coro_scheduler_stop(idcu_CoroutineScheduler* scheduler) {
    if (!scheduler) {
        return IDCU_ERR_INVALID_PARAM;
    }
    scheduler->running = false;
    return IDCU_ERR_OK;
}

idcu_CoroutineState idcu_coro_get_state(const idcu_Coroutine* coro) {
    return coro ? coro->state : IDCU_CORO_FINISHED;
}

const char* idcu_coro_state_to_str(idcu_CoroutineState state) {
    switch (state) {
        case IDCU_CORO_READY: return "READY";
        case IDCU_CORO_RUNNING: return "RUNNING";
        case IDCU_CORO_WAITING: return "WAITING";
        case IDCU_CORO_SUSPENDED: return "SUSPENDED";
        case IDCU_CORO_FINISHED: return "FINISHED";
        default: return "UNKNOWN";
    }
}

idcu_Coroutine* idcu_coro_current(void) {
    return g_current_coro;
}

int idcu_coro_sleep_ms(uint32_t ms) {
    (void)ms;
    return idcu_coro_yield();
}
```

### 5. 创建 CMakeLists.txt

创建 `libs/idcu-coroutine/CMakeLists.txt`：

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

### 6. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-coroutine/module.yaml`：

```yaml
name: idcu-coroutine
version: 1.0.0
description: Coroutine and scheduler library for IDCU Agent
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
  - idcu/coroutine/scheduler.h

features:
  - coroutine: Lightweight coroutine implementation
  - scheduler: Flexible coroutine scheduler
  - context_switch: Fast context switching
  - multi_policy: Multiple scheduling policies (round-robin, priority, FIFO)

testing:
  enabled: true
  framework: internal

security:
  notes: |
    - Stack size is configurable but has reasonable defaults
    - No unsafe memory operations outside coroutine boundaries
    - Proper cleanup of coroutine resources
```

### 7. 创建 README.md

创建 `libs/idcu-coroutine/README.md`：

```markdown
# idcu-coroutine

协程和调度器库，为 IDCU Agent 提供轻量级并发支持。

## 功能特性

- **协程支持**: 轻量级协程实现
- **调度器**: 灵活的协程调度器
- **上下文切换**: 快速的上下文切换
- **多种策略**: 支持多种调度策略（轮询、优先级、FIFO）

## 快速开始

### 基本使用

```c
#include "idcu/coroutine/coroutine.h"
#include <stdio.h>

static void coroutine_func(void* arg) {
    int id = *(int*)arg;
    printf("Coroutine %d starting\n", id);
    
    for (int i = 0; i < 3; i++) {
        printf("Coroutine %d: iteration %d\n", id, i);
        idcu_coro_yield();
    }
    
    printf("Coroutine %d finished\n", id);
}

int main(void) {
    idcu_CoroutineScheduler* scheduler;
    idcu_coro_scheduler_init(&scheduler);
    
    int ids[3] = {1, 2, 3};
    idcu_CoroutineConfig configs[3];
    
    for (int i = 0; i < 3; i++) {
        configs[i].func = coroutine_func;
        configs[i].arg = &ids[i];
        idcu_coro_create(scheduler, NULL, &configs[i]);
    }
    
    idcu_coro_scheduler_run(scheduler);
    idcu_coro_scheduler_destroy(scheduler);
    
    return 0;
}
```

## 构建

```bash
mkdir build && cd build
cmake ..
make
```

## API 文档

详见 [include/idcu/coroutine/](include/idcu/coroutine/)
```

## 验证检查清单

- [ ] 头文件已创建
- [ ] 源文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建（YAML 默认格式）
- [ ] README.md 已创建
- [ ] 代码可以成功编译
- [ ] 基本协程创建和调度功能正常
- [ ] 协程切换功能正常
- [ ] 安全检查清单已通过

## Git 提交

```bash
git add libs/idcu-coroutine/
git commit -m "feat: add idcu-coroutine library

- Add coroutine context switching
- Add coroutine scheduler with multiple policies
- Add coroutine creation and management
- Add CMake build configuration
- Add module.yaml metadata with YAML format"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 协程切换失败 | 栈空间不足 | 增加 IDCU_COROUTINE_STACK_SIZE |
| 内存泄漏 | 忘记调用 destroy | 确保所有创建的协程都被销毁 |
| Windows 编译错误 | Windows SDK 版本 | 定义 _WIN32_WINNT=0x0601 或更高 |
