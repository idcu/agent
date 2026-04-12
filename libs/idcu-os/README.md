# idcu-os - Cross-platform OS Abstraction Layer

A unified OS abstraction layer providing consistent APIs across multiple platforms.

## Supported Platforms

- Linux (P0)
- Windows (P0)
- macOS (P1)
- FreeBSD (P1)
- Android (P2)
- HarmonyOS (P2)
- Vector OS (P3)
- RT-Thread (P4)

## Features

- Thread synchronization (mutex, cond, rwlock)
- Coroutine support
- Time and sleep functions
- Network socket operations
- I/O multiplexing (poll/select)
- File I/O
- Path manipulation
- Environment variables
- Signal handling
- Backtrace capture
- Dynamic library loading
- System information

## Usage

Include the header:

```c
#include <idcu/os.h>
```

Link against the library:

```cmake
target_link_libraries(your_target idcu::os)
```
