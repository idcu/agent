# 任务 5.5: task-queue - 任务队列模块

## 目标

创建任务队列模块，支持：
- 任务提交和执行
- 任务优先级
- 任务队列管理
- 任务调度
- 任务依赖
- 任务重试
- 任务超时
- 任务状态监控
- 任务历史记录
- 持久化队列

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/task-queue/include/idcu/task_queue
mkdir -p modules/task-queue/src/idcu/task_queue
mkdir -p modules/task-queue/tests
mkdir -p modules/task-queue/examples
```

### 2. 创建任务队列头文件 (task_queue.h)

创建 `modules/task-queue/include/idcu/task_queue/task_queue.h`：

```c
#ifndef IDCU_TASK_QUEUE_TASK_QUEUE_H
#define IDCU_TASK_QUEUE_TASK_QUEUE_H

#include "idcu/common/error_code.h"
#include "idcu/scheduler/scheduler.h"
#include "idcu/storage/storage.h"
#include "idcu/metrics/metrics.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_TaskId;

typedef enum
{
    IDCU_TASK_STATE_PENDING = 0,
    IDCU_TASK_STATE_READY,
    IDCU_TASK_STATE_RUNNING,
    IDCU_TASK_STATE_COMPLETED,
    IDCU_TASK_STATE_FAILED,
    IDCU_TASK_STATE_CANCELLED,
    IDCU_TASK_STATE_TIMEDOUT
} idcu_TaskState;

typedef enum
{
    IDCU_TASK_PRIORITY_LOW = 0,
    IDCU_TASK_PRIORITY_NORMAL,
    IDCU_TASK_PRIORITY_HIGH,
    IDCU_TASK_PRIORITY_CRITICAL
} idcu_TaskPriority;

typedef int (*idcu_TaskFunc)(void* task_data, void* user_data);
typedef void (*idcu_TaskCallback)(idcu_TaskId task_id, idcu_TaskState state, void* user_data);

typedef struct
{
    idcu_TaskId id;
    char name[256];
    char description[1024];
    
    idcu_TaskFunc func;
    void* task_data;
    size_t task_data_size;
    
    idcu_TaskPriority priority;
    idcu_TaskState state;
    
    uint64_t created_at;
    uint64_t scheduled_at;
    uint64_t started_at;
    uint64_t completed_at;
    
    int retry_count;
    int max_retries;
    uint64_t retry_delay_ms;
    
    uint64_t timeout_ms;
    
    idcu_Vector dependencies;
    
    char error_message[1024];
    int error_code;
    
    double progress;
    
    idcu_TaskCallback callback;
    void* callback_user_data;
} idcu_Task;

typedef struct
{
    idcu_Vector pending_tasks;
    idcu_Vector ready_tasks;
    idcu_Vector running_tasks;
    idcu_Vector completed_tasks;
    idcu_Vector failed_tasks;
    
    idcu_HashMap tasks_by_id;
    idcu_Mutex lock;
    
    idcu_Thread worker_threads[8];
    size_t worker_count;
    
    idcu_Condition condition;
    
    idcu_KVStore* storage;
    int persistence_enabled;
    char persistence_path[1024];
    
    idcu_Counter* tasks_submitted_counter;
    idcu_Counter* tasks_completed_counter;
    idcu_Counter* tasks_failed_counter;
    idcu_Gauge* tasks_pending_gauge;
    idcu_Gauge* tasks_running_gauge;
    
    uint64_t default_timeout_ms;
    int default_max_retries;
    uint64_t default_retry_delay_ms;
    
    size_t max_history_size;
    
    int running;
    int initialized;
} idcu_TaskQueue;

typedef struct
{
    size_t worker_count;
    uint64_t default_timeout_ms;
    int default_max_retries;
    uint64_t default_retry_delay_ms;
    size_t max_history_size;
    int enable_persistence;
    char persistence_path[1024];
    int enable_metrics;
} idcu_TaskQueueConfig;

int  idcu_task_queue_config_init(idcu_TaskQueueConfig* config);

int  idcu_task_queue_init(idcu_TaskQueue* tq, const idcu_TaskQueueConfig* config);
void idcu_task_queue_destroy(idcu_TaskQueue* tq);
int  idcu_task_queue_start(idcu_TaskQueue* tq);
void idcu_task_queue_stop(idcu_TaskQueue* tq);

idcu_TaskId idcu_task_queue_submit(idcu_TaskQueue* tq, const idcu_Task* task);
idcu_TaskId idcu_task_queue_submit_simple(idcu_TaskQueue* tq, const char* name, idcu_TaskFunc func, 
                                             void* task_data, size_t task_data_size);
