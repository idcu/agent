# 任务 5.2: log-module - 日志业务模块

## 目标

创建日志业务模块，支持：
- 统一日志接口
- 多级别日志
- 多输出目标
- 日志轮转
- 日志过滤
- 日志查询
- 日志归档
- 日志分析
- 日志告警

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p modules/log-module/include/idcu/log_module
mkdir -p modules/log-module/src/idcu/log_module
mkdir -p modules/log-module/tests
mkdir -p modules/log-module/examples
```

### 2. 创建日志业务模块头文件 (log_module.h)

创建 `modules/log-module/include/idcu/log_module/log_module.h`：

```c
#ifndef IDCU_LOG_MODULE_LOG_MODULE_H
#define IDCU_LOG_MODULE_LOG_MODULE_H

#include "idcu/common/error_code.h"
#include "idcu/log/log.h"
#include "idcu/msgbus/msg_bus.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_LogModuleId;

typedef enum
{
    IDCU_LOG_OUTPUT_CONSOLE = 0,
    IDCU_LOG_OUTPUT_FILE,
    IDCU_LOG_OUTPUT_SYSLOG,
    IDCU_LOG_OUTPUT_NETWORK,
    IDCU_LOG_OUTPUT_MSGBUS
} idcu_LogOutputType;

typedef struct
{
    char name[128];
    idcu_LogOutputType type;
    idcu_LogLevel min_level;
    idcu_LogLevel max_level;
    
    char file_path[1024];
    uint64_t max_file_size;
    int max_files;
    int compress_rotated;
    
    char syslog_ident[64];
    int syslog_facility;
    
    char network_host[256];
    uint16_t network_port;
    
    char msgbus_topic[256];
    
    char pattern[512];
    int enabled;
} idcu_LogOutput;

typedef struct
{
    idcu_LogLevel min_level;
    int include_timestamp;
    int include_level;
    int include_module;
    int include_thread;
    int include_file;
    int include_line;
    char timestamp_format[128];
} idcu_LogFormat;

typedef struct
{
    char field[64];
    char pattern[256];
    int exclude;
} idcu_LogFilter;

typedef struct
{
    idcu_LogModuleId id;
    char name[128];
    
    idcu_Logger logger;
    idcu_Vector outputs;
    idcu_HashMap outputs_by_name;
    idcu_Mutex lock;
    
    idcu_LogFormat format;
    idcu_Vector filters;
    
    idcu_Vector log_buffer;
    size_t max_buffer_size;
    
    idcu_MsgBus* msg_bus;
    char msgbus_topic[256];
    
    uint64_t total_logs;
    uint64_t logs_by_level[6];
    
    int initialized;
} idcu_LogModule;

typedef struct
{
    uint64_t id;
    uint64_t timestamp;
    idcu_LogLevel level;
    char module[128];
    char thread[128];
    char file[256];
    int line;
    char message[2048];
} idcu_LogEntry;

typedef struct
{
    idcu_LogLevel min_level;
    idcu_LogLevel max_level;
    uint64_t start_time;
    uint64_t end_time;
    char module_pattern[128];
    char message_pattern[256];
    size_t limit;
    size_t offset;
} idcu_LogQuery;

typedef struct
{
    idcu_LogLevel alert_level;
    char pattern[256];
    int threshold_count;
    uint64_t threshold_window_ms;
    char alert_topic[256];
} idcu_LogAlertRule;

typedef struct
{
    idcu_LogModule* log_module;
    idcu_Vector alert_rules;
    idcu_Mutex lock;
    idcu_Thread alert_thread;
    int running;
} idcu_LogAlertManager;

int  idcu_log_module_init(idcu_LogModule* lm, const char* name);
void idcu_log_module_destroy(idcu_LogModule* lm);
int  idcu_log_module_start(idcu_LogModule* lm);
void idcu_log_module_stop(idcu_LogModule* lm);

