# idcu-device-collector

Device-Collector library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/device-collector/device-collector.h>

idcu_Device-Collector_Context* ctx;
int ret = idcu_device-collector_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_device-collector_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
