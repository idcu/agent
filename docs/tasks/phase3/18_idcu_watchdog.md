# 任务 3.18: idcu-watchdog - 看门狗库

## 目标

创建看门狗库，支持：
- 看门狗定时器
- 心跳检测
- 超时检测
- 自动恢复
- 健康监控
- 告警通知
- 进程监控

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-watchdog/include/idcu/watchdog
mkdir -p libs/idcu-watchdog/src/idcu/watchdog
mkdir -p libs/idcu-watchdog/tests
mkdir -p libs/idcu-watchdog/examples
```

### 2. 创建看门狗头文件 (watchdog.h)

创建 `libs/idcu-watchdog/include/idcu/watchdog/watchdog.h`：

```c
#ifndef IDCU_WATCHDOG_WATCHDOG_H
#define IDCU_WATCHDOG_WATCHDOG_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/lock.h"
#include "idcu/common/thread.h"
#include "idcu/common/condition.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_WATCHDOG_STATUS_HEALTHY = 0,
    IDCU_WATCHDOG_STATUS_WARNING,
    IDCU_WATCHDOG_STATUS_CRITICAL,
    IDCU_WATCHDOG_STATUS_DEAD
} idcu_WatchdogStatus;

typedef uint64_t idcu_WatchdogId;

typedef void (*idcu_WatchdogCallback)(idcu_WatchdogId id, idcu_WatchdogStatus status, void* user_data);

typedef struct
{
    idcu_WatchdogId id;
    char name[128];
    uint64_t timeout_ms;
    uint64_t last_heartbeat;
    uint64_t created_at;
    idcu_WatchdogStatus status;
    idcu_WatchdogCallback callback;
    void* callback_user_data;
    int enabled;
    uint64_t warning_threshold_ms;
    uint64_t critical_threshold_ms;
    uint64_t missed_heartbeats;
    uint64_t total_heartbeats;
} idcu_WatchdogEntry;

typedef struct
{
    idcu_Vector entries;
    idcu_HashMap entries_by_id;
    idcu_Mutex lock;
    idcu_Thread monitor_thread;
    idcu_Condition wakeup_cond;
    idcu_WatchdogCallback global_callback;
    void* global_callback_user_data;
    uint64_t check_interval_ms;
    int running;
    int initialized;
    idcu_WatchdogId next_id;
} idcu_Watchdog;

typedef struct
{
    uint64_t check_interval_ms;
} idcu_WatchdogConfig;

int  idcu_watchdog_config_init(idcu_WatchdogConfig* config);

int  idcu_watchdog_init(idcu_Watchdog* watchdog, const idcu_WatchdogConfig* config);
void idcu_watchdog_destroy(idcu_Watchdog* watchdog);
int  idcu_watchdog_start(idcu_Watchdog* watchdog);
void idcu_watchdog_stop(idcu_Watchdog* watchdog);

idcu_WatchdogId idcu_watchdog_register(idcu_Watchdog* watchdog, const char* name, uint64_t timeout_ms);
idcu_WatchdogId idcu_watchdog_register_with_callback(idcu_Watchdog* watchdog, const char* name, 
                                                      uint64_t timeout_ms, idcu_WatchdogCallback callback, void* user_data);
int  idcu_watchdog_unregister(idcu_Watchdog* watchdog, idcu_WatchdogId id);
int  idcu_watchdog_enable(idcu_Watchdog* watchdog, idcu_WatchdogId id);
int  idcu_watchdog_disable(idcu_Watchdog* watchdog, idcu_WatchdogId id);

int  idcu_watchdog_heartbeat(idcu_Watchdog* watchdog, idcu_WatchdogId id);
int  idcu_watchdog_heartbeat_all(idcu_Watchdog* watchdog);

int  idcu_watchdog_set_timeout(idcu_Watchdog* watchdog, idcu_WatchdogId id, uint64_t timeout_ms);
int  idcu_watchdog_set_thresholds(idcu_Watchdog* watchdog, idcu_WatchdogId id, 
                                   uint64_t warning_threshold_ms, uint64_t critical_threshold_ms);
int  idcu_watchdog_set_callback(idcu_Watchdog* watchdog, idcu_WatchdogId id, 
                                 idcu_WatchdogCallback callback, void* user_data);
int  idcu_watchdog_set_global_callback(idcu_Watchdog* watchdog, idcu_WatchdogCallback callback, void* user_data);

