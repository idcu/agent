# 任务 3.18: idcu-watchdog - 看门狗库

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的看门狗库，实现：
- 支持 100 个以上看门狗实例的并发监控
- 看门狗定时器精度 ≤ 10ms
- 心跳检测超时检测，支持多级状态（健康/警告/严重/死亡）
- 进程监控与自动重启功能，最大 5 次重启限制
- 告警通知集成

### 1.2 不做什么
- 不实现分布式看门狗
- 不实现内核级看门狗驱动
- 不实现跨机器的进程监控

### 1.3 输入
- 配置参数：看门狗超时时间、检查间隔、重启延迟
- 心跳信号：来自各模块的心跳调用
- 进程信息：进程名称、启动命令、重启策略

### 1.4 输出
- 状态码：成功返回 0，失败返回错误码
- 回调通知：看门狗状态变化回调
- 统计数据：心跳统计、重启统计
- 进程状态：进程运行状态信息

### 1.5 前置依赖
- idcu-common 库已实现
- idcu-log 库已实现
- idcu-alert 库已实现
- idcu-common 的线程、锁、条件变量已可用

---

## 2. 技术实现方案

### 2.1 核心选型
- 线程模型：使用独立监控线程 + 条件变量
- 数据结构：Vector 存储看门狗条目，HashMap 快速查找
- 锁机制：互斥锁保护共享数据
- 进程监控：Linux 使用 pid 文件 + kill 信号，Windows 使用 CreateProcess + TerminateProcess

### 2.2 核心逻辑
看门狗监控流程：
```
1. 初始化看门狗管理器
   └─&gt; 创建监控线程
   └─&gt; 初始化数据结构

2. 注册看门狗
   └─&gt; 分配唯一 ID
   └─&gt; 初始化条目
   └─&gt; 加入监控列表

3. 监控循环（独立线程）
   └─&gt; 等待条件变量或超时
   └─&gt; 遍历所有看门狗
   └─&gt; 检查心跳时间
   └─&gt; 更新状态（健康→警告→严重→死亡）
   └─&gt; 触发回调

4. 心跳更新
   └─&gt; 更新最后心跳时间
   └─&gt; 重置状态

5. 进程监控流程
   └─&gt; 定期检查进程状态
   └─&gt; 进程异常时自动重启
   └─&gt; 记录重启次数
```

### 2.3 数据结构/接口

核心数据结构：
```c
typedef enum
{
    IDCU_WATCHDOG_STATUS_HEALTHY = 0,
    IDCU_WATCHDOG_STATUS_WARNING,
    IDCU_WATCHDOG_STATUS_CRITICAL,
    IDCU_WATCHDOG_STATUS_DEAD
} idcu_WatchdogStatus;

typedef uint64_t idcu_WatchdogId;

typedef void (*idcu_WatchdogCallback)(idcu_WatchdogId id, 
                                      idcu_WatchdogStatus status, 
                                      void* user_data);

typedef struct
{
    idcu_WatchdogId id;
    char name[128];
    uint64_t timeout_ms;
    uint64_t last_heartbeat;
    idcu_WatchdogStatus status;
    idcu_WatchdogCallback callback;
    void* callback_user_data;
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
    idcu_WatchdogId next_id;
} idcu_Watchdog;
```

核心 API：
```c
int  idcu_watchdog_init(idcu_Watchdog* watchdog, const idcu_WatchdogConfig* config);
void idcu_watchdog_destroy(idcu_Watchdog* watchdog);
int  idcu_watchdog_start(idcu_Watchdog* watchdog);
void idcu_watchdog_stop(idcu_Watchdog* watchdog);

idcu_WatchdogId idcu_watchdog_register(idcu_Watchdog* watchdog, const char* name, uint64_t timeout_ms);
int  idcu_watchdog_unregister(idcu_Watchdog* watchdog, idcu_WatchdogId id);
int  idcu_watchdog_heartbeat(idcu_Watchdog* watchdog, idcu_WatchdogId id);

int  idcu_process_monitor_init(idcu_ProcessMonitor* monitor);
int  idcu_process_monitor_add(idcu_ProcessMonitor* monitor, const char* name, const char* command,
                              int auto_restart, int max_restarts, uint64_t restart_delay_ms);
int  idcu_process_monitor_start(idcu_ProcessMonitor* monitor);
```

