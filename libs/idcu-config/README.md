# idcu-config

Config library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/config/config.h>

idcu_Config_Context* ctx;
int ret = idcu_config_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_config_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