idcu_WatchdogEntry* idcu_watchdog_get_entry(idcu_Watchdog* watchdog, idcu_WatchdogId id);
idcu_WatchdogStatus idcu_watchdog_get_status(idcu_Watchdog* watchdog, idcu_WatchdogId id);
size_t idcu_watchdog_get_entry_count(idcu_Watchdog* watchdog);
int  idcu_watchdog_get_all_entries(idcu_Watchdog* watchdog, idcu_Vector* entries);

int  idcu_watchdog_reset(idcu_Watchdog* watchdog, idcu_WatchdogId id);
int  idcu_watchdog_reset_all(idcu_Watchdog* watchdog);

typedef struct
{
    idcu_WatchdogId id;
    char name[128];
    idcu_WatchdogStatus status;
    uint64_t timeout_ms;
    uint64_t time_since_last_heartbeat_ms;
    uint64_t missed_heartbeats;
    uint64_t total_heartbeats;
} idcu_WatchdogStats;

int  idcu_watchdog_get_stats(idcu_Watchdog* watchdog, idcu_WatchdogId id, idcu_WatchdogStats* stats);
int  idcu_watchdog_get_all_stats(idcu_Watchdog* watchdog, idcu_Vector* stats);

typedef enum
{
    IDCU_PROCESS_STATUS_RUNNING = 0,
    IDCU_PROCESS_STATUS_STOPPED,
    IDCU_PROCESS_STATUS_UNKNOWN
} idcu_ProcessStatus;

typedef struct
{
    char name[128];
    char command[1024];
    int pid;
    idcu_ProcessStatus status;
    uint64_t started_at;
    uint64_t last_checked_at;
    int restart_count;
    int max_restarts;
    uint64_t restart_delay_ms;
    int auto_restart;
} idcu_MonitoredProcess;

typedef struct
{
    idcu_Vector processes;
    idcu_Mutex lock;
    idcu_Thread monitor_thread;
    int running;
    int initialized;
} idcu_ProcessMonitor;

int  idcu_process_monitor_init(idcu_ProcessMonitor* monitor);
void idcu_process_monitor_destroy(idcu_ProcessMonitor* monitor);
int  idcu_process_monitor_start(idcu_ProcessMonitor* monitor);
void idcu_process_monitor_stop(idcu_ProcessMonitor* monitor);

int  idcu_process_monitor_add(idcu_ProcessMonitor* monitor, const char* name, const char* command,
                              int auto_restart, int max_restarts, uint64_t restart_delay_ms);
int  idcu_process_monitor_remove(idcu_ProcessMonitor* monitor, const char* name);
int  idcu_process_monitor_start_process(idcu_ProcessMonitor* monitor, const char* name);
int  idcu_process_monitor_stop_process(idcu_ProcessMonitor* monitor, const char* name);
int  idcu_process_monitor_restart_process(idcu_ProcessMonitor* monitor, const char* name);

idcu_MonitoredProcess* idcu_process_monitor_get(idcu_ProcessMonitor* monitor, const char* name);
idcu_ProcessStatus idcu_process_monitor_get_status(idcu_ProcessMonitor* monitor, const char* name);
size_t idcu_process_monitor_get_count(idcu_ProcessMonitor* monitor);

const char* idcu_watchdog_status_to_string(idcu_WatchdogStatus status);
const char* idcu_process_status_to_string(idcu_ProcessStatus status);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-watchdog/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-watchdog VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-watchdog STATIC
    src/idcu/watchdog/watchdog.c
)

target_include_directories(idcu-watchdog PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-watchdog PRIVATE
    idcu::common
    idcu::log
    idcu::alert
)

add_library(idcu::watchdog ALIAS idcu-watchdog)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-watchdog/module.yaml`：

```yaml
name: idcu-watchdog
version: 1.0.0
description: Watchdog library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-log
  - idcu-alert

build:
  type: cmake
  targets:
    - idcu-watchdog

headers:
  - idcu/watchdog/watchdog.h

features:
  - watchdog: Watchdog timer
  - heartbeat: Heartbeat detection
  - timeout: Timeout detection
  - auto_recover: Auto recovery
  - health: Health monitoring
  - alert: Alert notification
  - process: Process monitoring
  - auto_restart: Auto process restart

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-watchdog/README.md`：

```markdown
# idcu-watchdog