idcu_TaskId idcu_task_queue_submit_with_priority(idcu_TaskQueue* tq, const char* name, idcu_TaskFunc func,
                                                    void* task_data, size_t task_data_size,
                                                    idcu_TaskPriority priority);
idcu_TaskId idcu_task_queue_submit_delayed(idcu_TaskQueue* tq, const char* name, idcu_TaskFunc func,
                                              void* task_data, size_t task_data_size,
                                              uint64_t delay_ms);

int  idcu_task_queue_cancel(idcu_TaskQueue* tq, idcu_TaskId task_id);
int  idcu_task_queue_retry(idcu_TaskQueue* tq, idcu_TaskId task_id);
int  idcu_task_queue_resubmit(idcu_TaskQueue* tq, idcu_TaskId task_id);

idcu_Task* idcu_task_queue_get(idcu_TaskQueue* tq, idcu_TaskId task_id);
idcu_TaskState idcu_task_queue_get_state(idcu_TaskQueue* tq, idcu_TaskId task_id);
int  idcu_task_queue_get_progress(idcu_TaskQueue* tq, idcu_TaskId task_id, double* progress);
int  idcu_task_queue_set_progress(idcu_TaskQueue* tq, idcu_TaskId task_id, double progress);

int  idcu_task_queue_add_dependency(idcu_TaskQueue* tq, idcu_TaskId task_id, idcu_TaskId dependency_id);
int  idcu_task_queue_remove_dependency(idcu_TaskQueue* tq, idcu_TaskId task_id, idcu_TaskId dependency_id);
int  idcu_task_queue_has_dependencies(idcu_TaskQueue* tq, idcu_TaskId task_id);
int  idcu_task_queue_dependencies_met(idcu_TaskQueue* tq, idcu_TaskId task_id);

int  idcu_task_queue_set_callback(idcu_TaskQueue* tq, idcu_TaskId task_id, 
                                    idcu_TaskCallback callback, void* user_data);

int  idcu_task_queue_get_pending(idcu_TaskQueue* tq, idcu_Vector* tasks);
int  idcu_task_queue_get_running(idcu_TaskQueue* tq, idcu_Vector* tasks);
int  idcu_task_queue_get_completed(idcu_TaskQueue* tq, idcu_Vector* tasks);
int  idcu_task_queue_get_failed(idcu_TaskQueue* tq, idcu_Vector* tasks);
int  idcu_task_queue_get_all(idcu_TaskQueue* tq, idcu_Vector* tasks);

size_t idcu_task_queue_pending_count(idcu_TaskQueue* tq);
size_t idcu_task_queue_running_count(idcu_TaskQueue* tq);
size_t idcu_task_queue_completed_count(idcu_TaskQueue* tq);
size_t idcu_task_queue_failed_count(idcu_TaskQueue* tq);
size_t idcu_task_queue_total_count(idcu_TaskQueue* tq);

int  idcu_task_queue_clear_completed(idcu_TaskQueue* tq);
int  idcu_task_queue_clear_failed(idcu_TaskQueue* tq);
int  idcu_task_queue_clear_all(idcu_TaskQueue* tq);

int  idcu_task_queue_set_storage(idcu_TaskQueue* tq, idcu_KVStore* storage, const char* path);
int  idcu_task_queue_enable_persistence(idcu_TaskQueue* tq);
int  idcu_task_queue_disable_persistence(idcu_TaskQueue* tq);
int  idcu_task_queue_save(idcu_TaskQueue* tq);
int  idcu_task_queue_load(idcu_TaskQueue* tq);

int  idcu_task_queue_set_metrics(idcu_TaskQueue* tq, idcu_Counter* submitted, idcu_Counter* completed,
                                   idcu_Counter* failed, idcu_Gauge* pending, idcu_Gauge* running);

int  idcu_task_init(idcu_Task* task);
void idcu_task_destroy(idcu_Task* task);
int  idcu_task_set_name(idcu_Task* task, const char* name);
int  idcu_task_set_description(idcu_Task* task, const char* description);
int  idcu_task_set_function(idcu_Task* task, idcu_TaskFunc func, void* task_data, size_t task_data_size);
int  idcu_task_set_priority(idcu_Task* task, idcu_TaskPriority priority);
int  idcu_task_set_timeout(idcu_Task* task, uint64_t timeout_ms);
int  idcu_task_set_retry(idcu_Task* task, int max_retries, uint64_t retry_delay_ms);
int  idcu_task_set_schedule(idcu_Task* task, uint64_t scheduled_at);

const char* idcu_task_state_to_string(idcu_TaskState state);
const char* idcu_task_priority_to_string(idcu_TaskPriority priority);

