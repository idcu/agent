# idcu-server-monitor

Server-Monitor library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/server-monitor/server-monitor.h>

idcu_Server-Monitor_Context* ctx;
int ret = idcu_server-monitor_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_server-monitor_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