IDCU Agent 的看门狗库。

## 功能特性

- **看门狗定时器**: 看门狗定时器
- **心跳检测**: 心跳检测
- **超时检测**: 超时检测
- **自动恢复**: 自动恢复
- **健康监控**: 健康监控
- **告警通知**: 告警通知
- **进程监控**: 进程监控
- **自动重启**: 自动进程重启

## 快速开始

### 初始化看门狗

```c
#include "idcu/watchdog/watchdog.h"

idcu_WatchdogConfig config;
idcu_watchdog_config_init(&config);

config.check_interval_ms = 1000;

idcu_Watchdog watchdog;
idcu_watchdog_init(&watchdog, &config);
idcu_watchdog_start(&watchdog);
```

### 注册看门狗

```c
void my_callback(idcu_WatchdogId id, idcu_WatchdogStatus status, void* user_data)
{
    printf("Watchdog %" PRIu64 " status: %s\n", 
           id, idcu_watchdog_status_to_string(status));
}

idcu_WatchdogId id = idcu_watchdog_register_with_callback(
    &watchdog, "my_service", 5000, my_callback, NULL
);
```

### 设置阈值

```c
idcu_watchdog_set_thresholds(&watchdog, id, 3000, 5000);
```

### 发送心跳

```c
idcu_watchdog_heartbeat(&watchdog, id);
```

### 全局回调

```c
void global_callback(idcu_WatchdogId id, idcu_WatchdogStatus status, void* user_data)
{
    printf("Global callback: Watchdog %" PRIu64 " is %s\n", 
           id, idcu_watchdog_status_to_string(status));
}

idcu_watchdog_set_global_callback(&watchdog, global_callback, NULL);
```

### 获取统计信息

```c
idcu_WatchdogStats stats;
idcu_watchdog_get_stats(&watchdog, id, &stats);

printf("Status: %s\n", idcu_watchdog_status_to_string(stats.status));
printf("Missed heartbeats: %" PRIu64 "\n", stats.missed_heartbeats);
printf("Total heartbeats: %" PRIu64 "\n", stats.total_heartbeats);
```

### 取消注册

```c
idcu_watchdog_unregister(&watchdog, id);
```

### 停止看门狗

```c
idcu_watchdog_stop(&watchdog);
idcu_watchdog_destroy(&watchdog);
```

### 进程监控

```c
idcu_ProcessMonitor proc_monitor;
idcu_process_monitor_init(&proc_monitor);

idcu_process_monitor_add(&proc_monitor, "my_daemon", "/usr/bin/my_daemon",
                        1, 5, 5000);

idcu_process_monitor_start(&proc_monitor);
idcu_process_monitor_start_process(&proc_monitor, "my_daemon");

idcu_ProcessStatus status = idcu_process_monitor_get_status(&proc_monitor, "my_daemon");
printf("Process status: %s\n", idcu_process_status_to_string(status));

idcu_process_monitor_stop(&proc_monitor);
idcu_process_monitor_destroy(&proc_monitor);
```

## 看门狗状态

| 状态 | 说明 |
|-----|------|
| HEALTHY | 健康 |
| WARNING | 警告 |
| CRITICAL | 严重 |
| DEAD | 死亡 |

## 进程状态

| 状态 | 说明 |
|-----|------|
| RUNNING | 运行中 |
| STOPPED | 已停止 |
| UNKNOWN | 未知 |

## API 文档

详见 [include/idcu/watchdog/watchdog.h](include/idcu/watchdog/watchdog.h)
```

## 验证检查清单

- [ ] 看门狗头文件已创建
- [ ] 看门狗实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 看门狗可以正常注册和心跳
- [ ] 超时检测正常工作
- [ ] 进程监控正常工作

## Git 提交

```bash
git add libs/idcu-watchdog/
git commit -m "feat: add idcu-watchdog library

- Add watchdog timer
- Add heartbeat detection
- Add timeout detection
- Add auto recovery
- Add health monitoring
- Add alert notification
- Add process monitoring
- Add auto process restart
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 误报 | 超时时间太短 | 增加 timeout_ms |
| 心跳丢失 | 未定期发送心跳 | 确保定期调用 heartbeat |
| 进程频繁重启 | 进程本身有问题 | 检查进程日志和错误 |