idcu_LogModuleId idcu_log_module_add_output(idcu_LogModule* lm, const idcu_LogOutput* output);
int  idcu_log_module_remove_output(idcu_LogModule* lm, idcu_LogModuleId id);
idcu_LogOutput* idcu_log_module_get_output(idcu_LogModule* lm, idcu_LogModuleId id);
idcu_LogOutput* idcu_log_module_get_output_by_name(idcu_LogModule* lm, const char* name);
int  idcu_log_module_enable_output(idcu_LogModule* lm, idcu_LogModuleId id);
int  idcu_log_module_disable_output(idcu_LogModule* lm, idcu_LogModuleId id);

int  idcu_log_module_set_format(idcu_LogModule* lm, const idcu_LogFormat* format);
int  idcu_log_module_set_level(idcu_LogModule* lm, idcu_LogLevel level);

int  idcu_log_module_add_filter(idcu_LogModule* lm, const idcu_LogFilter* filter);
int  idcu_log_module_remove_filter(idcu_LogModule* lm, size_t index);
int  idcu_log_module_clear_filters(idcu_LogModule* lm);

int  idcu_log_module_set_msgbus(idcu_LogModule* lm, idcu_MsgBus* msg_bus, const char* topic);

void idcu_log_module_log(idcu_LogModule* lm, idcu_LogLevel level, const char* module, 
                          const char* file, int line, const char* format, ...);
void idcu_log_module_debug(idcu_LogModule* lm, const char* module, const char* format, ...);
void idcu_log_module_info(idcu_LogModule* lm, const char* module, const char* format, ...);
void idcu_log_module_warn(idcu_LogModule* lm, const char* module, const char* format, ...);
void idcu_log_module_error(idcu_LogModule* lm, const char* module, const char* format, ...);
void idcu_log_module_fatal(idcu_LogModule* lm, const char* module, const char* format, ...);

int  idcu_log_module_query(idcu_LogModule* lm, const idcu_LogQuery* query, idcu_Vector* results);
int  idcu_log_module_get_recent(idcu_LogModule* lm, size_t count, idcu_Vector* results);
int  idcu_log_module_clear_buffer(idcu_LogModule* lm);

int  idcu_log_module_get_stats(idcu_LogModule* lm, uint64_t* total, uint64_t* by_level);
int  idcu_log_module_reset_stats(idcu_LogModule* lm);

int  idcu_log_module_archive(idcu_LogModule* lm, const char* output_path, uint64_t start_time, uint64_t end_time);
int  idcu_log_module_rotate(idcu_LogModule* lm, idcu_LogModuleId output_id);
int  idcu_log_module_rotate_all(idcu_LogModule* lm);

int  idcu_log_output_init(idcu_LogOutput* output, const char* name, idcu_LogOutputType type);
void idcu_log_output_destroy(idcu_LogOutput* output);
int  idcu_log_output_set_file(idcu_LogOutput* output, const char* path, uint64_t max_size, int max_files);
int  idcu_log_output_set_syslog(idcu_LogOutput* output, const char* ident, int facility);
int  idcu_log_output_set_network(idcu_LogOutput* output, const char* host, uint16_t port);
int  idcu_log_output_set_msgbus(idcu_LogOutput* output, const char* topic);
int  idcu_log_output_set_level(idcu_LogOutput* output, idcu_LogLevel min_level, idcu_LogLevel max_level);
int  idcu_log_output_set_pattern(idcu_LogOutput* output, const char* pattern);

int  idcu_log_format_init(idcu_LogFormat* format);
void idcu_log_format_destroy(idcu_LogFormat* format);

int  idcu_log_filter_init(idcu_LogFilter* filter, const char* field, const char* pattern, int exclude);
void idcu_log_filter_destroy(idcu_LogFilter* filter);

int  idcu_log_query_init(idcu_LogQuery* query);
void idcu_log_query_destroy(idcu_LogQuery* query);
int  idcu_log_query_set_level(idcu_LogQuery* query, idcu_LogLevel min_level, idcu_LogLevel max_level);
int  idcu_log_query_set_time(idcu_LogQuery* query, uint64_t start_time, uint64_t end_time);
int  idcu_log_query_set_patterns(idcu_LogQuery* query, const char* module_pattern, const char* message_pattern);
int  idcu_log_query_set_limit(idcu_LogQuery* query, size_t limit, size_t offset);

