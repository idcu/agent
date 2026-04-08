# 任务 3.17: idcu-alert - 告警管理库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建告警管理库，支持告警定义和管理、告警触发和恢复、告警级别（INFO/WARN/ERROR/CRITICAL）、告警通知（HTTP/邮件/Webhook/回调）、告警去重、告警历史记录和告警聚合。

### 1.2 不做什么
- 不实现告警规则的自动评估逻辑（仅提供规则管理和手动触发）
- 不实现短信通知的底层发送功能（仅提供接口）
- 不实现邮件发送的 SMTP 服务器连接（仅提供配置接口）

### 1.3 输入
- 告警规则配置（名称、级别、描述、标签、注解、持续时间）
- 告警触发/恢复请求（规则名称、描述、标签）
- 通知器配置（HTTP URL、邮件地址、Webhook URL、回调函数、短信号码）
- 告警管理器配置（最大历史记录数、去重窗口时间）

### 1.4 输出
- 告警触发成功返回 0，失败返回错误码
- 告警恢复成功返回 0，失败返回错误码
- 活跃告警列表
- 告警历史记录列表
- 告警 JSON 格式字符串

### 1.5 前置依赖
- idcu-common 库已实现
- idcu-http-client 库已实现
- idcu-json 库已实现
- idcu-log 库已实现

---

## 2. 技术实现方案

### 2.1 核心选型
- 编程语言：C11
- 构建系统：CMake 3.15+
- 依赖库：idcu-common、idcu-http-client、idcu-json、idcu-log

### 2.2 核心逻辑
1. 初始化告警管理器，配置最大历史记录数和去重窗口
2. 创建告警规则并注册到管理器
3. 配置通知器（HTTP、邮件、Webhook、回调、短信）
4. 触发告警时，计算指纹，检查去重窗口，生成告警实例
5. 发送告警通知，记录历史，更新活跃告警列表
6. 恢复告警时，更新告警状态，发送恢复通知
7. 支持确认告警、静默告警、查询活跃告警和历史记录

### 2.3 数据结构/接口
```c
// 告警级别
typedef enum {
    IDCU_ALERT_LEVEL_INFO = 0,
    IDCU_ALERT_LEVEL_WARN,
    IDCU_ALERT_LEVEL_ERROR,
    IDCU_ALERT_LEVEL_CRITICAL
} idcu_AlertLevel;

// 告警状态
typedef enum {
    IDCU_ALERT_STATUS_ACTIVE = 0,
    IDCU_ALERT_STATUS_RESOLVED,
    IDCU_ALERT_STATUS_ACKNOWLEDGED,
    IDCU_ALERT_STATUS_SUPPRESSED
} idcu_AlertStatus;

// 告警管理器
typedef struct {
    idcu_Vector rules;
    idcu_Vector active_alerts;
    idcu_Vector alert_history;
    idcu_HashMap alerts_by_fingerprint;
    idcu_Vector notifiers;
    idcu_Mutex lock;
    uint64_t max_history_size;
    uint64_t dedup_window_ms;
    int initialized;
} idcu_AlertManager;

// 核心接口
int idcu_alert_manager_init(idcu_AlertManager* manager, const idcu_AlertManagerConfig* config);
int idcu_alert_manager_add_rule(idcu_AlertManager* manager, const idcu_AlertRule* rule);
int idcu_alert_manager_trigger(idcu_AlertManager* manager, const char* rule_name, const char* description);
int idcu_alert_manager_resolve(idcu_AlertManager* manager, const char* rule_name);
int idcu_alert_manager_add_notifier(idcu_AlertManager* manager, const idcu_AlertNotifier* notifier);
```

### 2.4 跨平台适配
- 线程同步：使用 idcu-common 库提供的跨平台互斥锁
- 时间戳获取：使用 idcu-common 库提供的跨平台时间函数
- HTTP 通知：使用 idcu-http-client 库提供的跨平台 HTTP 客户端

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以创建和管理告警规则
- [ ] 可以触发和恢复告警
- [ ] 支持 4 种告警级别（INFO/WARN/ERROR/CRITICAL）
- [ ] 支持 5 种通知类型（HTTP/邮件/Webhook/回调/短信）
- [ ] 告警去重功能正常工作
- [ ] 告警历史记录正常记录和查询
- [ ] 可以确认和静默告警

### 3.2 性能验收
- 告警触发耗时 ≤ 10ms（不含通知发送时间）
- 支持并发 100 个告警触发
- 内存占用 ≤ 2MB（包含 1000 条历史记录）
- 告警历史查询耗时 ≤ 5ms（查询 100 条记录）

### 3.3 异常验收
- 触发不存在的规则返回错误码，系统不崩溃
- 通知失败不影响告警记录，日志输出错误信息
- 历史记录超过上限时自动清理最旧记录
- 并发触发告警时数据一致性保证

