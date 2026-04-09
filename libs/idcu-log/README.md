# idcu-log

Logging library for IDCU Agent.

## Features

- Multi-level logging (DEBUG, INFO, WARN, ERROR, FATAL)
- Console and file output
- Thread-safe logging with mutex
- Timestamp, file, and line number information
- Colorized console output

## Usage

```c
#include <idcu/log/log.h>

int main() {
    idcu_log_init("app.log", IDCU_LOG_INFO);
    
    IDCU_LOG_INFO("Application started");
    IDCU_LOG_WARN("Low memory warning");
    IDCU_LOG_ERROR("Failed to open file: %s", "data.txt");
    
    idcu_log_shutdown();
    return 0;
}
```

## Build

```bash
cmake -B build
cmake --build build
```
