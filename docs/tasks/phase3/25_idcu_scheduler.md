# 任务 3.25: idcu-scheduler - 任务调度库

## 目标

创建任务调度库，支持：
- 定时任务
- 周期任务
- 一次性任务
- Cron 表达式
- 任务优先级
- 任务依赖
- 任务状态监控
- 任务取消

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-scheduler/include/idcu/scheduler
mkdir -p libs/idcu-scheduler/src/idcu/scheduler
mkdir -p libs/idcu-scheduler/tests
mkdir -p libs/idcu-scheduler/examples
```

### 2. 创建任务调度头文件 (scheduler.h)

创建 `libs/idcu-scheduler/include/idcu/scheduler/scheduler.h`：

```c
#ifndef IDCU_SCHEDULER_SCHEDULER_H
#define IDCU_SCHEDULER_SCHEDULER_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/common/thread.h"
#include "idcu/common/condition.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_TaskId;

typedef enum
{
    IDCU_TASK_TYPE_ONCE = 0,
    IDCU_TASK_TYPE_PERIODIC,
    IDCU_TASK_TYPE_CRON
} idcu_TaskType;

typedef enum
{
    IDCU_TASK_STATUS_PENDING = 0,
    IDCU_TASK_STATUS_SCHEDULED,
    IDCU_TASK_STATUS_RUNNING,
    IDCU_TASK_STATUS_COMPLETED,
    IDCU_TASK_STATUS_FAILED,
    IDCU_TASK_STATUS_CANCELLED,
    IDCU_TASK_STATUS_PAUSED
} idcu_TaskStatus;

typedef enum
{
    IDCU_TASK_PRIORITY_LOW = 0,
    IDCU_TASK_PRIORITY_NORMAL,
    IDCU_TASK_PRIORITY_HIGH,
    IDCU_TASK_PRIORITY_CRITICAL
} idcu_TaskPriority;

typedef int (*idcu_TaskFunc)(void* user_data);
typedef void (*idcu_TaskCompleteCallback)(idcu_TaskId id, int result, void* user_data);

typedef struct
{
    uint64_t second;
    uint64_t minute;
    uint64_t hour;
    uint64_t day_of_month;
    uint64_t month;
    uint64_t day_of_week;
} idcu_CronExpr;

typedef struct
{
    idcu_TaskId id;
    char name[128];
    idcu_TaskType type;
    idcu_TaskStatus status;
    idcu_TaskPriority priority;
    
    idcu_TaskFunc task_func;
    void* task_user_data;
    
    idcu_TaskCompleteCallback complete_callback;
    void* callback_user_data;
    
    uint64_t scheduled_at;
    uint64_t interval_ms;
    idcu_CronExpr cron_expr;
    
    uint64_t started_at;
    uint64_t completed_at;
    uint64_t last_run_at;
    uint64_t next_run_at;
    
    uint64_t run_count;
    uint64_t success_count;
    uint64_t failure_count;
    
    int last_result;
    char last_error[512];
    
    int enabled;
    int max_retry_count;
    int retry_count;
    uint64_t retry_delay_ms;
    
    idcu_Vector dependencies;
} idcu_Task;

typedef struct
{
    idcu_Vector tasks;
    idcu_HashMap tasks_by_id;
    idcu_Mutex lock;
    
    idcu_Thread scheduler_thread;
    idcu_Thread worker_thread;
    idcu_Condition wakeup_cond;
    
    idcu_Vector ready_queue;
    idcu_Vector running_tasks;
    
    int running;
    int initialized;
    
    uint64_t tick_interval_ms;
} idcu_Scheduler;

typedef struct
{
    uint64_t tick_interval_ms;
} idcu_SchedulerConfig;

int  idcu_scheduler_config_init(idcu_SchedulerConfig* config);

int  idcu_scheduler_init(idcu_Scheduler* scheduler, const idcu_SchedulerConfig* config);
void idcu_scheduler_destroy(idcu_Scheduler* scheduler);
int  idcu_scheduler_start(idcu_Scheduler* scheduler);
void idcu_scheduler_stop(idcu_Scheduler* scheduler);
void idcu_scheduler_pause(idcu_Scheduler* scheduler);
void idcu_scheduler_resume(idcu_Scheduler* scheduler);

idcu_TaskId idcu_scheduler_add_once(idcu_Scheduler* scheduler, const char* name,
                                      uint64_t delay_ms, idcu_TaskFunc func, void* user_data);
idcu_TaskId idcu_scheduler_add_periodic(idcu_Scheduler* scheduler, const char* name,
                                          uint64_t interval_ms, idcu_TaskFunc func, void* user_data);
idcu_TaskId idcu_scheduler_add_cron(idcu_Scheduler* scheduler, const char* name,
                                      const char* cron_expr, idcu_TaskFunc func, void* user_data);

