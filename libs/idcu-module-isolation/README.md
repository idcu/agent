# idcu-module-isolation

Module-Isolation library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/module-isolation/module-isolation.h>

idcu_Module-Isolation_Context* ctx;
int ret = idcu_module-isolation_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_module-isolation_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