---

## 4. 执行计划

### 4.1 工期
3 天/人

### 4.2 里程碑
- D1：完成接口定义和数据结构设计
- D2：完成核心逻辑实现（告警触发、恢复、去重、历史记录）
- D3：完成通知器实现、单元测试和文档编写

### 4.3 人力
1 人（技能要求：C 语言开发、跨平台开发经验）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名使用小写 + 下划线，前缀为 idcu_alert_
- 结构体前缀为 idcu_
- 宏定义使用大写 + 下划线

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 集成测试覆盖告警触发、恢复、通知、去重、历史记录等场景
- 异常测试覆盖至少 5 种异常场景

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 部署路径：`libs/idcu-alert/`
- 头文件安装路径：`include/idcu/alert/`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险 1：告警风暴
描述：短时间内大量告警触发导致系统负载过高  
应对：配置合理的去重窗口时间，限制最大活跃告警数量，实现告警静默功能

### 6.2 风险 2：通知失败
描述：网络或配置问题导致通知发送失败  
应对：通知失败不影响告警记录，日志输出详细错误信息，支持重试机制

---

## 7. 详细实现步骤

### 7.1 创建目录结构

```bash
mkdir -p libs/idcu-alert/include/idcu/alert
mkdir -p libs/idcu-alert/src/idcu/alert
mkdir -p libs/idcu-alert/tests
mkdir -p libs/idcu-alert/examples
```

### 7.2 创建告警头文件 (alert.h)

创建 `libs/idcu-alert/include/idcu/alert/alert.h`：