int  idcu_scheduler_remove_task(idcu_Scheduler* scheduler, idcu_TaskId id);
int  idcu_scheduler_enable_task(idcu_Scheduler* scheduler, idcu_TaskId id);
int  idcu_scheduler_disable_task(idcu_Scheduler* scheduler, idcu_TaskId id);
int  idcu_scheduler_pause_task(idcu_Scheduler* scheduler, idcu_TaskId id);
int  idcu_scheduler_resume_task(idcu_Scheduler* scheduler, idcu_TaskId id);
int  idcu_scheduler_cancel_task(idcu_Scheduler* scheduler, idcu_TaskId id);
int  idcu_scheduler_run_now(idcu_Scheduler* scheduler, idcu_TaskId id);

idcu_Task* idcu_scheduler_get_task(idcu_Scheduler* scheduler, idcu_TaskId id);
idcu_Task* idcu_scheduler_get_task_by_name(idcu_Scheduler* scheduler, const char* name);
int  idcu_scheduler_get_task_status(idcu_Scheduler* scheduler, idcu_TaskId id, idcu_TaskStatus* status);
size_t idcu_scheduler_get_task_count(idcu_Scheduler* scheduler);
int  idcu_scheduler_get_all_tasks(idcu_Scheduler* scheduler, idcu_Vector* tasks);
int  idcu_scheduler_get_pending_tasks(idcu_Scheduler* scheduler, idcu_Vector* tasks);
int  idcu_scheduler_get_running_tasks(idcu_Scheduler* scheduler, idcu_Vector* tasks);

int  idcu_task_init(idcu_Task* task, const char* name, idcu_TaskType type, idcu_TaskFunc func, void* user_data);
void idcu_task_destroy(idcu_Task* task);
int  idcu_task_set_priority(idcu_Task* task, idcu_TaskPriority priority);
int  idcu_task_set_interval(idcu_Task* task, uint64_t interval_ms);
int  idcu_task_set_cron(idcu_Task* task, const char* cron_expr);
int  idcu_task_set_callback(idcu_Task* task, idcu_TaskCompleteCallback callback, void* user_data);
int  idcu_task_set_retry(idcu_Task* task, int max_retry, uint64_t retry_delay_ms);
int  idcu_task_add_dependency(idcu_Task* task, idcu_TaskId dependency_id);

int  idcu_cron_parse(const char* expr, idcu_CronExpr* cron);
int  idcu_cron_format(const idcu_CronExpr* cron, char* buffer, size_t buffer_size);
uint64_t idcu_cron_next_run(const idcu_CronExpr* cron, uint64_t from);
int  idcu_cron_is_valid(const char* expr);

uint64_t idcu_scheduler_now(void);
int  idcu_scheduler_sleep_until(uint64_t timestamp_ms);

typedef struct
{
    idcu_TaskId id;
    char name[128];
    idcu_TaskStatus status;
    idcu_TaskPriority priority;
    uint64_t next_run_at;
    uint64_t last_run_at;
    uint64_t run_count;
    uint64_t success_count;
    uint64_t failure_count;
} idcu_TaskStats;

int  idcu_scheduler_get_task_stats(idcu_Scheduler* scheduler, idcu_TaskId id, idcu_TaskStats* stats);
int  idcu_scheduler_get_all_stats(idcu_Scheduler* scheduler, idcu_Vector* stats);

const char* idcu_task_type_to_string(idcu_TaskType type);
const char* idcu_task_status_to_string(idcu_TaskStatus status);
const char* idcu_task_priority_to_string(idcu_TaskPriority priority);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-scheduler/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-scheduler VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-scheduler STATIC
    src/idcu/scheduler/scheduler.c
    src/idcu/scheduler/cron.c
)

target_include_directories(idcu-scheduler PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-scheduler PRIVATE
    idcu::common
    idcu::log
    idcu::utils
)

add_library(idcu::scheduler ALIAS idcu-scheduler)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-scheduler/module.yaml`：

```yaml
name: idcu-scheduler
version: 1.0.0
description: Task scheduler library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-log
  - idcu-utils

build:
  type: cmake
  targets:
    - idcu-scheduler

headers:
  - idcu/scheduler/scheduler.h

features:
  - once: One-time tasks
  - periodic: Periodic tasks
  - cron: Cron expressions
  - priority: Task priorities
  - dependencies: Task dependencies
  - status: Task status monitoring
  - cancel: Task cancellation
  - retry: Task retry
  - stats: Task statistics

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-scheduler/README.md`：

```markdown
# idcu-scheduler

IDCU Agent 的任务调度库。

## 功能特性

- **一次性任务**: 一次性任务
- **周期任务**: 周期任务
- **Cron 表达式**: Cron 表达式支持
- **任务优先级**: 任务优先级
- **任务依赖**: 任务依赖
- **状态监控**: 任务状态监控
- **任务取消**: 任务取消
- **任务重试**: 任务重试
- **统计信息**: 任务统计

## 快速开始

### 初始化调度器

```c
#include "idcu/scheduler/scheduler.h"

idcu_SchedulerConfig config;
idcu_scheduler_config_init(&config);

config.tick_interval_ms = 100;

idcu_Scheduler scheduler;
idcu_scheduler_init(&scheduler, &config);
idcu_scheduler_start(&scheduler);
```

