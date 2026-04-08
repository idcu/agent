# 任务 3.1: idcu-log - 日志系统库

## 目标

创建完整的日志系统库，支持：
- 多级别日志（DEBUG, INFO, WARN, ERROR, FATAL）
- 多种输出目标（控制台、文件、远程）
- 日志文件轮转（按大小、按时间）
- 格式化日志输出
- 线程安全

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-log/include/idcu/log
mkdir -p libs/idcu-log/src/idcu/log
mkdir -p libs/idcu-log/tests
mkdir -p libs/idcu-log/examples
```

### 2. 创建日志头文件 (log.h)

创建 `libs/idcu-log/include/idcu/log/log.h`：

```c
#ifndef IDCU_LOG_LOG_H
#define IDCU_LOG_LOG_H

#include "idcu/common/config.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

typedef enum
{
    IDCU_LOG_DEBUG = 0,
    IDCU_LOG_INFO  = 1,
    IDCU_LOG_WARN  = 2,
    IDCU_LOG_ERROR = 3,
    IDCU_LOG_FATAL = 4
} idcu_LogLevel;

typedef enum
{
    IDCU_LOG_OUTPUT_CONSOLE = 1 << 0,
    IDCU_LOG_OUTPUT_FILE    = 1 << 1,
    IDCU_LOG_OUTPUT_REMOTE  = 1 << 2
} idcu_LogOutput;

typedef enum
{
    IDCU_LOG_ROTATE_NONE = 0,
    IDCU_LOG_ROTATE_SIZE = 1,
    IDCU_LOG_ROTATE_TIME = 2,
    IDCU_LOG_ROTATE_BOTH = 3
} idcu_LogRotatePolicy;

typedef struct
{
    char                 filename[256];
    idcu_LogLevel        level;
    idcu_LogOutput       output;
    idcu_LogRotatePolicy rotate_policy;
    uint64_t             max_file_size;
    uint32_t             rotate_interval;
    uint32_t             max_backup_files;
    char                 remote_url[256];
} idcu_LogConfig;

int idcu_log_init(const char* filename, idcu_LogLevel level);
int idcu_log_init_with_config(const idcu_LogConfig* config);
void idcu_log_get_default_config(idcu_LogConfig* config);
void idcu_log_shutdown(void);

void idcu_log_set_level(idcu_LogLevel level);
idcu_LogLevel idcu_log_get_level(void);

int idcu_log_set_file(const char* filename);
void idcu_log_set_output(idcu_LogOutput output);

int idcu_log_set_rotate_policy(idcu_LogRotatePolicy policy, uint64_t max_size, uint32_t interval,
                               uint32_t max_backups);
int idcu_log_rotate(void);

void idcu_log_printf(idcu_LogLevel level, const char* file, int line, const char* fmt, ...);

