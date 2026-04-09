# idcu-utils

Utils library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/utils/utils.h>

idcu_Utils_Context* ctx;
int ret = idcu_utils_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_utils_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
