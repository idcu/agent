# 任务 3.17: idcu-alert - 告警管理库

## 目标

创建告警管理库，支持：
- 告警定义和管理
- 告警触发和恢复
- 告警级别（INFO/WARN/ERROR/CRITICAL）
- 告警通知（HTTP/邮件/回调）
- 告警去重
- 告警历史记录
- 告警聚合

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-alert/include/idcu/alert
mkdir -p libs/idcu-alert/src/idcu/alert
mkdir -p libs/idcu-alert/tests
mkdir -p libs/idcu-alert/examples
```

### 2. 创建告警头文件 (alert.h)

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

### 3. 创建 CMakeLists.txt

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

### 4. 创建模块配置文件 (module.yaml)

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

### 5. 创建 README.md

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

## 验证检查清单

- [ ] 告警头文件已创建
- [ ] 告警实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以触发和恢复告警
- [ ] 告警通知可以正常工作
- [ ] 告警历史记录正常

## Git 提交

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

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 告警风暴 | 去重窗口太短 | 增加 dedup_window_ms |
| 通知失败 | 通知器配置错误 | 检查通知器配置 |
| 内存占用过高 | 历史记录太多 | 减少 max_history_size |