int  idcu_log_entry_init(idcu_LogEntry* entry);
void idcu_log_entry_destroy(idcu_LogEntry* entry);
int  idcu_log_entry_to_json(const idcu_LogEntry* entry, char* buffer, size_t buffer_size);
int  idcu_log_entry_from_json(idcu_LogEntry* entry, const char* json);
int  idcu_log_entry_format(const idcu_LogEntry* entry, const char* pattern, char* buffer, size_t buffer_size);

int  idcu_log_alert_manager_init(idcu_LogAlertManager* manager, idcu_LogModule* log_module);
void idcu_log_alert_manager_destroy(idcu_LogAlertManager* manager);
int  idcu_log_alert_manager_start(idcu_LogAlertManager* manager);
void idcu_log_alert_manager_stop(idcu_LogAlertManager* manager);
int  idcu_log_alert_manager_add_rule(idcu_LogAlertManager* manager, const idcu_LogAlertRule* rule);
int  idcu_log_alert_manager_remove_rule(idcu_LogAlertManager* manager, size_t index);

int  idcu_log_alert_rule_init(idcu_LogAlertRule* rule);
void idcu_log_alert_rule_destroy(idcu_log_alertRule* rule);
int  idcu_log_alert_rule_set_level(idcu_LogAlertRule* rule, idcu_LogLevel level);
int  idcu_log_alert_rule_set_pattern(idcu_LogAlertRule* rule, const char* pattern);
int  idcu_log_alert_rule_set_threshold(idcu_LogAlertRule* rule, int count, uint64_t window_ms);
int  idcu_log_alert_rule_set_topic(idcu_LogAlertRule* rule, const char* topic);