```c
#ifndef IDCU_ALERT_ALERT_H
#define IDCU_ALERT_ALERT_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_ALERT_LEVEL_INFO = 0,
    IDCU_ALERT_LEVEL_WARN,
    IDCU_ALERT_LEVEL_ERROR,
    IDCU_ALERT_LEVEL_CRITICAL
} idcu_AlertLevel;

typedef enum
{
    IDCU_ALERT_STATUS_ACTIVE = 0,
    IDCU_ALERT_STATUS_RESOLVED,
    IDCU_ALERT_STATUS_ACKNOWLEDGED,
    IDCU_ALERT_STATUS_SUPPRESSED
} idcu_AlertStatus;

typedef enum
{
    IDCU_ALERT_NOTIFY_HTTP = 0,
    IDCU_ALERT_NOTIFY_EMAIL,
    IDCU_ALERT_NOTIFY_WEBHOOK,
    IDCU_ALERT_NOTIFY_CALLBACK,
    IDCU_ALERT_NOTIFY_SMS
} idcu_AlertNotifyType;

typedef struct
{
    char name[128];
    idcu_AlertLevel level;
    char description[512];
    char labels[1024];
    char annotations[1024];
    uint64_t duration_ms;
    int enabled;
} idcu_AlertRule;

typedef struct
{
    uint64_t id;
    char name[128];
    idcu_AlertLevel level;
    idcu_AlertStatus status;
    char description[512];
    char labels[1024];
    char annotations[1024];
    char fingerprint[64];
    uint64_t started_at;
    uint64_t ended_at;
    uint64_t acknowledged_at;
    uint64_t last_updated_at;
    int acknowledged;
    char acknowledged_by[128];
} idcu_Alert;

typedef void (*idcu_AlertCallback)(const idcu_Alert* alert, void* user_data);

typedef struct
{
    idcu_AlertNotifyType type;
    char url[1024];
    char email[512];
    char webhook_url[1024];
    idcu_AlertCallback callback;
    void* callback_user_data;
    char sms_number[32];
    int enabled;
} idcu_AlertNotifier;

typedef struct
{
    idcu_Vector rules;
    idcu_Vector active_alerts;
    idcu_Vector alert_history;
    idcu_HashMap alerts_by_fingerprint;
    idcu_Vector notifiers;
    idcu_Mutex lock;
    uint64_t max_history_size;
    uint64_t dedup_window_ms;
    int initialized;
} idcu_AlertManager;

typedef struct
{
    uint64_t max_history_size;
    uint64_t dedup_window_ms;
} idcu_AlertManagerConfig;

int  idcu_alert_manager_config_init(idcu_AlertManagerConfig* config);

int  idcu_alert_manager_init(idcu_AlertManager* manager, const idcu_AlertManagerConfig* config);
void idcu_alert_manager_destroy(idcu_AlertManager* manager);

int  idcu_alert_rule_init(idcu_AlertRule* rule, const char* name, idcu_AlertLevel level, const char* description);
void idcu_alert_rule_destroy(idcu_AlertRule* rule);
int  idcu_alert_rule_set_label(idcu_AlertRule* rule, const char* key, const char* value);
int  idcu_alert_rule_set_annotation(idcu_AlertRule* rule, const char* key, const char* value);
int  idcu_alert_rule_set_duration(idcu_AlertRule* rule, uint64_t duration_ms);

int  idcu_alert_manager_add_rule(idcu_AlertManager* manager, const idcu_AlertRule* rule);
int  idcu_alert_manager_remove_rule(idcu_AlertManager* manager, const char* name);
idcu_AlertRule* idcu_alert_manager_get_rule(idcu_AlertManager* manager, const char* name);
int  idcu_alert_manager_enable_rule(idcu_AlertManager* manager, const char* name);
int  idcu_alert_manager_disable_rule(idcu_AlertManager* manager, const char* name);

int  idcu_alert_manager_trigger(idcu_AlertManager* manager, const char* rule_name, const char* description);
int  idcu_alert_manager_trigger_with_labels(idcu_AlertManager* manager, const char* rule_name, const char* description, const char* labels);
int  idcu_alert_manager_resolve(idcu_AlertManager* manager, const char* rule_name);
int  idcu_alert_manager_resolve_with_labels(idcu_AlertManager* manager, const char* rule_name, const char* labels);
int  idcu_alert_manager_acknowledge(idcu_AlertManager* manager, uint64_t alert_id, const char* acknowledged_by);
int  idcu_alert_manager_silence(idcu_AlertManager* manager, const char* rule_name, uint64_t duration_ms);

int  idcu_alert_manager_get_active_alerts(idcu_AlertManager* manager, idcu_Vector* alerts);
int  idcu_alert_manager_get_alert_history(idcu_AlertManager* manager, idcu_Vector* alerts, size_t limit);
size_t idcu_alert_manager_get_active_count(idcu_AlertManager* manager);

int  idcu_alert_notifier_init(idcu_AlertNotifier* notifier);
void idcu_alert_notifier_destroy(idcu_AlertNotifier* notifier);
int  idcu_alert_notifier_set_http(idcu_AlertNotifier* notifier, const char* url);
int  idcu_alert_notifier_set_email(idcu_AlertNotifier* notifier, const char* email);
int  idcu_alert_notifier_set_webhook(idcu_AlertNotifier* notifier, const char* url);
int  idcu_alert_notifier_set_callback(idcu_AlertNotifier* notifier, idcu_AlertCallback callback, void* user_data);
int  idcu_alert_notifier_set_sms(idcu_AlertNotifier* notifier, const char* number);

int  idcu_alert_manager_add_notifier(idcu_AlertManager* manager, const idcu_AlertNotifier* notifier);
int  idcu_alert_manager_remove_notifier(idcu_AlertManager* manager, size_t index);
int  idcu_alert_manager_notify_all(idcu_AlertManager* manager, const idcu_Alert* alert);

const char* idcu_alert_level_to_string(idcu_AlertLevel level);
const char* idcu_alert_status_to_string(idcu_AlertStatus status);

int  idcu_alert_init(idcu_Alert* alert);
void idcu_alert_destroy(idcu_Alert* alert);
int  idcu_alert_copy(idcu_Alert* dest, const idcu_Alert* src);
int  idcu_alert_to_json(const idcu_Alert* alert, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
```

### 7.3 创建 CMakeLists.txt

创建 `libs/idcu-alert/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-alert VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-alert STATIC
    src/idcu/alert/alert.c
)

target_include_directories(idcu-alert PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-alert PRIVATE
    idcu::common
    idcu::http-client
    idcu::json
    idcu::log
)

add_library(idcu::alert ALIAS idcu-alert)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 7.4 创建模块配置文件 (module.yaml)

创建 `libs/idcu-alert/module.yaml`：

```yaml
name: idcu-alert
version: 1.0.0
description: Alert management library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-http-client
  - idcu-json
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-alert

headers:
  - idcu/alert/alert.h

features:
  - rules: Alert rule definition and management
  - levels: Alert levels (INFO/WARN/ERROR/CRITICAL)
  - status: Alert status (active/resolved/acknowledged)
  - trigger: Alert trigger and resolve
  - notify: Alert notification (HTTP/email/webhook/callback)
  - dedup: Alert deduplication
  - history: Alert history recording
  - aggregate: Alert aggregation

testing:
  enabled: true
  framework: internal
```

### 7.5 创建 README.md

创建 `libs/idcu-alert/README.md`：

```markdown
# idcu-alert

IDCU Agent 的告警管理库。

## 功能特性

- **告警规则**: 告警规则定义和管理
- **告警级别**: 告警级别（INFO/WARN/ERROR/CRITICAL）
- **告警状态**: 告警状态（active/resolved/acknowledged）
- **触发恢复**: 告警触发和恢复
- **告警通知**: 告警通知（HTTP/邮件/Webhook/回调）
- **告警去重**: 告警去重
- **历史记录**: 告警历史记录
- **告警聚合**: 告警聚合

