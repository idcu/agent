# idcu-log API 文档

## 概述

idcu-log 是一个轻量级、高效的 C 语言日志库，提供灵活的日志功能，支持控制台输出和文件输出。

## 头文件

```c
#include <idcu/log/log.h>
```

## 数据类型

### idcu_LogLevel

日志级别枚举。

```c
typedef enum {
    IDCU_LOG_DEBUG   = 0,  // 调试信息
    IDCU_LOG_INFO    = 1,  // 一般信息
    IDCU_LOG_WARN    = 2,  // 警告信息
    IDCU_LOG_ERROR   = 3,  // 错误信息
    IDCU_LOG_FATAL   = 4   // 致命错误
} idcu_LogLevel;
```

日志级别从低到高，DEBUG < INFO < WARN < ERROR < FATAL。

## 函数

### idcu_log_init

初始化日志系统。

**函数原型**：
```c
int idcu_log_init(const char* filename, idcu_LogLevel level);
```

**参数**：
- `filename` - 日志文件名，如果为 NULL 则仅输出到控制台
- `level` - 初始日志级别

**返回值**：
- `IDCU_ERR_SUCCESS` - 初始化成功
- `IDCU_ERR_GENERAL` - 初始化失败（无法打开日志文件）

**示例**：
```c
// 仅输出到控制台，级别为 INFO
idcu_log_init(NULL, IDCU_LOG_INFO);

// 同时输出到控制台和文件，级别为 DEBUG
idcu_log_init("app.log", IDCU_LOG_DEBUG);
```

---

### idcu_log_shutdown

关闭日志系统，释放资源。

**函数原型**：
```c
void idcu_log_shutdown(void);
```

**参数**：
- 无

**返回值**：
- 无

**示例**：
```c
idcu_log_shutdown();
```

---

### idcu_log_set_level

设置日志级别。

**函数原型**：
```c
void idcu_log_set_level(idcu_LogLevel level);
```

**参数**：
- `level` - 新的日志级别

**返回值**：
- 无

**示例**：
```c
// 设置日志级别为 WARN，仅显示 WARN 及以上级别的日志
idcu_log_set_level(IDCU_LOG_WARN);
```

---

### idcu_log_get_level

获取当前日志级别。

**函数原型**：
```c
idcu_LogLevel idcu_log_get_level(void);
```

**参数**：
- 无

**返回值**：
- 当前日志级别

**示例**：
```c
idcu_LogLevel level = idcu_log_get_level();
```

---

### idcu_log_set_file

设置日志文件。

**函数原型**：
```c
int idcu_log_set_file(const char* filename);
```

**参数**：
- `filename` - 新的日志文件名，如果为 NULL 则关闭文件输出

**返回值**：
- `IDCU_ERR_SUCCESS` - 设置成功
- `IDCU_ERR_GENERAL` - 设置失败（无法打开日志文件）

**示例**：
```c
// 设置日志文件
idcu_log_set_file("app.log");

// 关闭文件输出
idcu_log_set_file(NULL);
```

---

### idcu_log_printf

底层日志输出函数（不建议直接使用）。

**函数原型**：
```c
void idcu_log_printf(idcu_LogLevel level, const char* file, int line, const char* fmt, ...);
```

**说明**：
建议使用宏封装的日志宏。

## 日志宏

### IDCU_LOG_DEBUG

输出调试信息。

**原型**：
```c
#define IDCU_LOG_DEBUG(fmt, ...)
```

**示例**：
```c
IDCU_LOG_DEBUG("Value: %d", 42);
```

---

### IDCU_LOG_INFO

输出一般信息。

**原型**：
```c
#define IDCU_LOG_INFO(fmt, ...)
```

**示例**：
```c
IDCU_LOG_INFO("Application started");
```

---

### IDCU_LOG_WARN

输出警告信息。

**原型**：
```c
#define IDCU_LOG_WARN(fmt, ...)
```

**示例**：
```c
IDCU_LOG_WARN("Memory usage: %d MB", 90);
```

---

### IDCU_LOG_ERROR

输出错误信息。

**原型**：
```c
#define IDCU_LOG_ERROR(fmt, ...)
```

**示例**：
```c
IDCU_LOG_ERROR("Failed to open file: %s", filename);
```

---

### IDCU_LOG_FATAL

输出致命错误（会终止程序）。

**原型**：
```c
#define IDCU_LOG_FATAL(fmt, ...)
```

**示例**：
```c
IDCU_LOG_FATAL("Critical error, exiting");
```

## 使用示例

### 基本使用

```c
#include <idcu/log/log.h>

int main() {
    idcu_log_init(NULL, IDCU_LOG_DEBUG);

    IDCU_LOG_DEBUG("Debug message");
    IDCU_LOG_INFO("Info message");
    IDCU_LOG_WARN("Warning message");
    IDCU_LOG_ERROR("Error message");

    idcu_log_shutdown();
    return 0;
}
```

### 文件日志

```c
#include <idcu/log/log.h>

int main() {
    idcu_log_init("app.log", IDCU_LOG_INFO);

    IDCU_LOG_INFO("This will be written to app.log");

    idcu_log_shutdown();
    return 0;
}
```

## 日志格式

每条日志包含以下信息：

```
[时间戳] [级别] [文件名:行号] 消息内容
```

例如：

```
[2026-04-05 10:30:00] [INFO] [main.c:10] Application started
```

## 依赖

- idcu-common
