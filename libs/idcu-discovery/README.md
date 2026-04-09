# idcu-discovery

Discovery library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/discovery/discovery.h>

idcu_Discovery_Context* ctx;
int ret = idcu_discovery_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_discovery_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