## 快速开始

### 初始化告警管理器

```c
#include "idcu/alert/alert.h"

idcu_AlertManagerConfig config;
idcu_alert_manager_config_init(&config);

config.max_history_size = 1000;
config.dedup_window_ms = 60000;

idcu_AlertManager manager;
idcu_alert_manager_init(&manager, &config);
```

### 创建告警规则

```c
idcu_AlertRule high_cpu_rule;
idcu_alert_rule_init(&high_cpu_rule, "high_cpu_usage", IDCU_ALERT_LEVEL_WARN, "High CPU usage detected");

idcu_alert_rule_set_label(&high_cpu_rule, "service", "api");
idcu_alert_rule_set_label(&high_cpu_rule, "severity", "warning");
idcu_alert_rule_set_annotation(&high_cpu_rule, "summary", "CPU usage is above 80%");
idcu_alert_rule_set_duration(&high_cpu_rule, 300000);

idcu_alert_manager_add_rule(&manager, &high_cpu_rule);
```

### 触发告警

```c
idcu_alert_manager_trigger(&manager, "high_cpu_usage", "CPU usage is 85%");
```

### 恢复告警

```c
idcu_alert_manager_resolve(&manager, "high_cpu_usage");
```

### 添加通知器

```c
idcu_AlertNotifier webhook_notifier;
idcu_alert_notifier_init(&webhook_notifier);
idcu_alert_notifier_set_webhook(&webhook_notifier, "https://alerts.example.com/webhook");

idcu_alert_manager_add_notifier(&manager, &webhook_notifier);
```

### 添加回调通知器

```c
void my_alert_callback(const idcu_Alert* alert, void* user_data)
{
    printf("Alert: %s, Level: %s\n", 
           alert->name, 
           idcu_alert_level_to_string(alert->level));
}

idcu_AlertNotifier callback_notifier;
idcu_alert_notifier_init(&callback_notifier);
idcu_alert_notifier_set_callback(&callback_notifier, my_alert_callback, NULL);

idcu_alert_manager_add_notifier(&manager, &callback_notifier);
```

### 获取活跃告警

```c
idcu_Vector active_alerts;
idcu_vector_init(&active_alerts, sizeof(idcu_Alert));
idcu_alert_manager_get_active_alerts(&manager, &active_alerts);

for (size_t i = 0; i < active_alerts.count; i++) {
    idcu_Alert* alert = (idcu_Alert*)idcu_vector_get(&active_alerts, i);
    printf("Alert: %s, Status: %s\n", 
           alert->name, 
           idcu_alert_status_to_string(alert->status));
}

idcu_vector_destroy(&active_alerts);
```

### 确认告警

```c
idcu_alert_manager_acknowledge(&manager, alert_id, "admin");
```

### 获取告警历史

```c
idcu_Vector history;
idcu_vector_init(&history, sizeof(idcu_Alert));
idcu_alert_manager_get_alert_history(&manager, &history, 100);

idcu_vector_destroy(&history);
```

### 销毁告警管理器

```c
idcu_alert_manager_destroy(&manager);
```

## 告警级别

| 级别 | 说明 |
|-----|------|
| INFO | 信息 |
| WARN | 警告 |
| ERROR | 错误 |
| CRITICAL | 严重 |

## 告警状态

| 状态 | 说明 |
|-----|------|
| ACTIVE | 活跃 |
| RESOLVED | 已解决 |
| ACKNOWLEDGED | 已确认 |
| SUPPRESSED | 已抑制 |

## API 文档

详见 [include/idcu/alert/alert.h](include/idcu/alert/alert.h)
```

---

## 8. 验证检查清单

- [ ] 告警头文件已创建
- [ ] 告警实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以触发和恢复告警
- [ ] 告警通知可以正常工作
- [ ] 告警历史记录正常
- [ ] 可以正常编译
- [ ] 单元测试通过
- [ ] 性能指标达标

---

## 9. Git 提交

```bash
git add libs/idcu-alert/
git commit -m "feat: add idcu-alert library

- Add alert rule definition and management
- Add alert levels (INFO/WARN/ERROR/CRITICAL)
- Add alert status (active/resolved/acknowledged)
- Add alert trigger and resolve
- Add alert notification (HTTP/email/webhook/callback)
- Add alert deduplication
- Add alert history recording
- Add alert aggregation
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 告警风暴 | 去重窗口太短 | 增加 dedup_window_ms |
| 通知失败 | 通知器配置错误 | 检查通知器配置 |
| 内存占用过高 | 历史记录太多 | 减少 max_history_size |
