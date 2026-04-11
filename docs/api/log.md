# idcu-log API Documentation

Logging system library.

## Quick Start

```c
#include <idcu/log/log.h>

int main() {
    // Initialize logging to console with INFO level
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    IDCU_LOG_INFO("Hello, world!");
    IDCU_LOG_WARN("Warning message");
    IDCU_LOG_ERROR("Error occurred");
    
    idcu_log_shutdown();
    return 0;
}
```

## Log Levels

```c
typedef enum {
    IDCU_LOG_DEBUG = 0,
    IDCU_LOG_INFO = 1,
    IDCU_LOG_WARN = 2,
    IDCU_LOG_ERROR = 3,
    IDCU_LOG_FATAL = 4
} idcu_LogLevel;
```

## Log Output

```c
typedef enum {
    IDCU_LOG_OUTPUT_CONSOLE = 1,
    IDCU_LOG_OUTPUT_FILE = 2
} idcu_LogOutput;
```

## Configuration

### Log Configuration Structure

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

## Initialization

### Simple Initialization

```c
idcu_ErrorCode idcu_log_init(const char* file_path, idcu_LogLevel level);
```

Initialize the logging system.

- `file_path`: Path to log file (NULL for console only)
- `level`: Minimum log level to output

### Config Initialization

```c
idcu_ErrorCode idcu_log_init_with_config(const idcu_LogConfig* config);
```

Initialize with full configuration.

### Shutdown

```c
void idcu_log_shutdown(void);
```

Shutdown the logging system.

## Log Macros

### Debug

```c
IDCU_LOG_DEBUG(format, ...)
```

Log a debug message.

### Info

```c
IDCU_LOG_INFO(format, ...)
```

Log an info message.

### Warn

```c
IDCU_LOG_WARN(format, ...)
```

Log a warning message.

### Error

```c
IDCU_LOG_ERROR(format, ...)
```

Log an error message.

### Fatal

```c
IDCU_LOG_FATAL(format, ...)
```

Log a fatal message.

## Dynamic Level Control

### Set Log Level

```c
void idcu_log_set_level(idcu_LogLevel level);
```

Change the minimum log level at runtime.

## Examples

### Console Only

```c
idcu_log_init(NULL, IDCU_LOG_DEBUG);
IDCU_LOG_DEBUG("Debug message");
IDCU_LOG_INFO("Info message");
idcu_log_shutdown();
```

### File Output

```c
idcu_log_init("app.log", IDCU_LOG_INFO);
IDCU_LOG_INFO("Application started");
idcu_log_shutdown();
```

### Custom Configuration

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

### Dynamic Level Change

```c
idcu_log_init(NULL, IDCU_LOG_INFO);
IDCU_LOG_INFO("Current level: INFO");
IDCU_LOG_DEBUG("This won't be logged");

idcu_log_set_level(IDCU_LOG_DEBUG);
IDCU_LOG_DEBUG("Now this will be logged");

idcu_log_shutdown();
```