int  idcu_task_queue_get_stats(idcu_TaskQueue* tq, uint64_t* total_submitted, uint64_t* total_completed,
                                uint64_t* total_failed, size_t* pending, size_t* running);
int  idcu_task_queue_reset_stats(idcu_TaskQueue* tq);

int  idcu_task_queue_export_json(idcu_TaskQueue* tq, char* buffer, size_t buffer_size);
int  idcu_task_queue_get_info(idcu_TaskQueue* tq, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/task-queue/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(task-queue VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(task-queue STATIC
    src/idcu/task_queue/task_queue.c
    src/idcu/task_queue/task.c
    src/idcu/task_queue/task_persistence.c
)

target_include_directories(task-queue PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(task-queue PRIVATE
    idcu::common
    idcu::scheduler
    idcu::storage
    idcu::metrics
    idcu::utils
    idcu::log
)

add_library(idcu::task-queue ALIAS task-queue)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/task-queue/module.yaml`：

```yaml
name: task-queue
version: 1.0.0
description: Task queue module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-scheduler
  - idcu-storage
  - idcu-metrics
  - idcu-utils
  - idcu-log

build:
  type: cmake
  targets:
    - task-queue

headers:
  - idcu/task_queue/task_queue.h

features:
  - submit: Task submission and execution
  - priority: Task priority
  - queue: Task queue management
  - scheduling: Task scheduling
  - dependency: Task dependency
  - retry: Task retry
  - timeout: Task timeout
  - monitoring: Task state monitoring
  - history: Task history
  - persistence: Persistent queue

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `modules/task-queue/README.md`：

```markdown
# task-queue

IDCU Agent 的任务队列模块。

## 功能特性

- **提交执行**: 任务提交和执行
- **优先级**: 任务优先级
- **队列管理**: 任务队列管理
- **任务调度**: 任务调度
- **任务依赖**: 任务依赖
- **任务重试**: 任务重试
- **任务超时**: 任务超时
- **状态监控**: 任务状态监控
- **历史记录**: 任务历史记录
- **持久化**: 持久化队列

## 快速开始

### 初始化任务队列

```c
#include "idcu/task_queue/task_queue.h"

idcu_TaskQueueConfig config;
idcu_task_queue_config_init(&config);

config.worker_count = 4;
config.default_timeout_ms = 30000;
config.default_max_retries = 3;
config.default_retry_delay_ms = 1000;
config.max_history_size = 1000;
config.enable_persistence = 1;
strncpy(config.persistence_path, "./data/tasks.kv", sizeof(config.persistence_path));
config.enable_metrics = 1;

idcu_TaskQueue tq;
idcu_task_queue_init(&tq, &config);
```

### 启动任务队列

```c
idcu_task_queue_start(&tq);
```

### 提交简单任务

```c
int my_task(void* task_data, void* user_data)
{
    printf("Task executed\n");
    return 0;
}

idcu_TaskId task_id = idcu_task_queue_submit_simple(&tq, "My Task", my_task, NULL, 0);
```

### 提交带优先级的任务

```c
idcu_TaskId high_priority_id = idcu_task_queue_submit_with_priority(&tq, "High Priority", my_task,
                                                                         NULL, 0, IDCU_TASK_PRIORITY_HIGH);
```

### 提交延迟任务

```c
idcu_TaskId delayed_id = idcu_task_queue_submit_delayed(&tq, "Delayed", my_task,
                                                            NULL, 0, 5000);
```

### 创建完整任务

```c
idcu_Task task;
idcu_task_init(&task);

idcu_task_set_name(&task, "Complex Task");
idcu_task_set_description(&task, "This is a complex task with all features");
idcu_task_set_function(&task, my_task, NULL, 0);
idcu_task_set_priority(&task, IDCU_TASK_PRIORITY_HIGH);
idcu_task_set_timeout(&task, 60000);
idcu_task_set_retry(&task, 5, 2000);

idcu_TaskId full_task_id = idcu_task_queue_submit(&tq, &task);
idcu_task_destroy(&task);
```

### 添加任务回调

```c
void task_callback(idcu_TaskId task_id, idcu_TaskState state, void* user_data)
{
    printf("Task %" PRIu64 " state: %s\n", task_id, idcu_task_state_to_string(state));
}

idcu_task_queue_set_callback(&tq, task_id, task_callback, NULL);
```

### 添加任务依赖

```c
idcu_TaskId task1 = idcu_task_queue_submit_simple(&tq, "Task 1", task1_func, NULL, 0);
idcu_TaskId task2 = idcu_task_queue_submit_simple(&tq, "Task 2", task2_func, NULL, 0);

idcu_task_queue_add_dependency(&tq, task2, task1);
```

### 获取任务状态

```c
idcu_TaskState state = idcu_task_queue_get_state(&tq, task_id);
printf("Task state: %s\n", idcu_task_state_to_string(state));

double progress;
idcu_task_queue_get_progress(&tq, task_id, &progress);
printf("Progress: %.2f%%\n", progress * 100);
```

### 更新任务进度

```c
idcu_task_queue_set_progress(&tq, task_id, 0.5);
```

### 获取任务

```c
idcu_Task* task = idcu_task_queue_get(&tq, task_id);
if (task) {
    printf("Task name: %s\n", task->name);
    printf("Task priority: %s\n", idcu_task_priority_to_string(task->priority));
}
```

### 取消任务

```c
idcu_task_queue_cancel(&tq, task_id);
```

### 重试任务

```c
idcu_task_queue_retry(&tq, task_id);
```

### 重新提交任务

```c
idcu_task_queue_resubmit(&tq, task_id);
```

### 获取任务列表

```c
idcu_Vector pending_tasks;
idcu_vector_init(&pending_tasks, sizeof(idcu_TaskId));

idcu_task_queue_get_pending(&tq, &pending_tasks);

for (size_t i = 0; i < pending_tasks.count; i++) {
    idcu_TaskId* id_ptr = (idcu_TaskId*)idcu_vector_get(&pending_tasks, i);
    printf("Pending task ID: %" PRIu64 "\n", *id_ptr);
}

idcu_vector_destroy(&pending_tasks);
```

### 获取队列统计

```c
printf("Pending count: %zu\n", idcu_task_queue_pending_count(&tq));
printf("Running count: %zu\n", idcu_task_queue_running_count(&tq));
printf("Completed count: %zu\n", idcu_task_queue_completed_count(&tq));
printf("Failed count: %zu\n", idcu_task_queue_failed_count(&tq));
printf("Total count: %zu\n", idcu_task_queue_total_count(&tq));
```

### 获取详细统计

```c
uint64_t total_submitted, total_completed, total_failed;
size_t pending, running;

idcu_task_queue_get_stats(&tq, &total_submitted, &total_completed, &total_failed, &pending, &running);

printf("Total submitted: %" PRIu64 "\n", total_submitted);
printf("Total completed: %" PRIu64 "\n", total_completed);
printf("Total failed: %" PRIu64 "\n", total_failed);
```

### 清空任务

```c
idcu_task_queue_clear_completed(&tq);
idcu_task_queue_clear_failed(&tq);
idcu_task_queue_clear_all(&tq);
```

### 持久化

```c
idcu_task_queue_enable_persistence(&tq);
idcu_task_queue_save(&tq);
idcu_task_queue_load(&tq);
idcu_task_queue_disable_persistence(&tq);
```

### 设置指标

```c
idcu_task_queue_set_metrics(&tq, submitted_counter, completed_counter,
                             failed_counter, pending_gauge, running_gauge);
```

### 导出信息

```c
char info_buffer[4096];
idcu_task_queue_get_info(&tq, info_buffer, sizeof(info_buffer));
printf("%s\n", info_buffer);

char json_buffer[8192];
idcu_task_queue_export_json(&tq, json_buffer, sizeof(json_buffer));
printf("%s\n", json_buffer);
```

### 停止任务队列

```c
idcu_task_queue_stop(&tq);
idcu_task_queue_destroy(&tq);
```

## 任务状态

| 状态 | 说明 |
|-----|------|
| PENDING | 待处理 |
| READY | 就绪 |
| RUNNING | 运行中 |
| COMPLETED | 已完成 |
| FAILED | 失败 |
| CANCELLED | 已取消 |
| TIMEDOUT | 超时 |

## 任务优先级

| 优先级 | 说明 |
|--------|------|
| LOW | 低 |
| NORMAL | 正常 |
| HIGH | 高 |
| CRITICAL | 紧急 |

## API 文档

详见 [include/idcu/task_queue/task_queue.h](include/idcu/task_queue/task_queue.h)
```

## 验证检查清单

- [ ] 任务队列头文件已创建
- [ ] 任务队列实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以提交和执行任务
- [ ] 任务优先级正常工作
- [ ] 任务依赖正常工作

## Git 提交

```bash
git add modules/task-queue/
git commit -m "feat: add task-queue module

- Add task submission and execution
- Add task priority
- Add task queue management
- Add task scheduling
- Add task dependency
- Add task retry
- Add task timeout
- Add task state monitoring
- Add task history
- Add persistent queue
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 任务未执行 | 队列未启动 | 确保任务队列已启动 |
| 任务超时 | 超时时间太短 | 增加超时时间 |
| 任务失败 | 任务函数错误 | 检查任务函数实现 |