### 2.4 跨平台适配

| 功能 | Linux 实现 | Windows 实现 |
|-----|----------|------------|
| 进程创建 | fork + execv | CreateProcess |
| 进程终止 | kill(pid, SIGTERM/SIGKILL) | TerminateProcess |
| 进程检查 | kill(pid, 0) 检查是否存在 | OpenProcess + GetExitCodeProcess |
| 等待进程 | waitpid | WaitForSingleObject |
| 线程创建 | pthread_create | CreateThread |
| 互斥锁 | pthread_mutex_t | CRITICAL_SECTION |
| 条件变量 | pthread_cond_t | CONDITION_VARIABLE |

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 看门狗可以正常初始化和销毁
- [ ] 支持注册 100 个以上看门狗实例无崩溃
- [ ] 心跳检测正常工作，超时后状态正确变化
- [ ] 警告、严重、死亡状态正确触发回调
- [ ] 进程监控可以正常启动和停止进程
- [ ] 进程异常退出后自动重启功能正常
- [ ] 重启次数达到限制后停止重启
- [ ] 可以获取看门狗统计信息（心跳数、错过次数）
- [ ] 全局回调和单个看门狗回调都正常工作

### 3.2 性能验收
- 看门狗检查间隔精度 ≤ 10ms
- 注册 100 个看门狗的总耗时 ≤ 50ms
- 单次心跳调用耗时 ≤ 1ms
- 监控线程 CPU 占用率 ≤ 1%（空闲时）
- 支持 1000 次/秒的心跳调用
- 进程重启延迟 ≤ 100ms

### 3.3 异常验收
- 注册重复名称的看门狗返回错误码
- 心跳未注册的看门狗 ID 返回错误
- 进程启动失败时返回错误码并记录日志
- 监控线程异常退出时不影响主程序
- 内存泄漏检测：1 小时运行无内存泄漏
- 多线程并发访问看门狗无数据竞争

---

## 4. 执行计划

### 4.1 工期
3 天/人

### 4.2 里程碑
- D1：完成接口定义、头文件、CMakeLists.txt、module.yaml、README.md
- D2：完成看门狗核心逻辑实现
- D3：完成进程监控实现、单元测试、集成测试

### 4.3 人力
1 人（技能要求：C 语言 + Linux/Windows 开发、多线程编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名：idcu_watchdog_* 小写加下划线
- 结构体前缀：idcu_
- 宏定义：IDCU_WATCHDOG_* 大写加下划线
- 错误码：使用 idcu-common 定义的错误码

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 单元测试覆盖所有公共 API
- 集成测试覆盖 5 种异常场景
- 性能测试验证性能指标
- 跨平台测试（Linux 和 Windows）

### 5.3 部署指引
- 编译命令：`cmake .. -DBUILD_TESTING=ON -DBUILD_EXAMPLES=ON && make
- Windows 使用：`cmake .. -G "Visual Studio 16 2019" && msbuild idcu-watchdog.sln
- 部署路径：libs/idcu-watchdog/
- 头文件路径：include/idcu/watchdog/

---

## 6. 风险与应对

### 6.1 风险 1
描述：多线程并发访问导致数据竞争  
应对：使用互斥锁保护所有共享数据结构，使用条件变量正确唤醒监控线程

### 6.2 风险 2
描述：跨平台进程监控实现差异大，兼容性问题  
应对：分别实现 Linux 和 Windows 版本，使用条件编译，充分测试

---

## 7. 详细实现步骤

### 7.1 创建目录结构

```bash
mkdir -p libs/idcu-watchdog/include/idcu/watchdog
mkdir -p libs/idcu-watchdog/src/idcu/watchdog
mkdir -p libs/idcu-watchdog/tests
mkdir -p libs/idcu-watchdog/examples
```

