# idcu-log API 文档

日志系统库。

## 快速开始

```c
#include <idcu/log/log.h>

int main() {
    // 初始化日志到控制台，级别为 INFO
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    IDCU_LOG_INFO("Hello, world!");
    IDCU_LOG_WARN("警告消息");
    IDCU_LOG_ERROR("发生错误");
    
    idcu_log_shutdown();
    return 0;
}
```

## 日志级别

```c
typedef enum {
    IDCU_LOG_DEBUG = 0,
    IDCU_LOG_INFO = 1,
    IDCU_LOG_WARN = 2,
    IDCU_LOG_ERROR = 3,
    IDCU_LOG_FATAL = 4
} idcu_LogLevel;
```

## 日志输出

```c
typedef enum {
    IDCU_LOG_OUTPUT_CONSOLE = 1,
    IDCU_LOG_OUTPUT_FILE = 2
} idcu_LogOutput;
```

## 配置

### 日志配置结构

```c
typedef struct {
    idcu_LogLevel level;
    idcu_LogOutput output;
    const char* file_path;
    int max_file_size;
    int max_backup_files;
    int color_enabled;
} idcu_LogConfig;
```

## 初始化

### 简单初始化

```c
idcu_ErrorCode idcu_log_init(const char* file_path, idcu_LogLevel level);
```

初始化日志系统。

- `file_path`: 日志文件路径（NULL 表示仅控制台输出）
- `level`: 输出的最低日志级别

### 配置初始化

```c
idcu_ErrorCode idcu_log_init_with_config(const idcu_LogConfig* config);
```

使用完整配置初始化。

### 关闭

```c
void idcu_log_shutdown(void);
```

关闭日志系统。

## 日志宏

### 调试

```c
IDCU_LOG_DEBUG(format, ...)
```

记录一条调试消息。

### 信息

```c
IDCU_LOG_INFO(format, ...)
```

记录一条信息消息。

### 警告

```c
IDCU_LOG_WARN(format, ...)
```

记录一条警告消息。

### 错误

```c
IDCU_LOG_ERROR(format, ...)
```

记录一条错误消息。

### 致命

```c
IDCU_LOG_FATAL(format, ...)
```

记录一条致命消息。

## 动态级别控制

### 设置日志级别

```c
void idcu_log_set_level(idcu_LogLevel level);
```

在运行时更改最低日志级别。

## 示例

### 仅控制台

```c
idcu_log_init(NULL, IDCU_LOG_DEBUG);
IDCU_LOG_DEBUG("调试消息");
IDCU_LOG_INFO("信息消息");
idcu_log_shutdown();
```

### 文件输出

```c
idcu_log_init("app.log", IDCU_LOG_INFO);
IDCU_LOG_INFO("应用程序已启动");
idcu_log_shutdown();
```

### 自定义配置

```c
idcu_LogConfig config = {
    .level = IDCU_LOG_DEBUG,
    .output = IDCU_LOG_OUTPUT_CONSOLE | IDCU_LOG_OUTPUT_FILE,
    .file_path = "app.log",
    .max_file_size = 10 * 1024 * 1024,
    .max_backup_files = 5,
    .color_enabled = 1
};

idcu_log_init_with_config(&config);
// ...
idcu_log_shutdown();
```

### 动态级别更改

```c
idcu_log_init(NULL, IDCU_LOG_INFO);
IDCU_LOG_INFO("当前级别: INFO");
IDCU_LOG_DEBUG("这不会被记录");

idcu_log_set_level(IDCU_LOG_DEBUG);
IDCU_LOG_DEBUG("现在这会被记录");

idcu_log_shutdown();
```
