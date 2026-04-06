# idcu-alert

IDCU 项目的独立告警库，提供告警规则管理、事件通知功能。

## 特性

- 跨平台支持（Windows、Linux、macOS）
- 支持多种告警级别：INFO、WARNING、ERROR、CRITICAL
- 灵活的告警规则配置
- 支持多种条件判断：大于、小于、等于、不等于等
- 支持告警标签和自定义消息
- 可配置的告警触发延迟
- 支持多种通知渠道：日志、文件、Webhook（待实现）
- 线程安全的操作
- 告警事件历史记录

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-common REQUIRED)
find_package(idcu-log REQUIRED)
find_package(idcu-metrics REQUIRED)
find_package(idcu-alert REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::alert)
```

## API 文档

### 初始化和清理

```c
#include <idcu/alert/alert.h>

// 初始化告警管理器
int idcu_alert_manager_init(idcu_AlertManager* mgr);

// 销毁告警管理器
void idcu_alert_manager_destroy(idcu_AlertManager* mgr);
```

### 告警规则管理

```c
// 初始化告警规则
void idcu_alert_rule_init(idcu_AlertRule* rule, const char* name, idcu_AlertLevel level,
                           const char* metric_name, idcu_AlertCondition cond, uint64_t threshold);

// 添加标签
int idcu_alert_rule_add_label(idcu_AlertRule* rule, const char* key, const char* value);

// 设置自定义消息
void idcu_alert_rule_set_message(idcu_AlertRule* rule, const char* msg);

// 设置触发延迟
void idcu_alert_rule_set_pending_duration(idcu_AlertRule* rule, uint64_t ms);

// 添加告警规则
int idcu_alert_manager_add_rule(idcu_AlertManager* mgr, const idcu_AlertRule* rule);

// 移除告警规则
int idcu_alert_manager_remove_rule(idcu_AlertManager* mgr, const char* name);
```

### 告警评估和事件

```c
// 设置告警回调
int idcu_alert_manager_set_callback(idcu_AlertManager* mgr, idcu_AlertCallback cb, void* user_data);

// 评估告警规则
int idcu_alert_manager_evaluate(idcu_AlertManager* mgr, idcu_MetricsCollector* metrics);

// 获取告警事件
int idcu_alert_manager_get_events(idcu_AlertManager* mgr, idcu_AlertEvent* events, int max_events, int* count);

// 清除已解决的告警
int idcu_alert_manager_clear_resolved(idcu_AlertManager* mgr);
```

### 通知器

```c
#include <idcu/alert/notifier.h>

// 初始化通知器
int idcu_notifier_init(idcu_Notifier* notifier);

// 销毁通知器
void idcu_notifier_destroy(idcu_Notifier* notifier);

// 添加日志通知渠道
int idcu_notifier_add_log_channel(idcu_Notifier* notifier);

// 添加文件通知渠道
int idcu_notifier_add_file_channel(idcu_Notifier* notifier, const char* file_path);

// 添加Webhook通知渠道
int idcu_notifier_add_webhook_channel(idcu_Notifier* notifier, const char* url, int timeout_ms);

// 发送通知
int idcu_notifier_send(idcu_Notifier* notifier, const idcu_AlertEvent* event);

// 告警回调适配器
void idcu_notifier_alert_callback(const idcu_AlertEvent* event, void* user_data);
```

### 告警级别

```c
typedef enum {
    IDCU_ALERT_LEVEL_INFO = 0,
    IDCU_ALERT_LEVEL_WARNING,
    IDCU_ALERT_LEVEL_ERROR,
    IDCU_ALERT_LEVEL_CRITICAL
} idcu_AlertLevel;
```

### 告警条件

```c
typedef enum {
    IDCU_ALERT_COND_GREATER = 0,
    IDCU_ALERT_COND_LESS,
    IDCU_ALERT_COND_EQUAL,
    IDCU_ALERT_COND_NOT_EQUAL,
    IDCU_ALERT_COND_GREATER_EQUAL,
    IDCU_ALERT_COND_LESS_EQUAL
} idcu_AlertCondition;
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [基本告警示例](examples/example_alert_basic.c) - 演示告警管理的基本使用

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

# 运行示例
./example_alert_basic
```

## 依赖

- idcu-common - IDCU 基础通用组件库
- idcu-log - IDCU 日志库
- idcu-metrics - IDCU 指标库

## 许可证

详见项目根目录的 LICENSE 文件。