### 7.2 创建看门狗头文件 (watchdog.h)

创建 `libs/idcu-watchdog/include/idcu/watchdog/watchdog.h`：

```c
#ifndef IDCU_WATCHDOG_WATCHDOG_H
#define IDCU_WATCHDOG_WATCHDOG_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/lock.h"
#include "idcu/common/thread.h"
#include "idcu/common/condition.h"
#include &lt;stddef.h&gt;
#include &lt;stdint.h&gt;

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

### 7.3 创建 CMakeLists.txt

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
    $&lt;BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include&gt;
    $&lt;INSTALL_INTERFACE:include&gt;
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

### 7.4 创建模块配置文件 (module.yaml)

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

### 7.5 创建 README.md

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
idcu_watchdog_config_init(&amp;config);

config.check_interval_ms = 1000;

idcu_Watchdog watchdog;
idcu_watchdog_init(&amp;watchdog, &amp;config);
idcu_watchdog_start(&amp;watchdog);
```

### 注册看门狗

```c
void my_callback(idcu_WatchdogId id, idcu_WatchdogStatus status, void* user_data)
{
    printf("Watchdog %" PRIu64 " status: %s\n", 
           id, idcu_watchdog_status_to_string(status));
}

idcu_WatchdogId id = idcu_watchdog_register_with_callback(
    &amp;watchdog, "my_service", 5000, my_callback, NULL
);
```

### 设置阈值

```c
idcu_watchdog_set_thresholds(&amp;watchdog, id, 3000, 5000);
```

### 发送心跳

```c
idcu_watchdog_heartbeat(&amp;watchdog, id);
```

### 全局回调

```c
void global_callback(idcu_WatchdogId id, idcu_WatchdogStatus status, void* user_data)
{
    printf("Global callback: Watchdog %" PRIu64 " is %s\n", 
           id, idcu_watchdog_status_to_string(status));
}

idcu_watchdog_set_global_callback(&amp;watchdog, global_callback, NULL);
```

### 获取统计信息

```c
idcu_WatchdogStats stats;
idcu_watchdog_get_stats(&amp;watchdog, id, &amp;stats);

printf("Status: %s\n", idcu_watchdog_status_to_string(stats.status));
printf("Missed heartbeats: %" PRIu64 "\n", stats.missed_heartbeats);
printf("Total heartbeats: %" PRIu64 "\n", stats.total_heartbeats);
```

### 取消注册

```c
idcu_watchdog_unregister(&amp;watchdog, id);
```

### 停止看门狗

```c
idcu_watchdog_stop(&amp;watchdog);
idcu_watchdog_destroy(&amp;watchdog);
```

### 进程监控

```c
idcu_ProcessMonitor proc_monitor;
idcu_process_monitor_init(&amp;proc_monitor);

idcu_process_monitor_add(&amp;proc_monitor, "my_daemon", "/usr/bin/my_daemon",
                        1, 5, 5000);

idcu_process_monitor_start(&amp;proc_monitor);
idcu_process_monitor_start_process(&amp;proc_monitor, "my_daemon");

idcu_ProcessStatus status = idcu_process_monitor_get_status(&amp;proc_monitor, "my_daemon");
printf("Process status: %s\n", idcu_process_status_to_string(status));

idcu_process_monitor_stop(&amp;proc_monitor);
idcu_process_monitor_destroy(&amp;proc_monitor);
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

---

## 8. 验证检查清单

- [ ] 看门狗头文件已创建
- [ ] 看门狗实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 看门狗可以正常注册和心跳
- [ ] 超时检测正常工作
- [ ] 进程监控正常工作
- [ ] 可以正常编译
- [ ] 单元测试通过
- [ ] 集成测试通过
- [ ] 性能指标达标
- [ ] 已提交 Git

---

## 9. Git 提交

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

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 误报 | 超时时间太短 | 增加 timeout_ms |
| 心跳丢失 | 未定期发送心跳 | 确保定期调用 heartbeat |
| 进程频繁重启 | 进程本身有问题 | 检查进程日志和错误
