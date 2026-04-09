# idcu-watchdog

Watchdog library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/watchdog/watchdog.h>

idcu_Watchdog_Context* ctx;
int ret = idcu_watchdog_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_watchdog_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