### 添加一次性任务

```c
int my_task(void* user_data)
{
    printf("Task executed!\n");
    return 0;
}

idcu_TaskId id = idcu_scheduler_add_once(&scheduler, "my-once-task", 5000, my_task, NULL);
```

### 添加周期任务

```c
int periodic_task(void* user_data)
{
    printf("Periodic task executed!\n");
    return 0;
}

idcu_TaskId id = idcu_scheduler_add_periodic(&scheduler, "my-periodic-task", 60000, periodic_task, NULL);
```

### 添加 Cron 任务

```c
int cron_task(void* user_data)
{
    printf("Cron task executed!\n");
    return 0;
}

idcu_TaskId id = idcu_scheduler_add_cron(&scheduler, "my-cron-task", "0 * * * *", cron_task, NULL);
```

### Cron 表达式格式

```
* * * * * *
| | | | | |
| | | | | +-- 星期几 (0-6, 0=周日)
| | | | +---- 月份 (1-12)
| | | +------ 日期 (1-31)
| | +-------- 小时 (0-23)
| +---------- 分钟 (0-59)
+------------ 秒 (0-59)
```

### 设置任务优先级

```c
idcu_Task* task = idcu_scheduler_get_task(&scheduler, id);
idcu_task_set_priority(task, IDCU_TASK_PRIORITY_HIGH);
```

### 设置回调

```c
void task_complete(idcu_TaskId id, int result, void* user_data)
{
    printf("Task %" PRIu64 " completed with result: %d\n", id, result);
}

idcu_Task* task = idcu_scheduler_get_task(&scheduler, id);
idcu_task_set_callback(task, task_complete, NULL);
```

### 设置重试

```c
idcu_Task* task = idcu_scheduler_get_task(&scheduler, id);
idcu_task_set_retry(task, 3, 1000);
```

### 任务依赖

```c
idcu_Task* task = idcu_scheduler_get_task(&scheduler, task_id);
idcu_task_add_dependency(task, dependency_id);
```

### 控制任务

```c
idcu_scheduler_pause_task(&scheduler, id);
idcu_scheduler_resume_task(&scheduler, id);
idcu_scheduler_disable_task(&scheduler, id);
idcu_scheduler_enable_task(&scheduler, id);
idcu_scheduler_cancel_task(&scheduler, id);
idcu_scheduler_run_now(&scheduler, id);
```

### 获取任务状态

```c
idcu_TaskStatus status;
idcu_scheduler_get_task_status(&scheduler, id, &status);
printf("Task status: %s\n", idcu_task_status_to_string(status));
```

### 获取任务统计

```c
idcu_TaskStats stats;
idcu_scheduler_get_task_stats(&scheduler, id, &stats);

printf("Task: %s\n", stats.name);
printf("Status: %s\n", idcu_task_status_to_string(stats.status));
printf("Run count: %" PRIu64 "\n", stats.run_count);
printf("Success count: %" PRIu64 "\n", stats.success_count);
printf("Failure count: %" PRIu64 "\n", stats.failure_count);
```

### 暂停/恢复调度器

```c
idcu_scheduler_pause(&scheduler);
idcu_scheduler_resume(&scheduler);
```

### 停止调度器

```c
idcu_scheduler_stop(&scheduler);
idcu_scheduler_destroy(&scheduler);
```

## 任务类型

| 类型 | 说明 |
|-----|------|
| ONCE | 一次性任务 |
| PERIODIC | 周期任务 |
| CRON | Cron 表达式任务 |

## 任务状态

| 状态 | 说明 |
|-----|------|
| PENDING | 待处理 |
| SCHEDULED | 已调度 |
| RUNNING | 运行中 |
| COMPLETED | 已完成 |
| FAILED | 失败 |
| CANCELLED | 已取消 |
| PAUSED | 已暂停 |

## 任务优先级

| 优先级 | 说明 |
|-------|------|
| LOW | 低 |
| NORMAL | 正常 |
| HIGH | 高 |
| CRITICAL | 严重 |

## API 文档

详见 [include/idcu/scheduler/scheduler.h](include/idcu/scheduler/scheduler.h)
```

## 验证检查清单

- [ ] 任务调度头文件已创建
- [ ] 任务调度实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以添加和执行任务
- [ ] Cron 表达式正常工作
- [ ] 任务状态监控正常工作

## Git 提交

```bash
git add libs/idcu-scheduler/
git commit -m "feat: add idcu-scheduler library

- Add one-time tasks
- Add periodic tasks
- Add cron expressions
- Add task priorities
- Add task dependencies
- Add task status monitoring
- Add task cancellation
- Add task retry
- Add task statistics
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 任务不执行 | 任务被禁用 | 确保任务已启用 |
| Cron 表达式错误 | 格式不正确 | 检查 Cron 表达式格式 |
| 任务失败 | 函数返回错误 | 检查任务函数实现 |
