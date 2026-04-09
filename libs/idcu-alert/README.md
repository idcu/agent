# idcu-alert

Alert library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/alert/alert.h>

idcu_Alert_Context* ctx;
int ret = idcu_alert_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_alert_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
