# idcu-plugin

Plugin library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/plugin/plugin.h>

idcu_Plugin_Context* ctx;
int ret = idcu_plugin_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_plugin_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