int  idcu_log_module_export_json(idcu_LogModule* lm, const idcu_LogQuery* query, char* buffer, size_t buffer_size);
int  idcu_log_module_get_info(idcu_LogModule* lm, char* buffer, size_t buffer_size);
int  idcu_log_module_get_info_json(idcu_LogModule* lm, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `modules/log-module/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(log-module VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(log-module STATIC
    src/idcu/log_module/log_module.c
    src/idcu/log_module/log_output.c
    src/idcu/log_module/log_query.c
    src/idcu/log_module/log_alert.c
)

target_include_directories(log-module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(log-module PRIVATE
    idcu::common
    idcu::log
    idcu::msgbus
    idcu::json
    idcu::utils
    idcu::storage
)

add_library(idcu::log-module ALIAS log-module)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `modules/log-module/module.yaml`：

```yaml
name: log-module
version: 1.0.0
description: Log business module for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-log
  - idcu-msgbus
  - idcu-json
  - idcu-utils
  - idcu-storage

build:
  type: cmake
  targets:
    - log-module

headers:
  - idcu/log_module/log_module.h

features:
  - unified: Unified log interface
  - levels: Multi-level logging
  - outputs: Multiple output targets
  - rotation: Log rotation
  - filter: Log filtering
  - query: Log query
  - archive: Log archive
  - analysis: Log analysis
  - alert: Log alert
  - msgbus: Message bus integration

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `modules/log-module/README.md`：

```markdown
# log-module

IDCU Agent 的日志业务模块。

## 功能特性

- **统一接口**: 统一日志接口
- **多级别日志**: 多级别日志
- **多输出目标**: 多输出目标
- **日志轮转**: 日志轮转
- **日志过滤**: 日志过滤
- **日志查询**: 日志查询
- **日志归档**: 日志归档
- **日志分析**: 日志分析
- **日志告警**: 日志告警
- **消息总线**: 消息总线集成

## 快速开始

### 初始化日志模块

```c
#include "idcu/log_module/log_module.h"

idcu_LogModule lm;
idcu_log_module_init(&lm, "my-app");
```

### 配置日志格式

```c
idcu_LogFormat format;
idcu_log_format_init(&format);

format.min_level = IDCU_LOG_LEVEL_INFO;
format.include_timestamp = 1;
format.include_level = 1;
format.include_module = 1;
format.include_file = 1;
format.include_line = 1;
strncpy(format.timestamp_format, "%Y-%m-%d %H:%M:%S", sizeof(format.timestamp_format));

idcu_log_module_set_format(&lm, &format);
```

### 添加控制台输出

```c
idcu_LogOutput console_output;
idcu_log_output_init(&console_output, "console", IDCU_LOG_OUTPUT_CONSOLE);
console_output.enabled = 1;
idcu_log_output_set_level(&console_output, IDCU_LOG_LEVEL_DEBUG, IDCU_LOG_LEVEL_FATAL);

idcu_log_module_add_output(&lm, &console_output);
```

### 添加文件输出

```c
idcu_LogOutput file_output;
idcu_log_output_init(&file_output, "file", IDCU_LOG_OUTPUT_FILE);
file_output.enabled = 1;
idcu_log_output_set_file(&file_output, "./logs/app.log", 100 * 1024 * 1024, 10);
idcu_log_output_set_level(&file_output, IDCU_LOG_LEVEL_INFO, IDCU_LOG_LEVEL_FATAL);

idcu_log_module_add_output(&lm, &file_output);
```

### 添加消息总线输出

```c
idcu_LogOutput msgbus_output;
idcu_log_output_init(&msgbus_output, "msgbus", IDCU_LOG_OUTPUT_MSGBUS);
msgbus_output.enabled = 1;
idcu_log_output_set_msgbus(&msgbus_output, "logs");
idcu_log_output_set_level(&msgbus_output, IDCU_LOG_LEVEL_WARN, IDCU_LOG_LEVEL_FATAL);

idcu_log_module_add_output(&lm, &msgbus_output);
```

### 设置消息总线

```c
idcu_log_module_set_msgbus(&lm, msg_bus, "logs");
```

### 启动日志模块

```c
idcu_log_module_start(&lm);
```

### 记录日志

```c
idcu_log_module_debug(&lm, "main", "Debug message");
idcu_log_module_info(&lm, "main", "Application started");
idcu_log_module_warn(&lm, "main", "Warning: low memory");
idcu_log_module_error(&lm, "main", "Error: connection failed");
idcu_log_module_fatal(&lm, "main", "Fatal error: shutting down");
```

### 添加过滤

```c
idcu_LogFilter filter;
idcu_log_filter_init(&filter, "module", "debug", 1);

idcu_log_module_add_filter(&lm, &filter);
```

### 查询日志

```c
idcu_LogQuery query;
idcu_log_query_init(&query);

idcu_log_query_set_level(&query, IDCU_LOG_LEVEL_INFO, IDCU_LOG_LEVEL_FATAL);
idcu_log_query_set_limit(&query, 100, 0);

idcu_Vector results;
idcu_vector_init(&results, sizeof(idcu_LogEntry));

idcu_log_module_query(&lm, &query, &results);

for (size_t i = 0; i < results.count; i++) {
    idcu_LogEntry* entry = (idcu_LogEntry*)idcu_vector_get(&results, i);
    printf("[%s] %s: %s\n", 
           idcu_log_level_to_string(entry->level),
           entry->module,
           entry->message);
}

idcu_vector_destroy(&results);
idcu_log_query_destroy(&query);
```

### 获取最近日志

```c
idcu_Vector recent;
idcu_vector_init(&recent, sizeof(idcu_LogEntry));

idcu_log_module_get_recent(&lm, 50, &recent);

idcu_vector_destroy(&recent);
```

### 获取统计

```c
uint64_t total;
uint64_t by_level[6];

idcu_log_module_get_stats(&lm, &total, by_level);

printf("Total logs: %" PRIu64 "\n", total);
printf("Debug: %" PRIu64 "\n", by_level[IDCU_LOG_LEVEL_DEBUG]);
printf("Info: %" PRIu64 "\n", by_level[IDCU_LOG_LEVEL_INFO]);
printf("Warn: %" PRIu64 "\n", by_level[IDCU_LOG_LEVEL_WARN]);
printf("Error: %" PRIu64 "\n", by_level[IDCU_LOG_LEVEL_ERROR]);
printf("Fatal: %" PRIu64 "\n", by_level[IDCU_LOG_LEVEL_FATAL]);
```

### 重置统计

```c
idcu_log_module_reset_stats(&lm);
```

### 日志轮转

```c
idcu_log_module_rotate_all(&lm);
```

### 日志归档

```c
uint64_t end_time = idcu_time_now_ms();
uint64_t start_time = end_time - 24 * 60 * 60 * 1000;

idcu_log_module_archive(&lm, "./logs/archive.log", start_time, end_time);
```

### 日志告警

```c
idcu_LogAlertManager alert_manager;
idcu_log_alert_manager_init(&alert_manager, &lm);

idcu_LogAlertRule error_rule;
idcu_log_alert_rule_init(&error_rule);
idcu_log_alert_rule_set_level(&error_rule, IDCU_LOG_LEVEL_ERROR);
idcu_log_alert_rule_set_threshold(&error_rule, 10, 60000);
idcu_log_alert_rule_set_topic(&error_rule, "alerts");

idcu_log_alert_manager_add_rule(&alert_manager, &error_rule);

idcu_log_alert_manager_start(&alert_manager);
```

### 导出 JSON

```c
char json_buffer[8192];

idcu_LogQuery query;
idcu_log_query_init(&query);
idcu_log_query_set_level(&query, IDCU_LOG_LEVEL_INFO, IDCU_LOG_LEVEL_FATAL);
idcu_log_query_set_limit(&query, 100, 0);

idcu_log_module_export_json(&lm, &query, json_buffer, sizeof(json_buffer));
printf("%s\n", json_buffer);

idcu_log_query_destroy(&query);
```

### 获取信息

```c
char info_buffer[2048];
idcu_log_module_get_info(&lm, info_buffer, sizeof(info_buffer));
printf("%s\n", info_buffer);

char json_buffer[4096];
idcu_log_module_get_info_json(&lm, json_buffer, sizeof(json_buffer));
printf("%s\n", json_buffer);
```

### 停止日志模块

```c
idcu_log_alert_manager_stop(&alert_manager);
idcu_log_alert_manager_destroy(&alert_manager);

idcu_log_module_stop(&lm);
idcu_log_module_destroy(&lm);
```

## 日志级别

| 级别 | 说明 |
|-----|------|
| DEBUG | 调试 |
| INFO | 信息 |
| WARN | 警告 |
| ERROR | 错误 |
| FATAL | 严重 |

## 输出类型

| 类型 | 说明 |
|-----|------|
| CONSOLE | 控制台 |
| FILE | 文件 |
| SYSLOG | 系统日志 |
| NETWORK | 网络 |
| MSGBUS | 消息总线 |

## API 文档

详见 [include/idcu/log_module/log_module.h](include/idcu/log_module/log_module.h)
```

## 验证检查清单

- [ ] 日志业务模块头文件已创建
- [ ] 日志业务模块实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以记录日志到多个输出
- [ ] 日志查询正常工作
- [ ] 日志告警正常工作

## Git 提交

```bash
git add modules/log-module/
git commit -m "feat: add log-module module

- Add unified log interface
- Add multi-level logging
- Add multiple output targets
- Add log rotation
- Add log filtering
- Add log query
- Add log archive
- Add log analysis
- Add log alert
- Add message bus integration
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 日志未输出 | 输出被禁用 | 确保输出已启用 |
| 日志级别不对 | 级别设置错误 | 检查日志级别配置 |
| 轮转不工作 | 文件权限问题 | 检查文件权限 |
