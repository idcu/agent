# idcu-sandbox

Sandbox library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/sandbox/sandbox.h>

idcu_Sandbox_Context* ctx;
int ret = idcu_sandbox_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_sandbox_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
