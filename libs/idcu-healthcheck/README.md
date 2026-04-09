# idcu-healthcheck

Healthcheck library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/healthcheck/healthcheck.h>

idcu_Healthcheck_Context* ctx;
int ret = idcu_healthcheck_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_healthcheck_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