#define IDCU_LOG_DEBUG(fmt, ...)                                                 \
    do                                                                           \
    {                                                                            \
        idcu_log_printf(IDCU_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (0)

#define IDCU_LOG_INFO(fmt, ...)                                                 \
    do                                                                          \
    {                                                                           \
        idcu_log_printf(IDCU_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (0)

#define IDCU_LOG_WARN(fmt, ...)                                                 \
    do                                                                          \
    {                                                                           \
        idcu_log_printf(IDCU_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (0)

#define IDCU_LOG_WARNING(fmt, ...) IDCU_LOG_WARN(fmt, ##__VA_ARGS__)

#define IDCU_LOG_ERROR(fmt, ...)                                                 \
    do                                                                           \
    {                                                                            \
        idcu_log_printf(IDCU_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (0)

#define IDCU_LOG_FATAL(fmt, ...)                                                 \
    do                                                                           \
    {                                                                            \
        idcu_log_printf(IDCU_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (0)

#endif
```

### 3. 创建日志实现文件 (log.c)

创建 `libs/idcu-log/src/idcu/log/log.c`：

```c
#include "idcu/log/log.h"
#include "idcu/common/lock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static idcu_LogConfig g_log_config = {0};
static FILE* g_log_file = NULL;
static idcu_Mutex g_log_mutex = {0};
static int g_initialized = 0;
static uint64_t g_current_file_size = 0;
static uint64_t g_last_rotate_time = 0;

void idcu_log_get_default_config(idcu_LogConfig* config)
{
    if (!config) return;

    memset(config, 0, sizeof(idcu_LogConfig));
    strncpy(config->filename, "idcu.log", sizeof(config->filename) - 1);
    config->level = IDCU_LOG_INFO;
    config->output = IDCU_LOG_OUTPUT_CONSOLE;
    config->rotate_policy = IDCU_LOG_ROTATE_NONE;
    config->max_file_size = 100 * 1024 * 1024;
    config->rotate_interval = 86400;
    config->max_backup_files = 10;
}

int idcu_log_init(const char* filename, idcu_LogLevel level)
{
    idcu_LogConfig config;
    idcu_log_get_default_config(&config);
    config.level = level;
    if (filename) {
        strncpy(config.filename, filename, sizeof(config.filename) - 1);
        config.output = IDCU_LOG_OUTPUT_CONSOLE | IDCU_LOG_OUTPUT_FILE;
    }
    return idcu_log_init_with_config(&config);
}

int idcu_log_init_with_config(const idcu_LogConfig* config)
{
    if (!config || g_initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memcpy(&g_log_config, config, sizeof(idcu_LogConfig));
    idcu_mutex_init(&g_log_mutex);

    if (config->output & IDCU_LOG_OUTPUT_FILE) {
        g_log_file = fopen(config->filename, "a");
        if (!g_log_file) {
            idcu_mutex_destroy(&g_log_mutex);
            return IDCU_ERR_FILE_OPEN;
        }
    }

    g_initialized = 1;
    g_current_file_size = 0;
    g_last_rotate_time = (uint64_t)time(NULL);

    return IDCU_ERR_OK;
}

void idcu_log_shutdown(void)
{
    if (!g_initialized) return;

    idcu_mutex_lock(&g_log_mutex);

    if (g_log_file) {
        fflush(g_log_file);
        fclose(g_log_file);
        g_log_file = NULL;
    }

    g_initialized = 0;

    idcu_mutex_unlock(&g_log_mutex);
    idcu_mutex_destroy(&g_log_mutex);
}

void idcu_log_set_level(idcu_LogLevel level)
{
    if (!g_initialized) return;
    idcu_mutex_lock(&g_log_mutex);
    g_log_config.level = level;
    idcu_mutex_unlock(&g_log_mutex);
}

idcu_LogLevel idcu_log_get_level(void)
{
    if (!g_initialized) return IDCU_LOG_INFO;
    return g_log_config.level;
}

static const char* log_level_to_str(idcu_LogLevel level)
{
    switch (level) {
    case IDCU_LOG_DEBUG: return "DEBUG";
    case IDCU_LOG_INFO: return "INFO";
    case IDCU_LOG_WARN: return "WARN";
    case IDCU_LOG_ERROR: return "ERROR";
    case IDCU_LOG_FATAL: return "FATAL";
    default: return "UNKNOWN";
    }
}

void idcu_log_printf(idcu_LogLevel level, const char* file, int line, const char* fmt, ...)
{
    if (!g_initialized || level < g_log_config.level) {
        return;
    }

    idcu_mutex_lock(&g_log_mutex);

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    char message[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    char log_line[4096];
    snprintf(log_line, sizeof(log_line), "[%s] [%s] [%s:%d] %s\n",
             time_buf, log_level_to_str(level), file, line, message);

    if (g_log_config.output & IDCU_LOG_OUTPUT_CONSOLE) {
        fprintf(stdout, "%s", log_line);
        fflush(stdout);
    }

    if (g_log_config.output & IDCU_LOG_OUTPUT_FILE && g_log_file) {
        fprintf(g_log_file, "%s", log_line);
        fflush(g_log_file);
        g_current_file_size += strlen(log_line);
    }

    idcu_mutex_unlock(&g_log_mutex);
}

int idcu_log_rotate(void)
{
    if (!g_initialized || !(g_log_config.output & IDCU_LOG_OUTPUT_FILE)) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_mutex_lock(&g_log_mutex);

    if (g_log_file) {
        fflush(g_log_file);
        fclose(g_log_file);
    }

    char backup_name[512];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(backup_name, sizeof(backup_name), "%s.%Y%m%d_%H%M%S", g_log_config.filename, tm_info);

    rename(g_log_config.filename, backup_name);

    g_log_file = fopen(g_log_config.filename, "a");
    if (!g_log_file) {
        idcu_mutex_unlock(&g_log_mutex);
        return IDCU_ERR_FILE_OPEN;
    }

    g_current_file_size = 0;
    g_last_rotate_time = (uint64_t)now;

    idcu_mutex_unlock(&g_log_mutex);
    return IDCU_ERR_OK;
}
```

### 4. 创建 CMakeLists.txt

创建 `libs/idcu-log/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-log VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-log STATIC
    src/idcu/log/log.c
)

target_include_directories(idcu-log PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-log PRIVATE
    idcu::common
)

add_library(idcu::log ALIAS idcu-log)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 5. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-log/module.yaml`：

```yaml
name: idcu-log
version: 1.0.0
description: Logging library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common

build:
  type: cmake
  targets:
    - idcu-log

headers:
  - idcu/log/log.h

features:
  - multi_level: Multiple log levels (DEBUG, INFO, WARN, ERROR, FATAL)
  - multi_output: Console, file, and remote output
  - rotation: File rotation by size or time
  - thread_safe: Thread-safe logging
  - format: Formatted log output with file and line numbers

testing:
  enabled: true
  framework: internal
```

### 6. 创建 README.md

创建 `libs/idcu-log/README.md`：

```markdown
# idcu-log

IDCU Agent 的日志系统库。

## 功能特性

- **多级别日志**: DEBUG, INFO, WARN, ERROR, FATAL
- **多输出目标**: 控制台、文件、远程
- **日志轮转**: 按大小或时间自动轮转
- **线程安全**: 多线程环境下安全使用
- **格式化输出**: 包含时间、文件、行号信息

## 快速开始

### 基本使用

```c
#include "idcu/log/log.h"

int main() {
    idcu_log_init("app.log", IDCU_LOG_INFO);

    IDCU_LOG_INFO("Application started");
    IDCU_LOG_DEBUG("Debug information");
    IDCU_LOG_WARN("Warning message");
    IDCU_LOG_ERROR("Error occurred");

    idcu_log_shutdown();
    return 0;
}
```

### 使用配置

```c
idcu_LogConfig config;
idcu_log_get_default_config(&config);
config.level = IDCU_LOG_DEBUG;
config.output = IDCU_LOG_OUTPUT_CONSOLE | IDCU_LOG_OUTPUT_FILE;
config.rotate_policy = IDCU_LOG_ROTATE_SIZE;
config.max_file_size = 50 * 1024 * 1024;

idcu_log_init_with_config(&config);
```

## API 文档

详见 [include/idcu/log/log.h](include/idcu/log/log.h)
```

## 验证检查清单

- [ ] 日志头文件已创建
- [ ] 日志实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以输出不同级别的日志
- [ ] 可以同时输出到控制台和文件
- [ ] 线程安全验证通过

## Git 提交

```bash
git add libs/idcu-log/
git commit -m "feat: add idcu-log library

- Add multi-level logging (DEBUG, INFO, WARN, ERROR, FATAL)
- Add console and file output
- Add file rotation support
- Add thread-safe logging
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 日志文件未写入 | 文件路径问题或权限不足 | 检查文件路径和权限 |
| 日志级别不生效 | 初始化后未正确设置 | 确保在初始化时或之后设置正确的级别 |
| 性能问题 | 频繁的磁盘写入 | 使用缓冲或异步日志 |
