# idcu-permission

Permission library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/permission/permission.h>

idcu_Permission_Context* ctx;
int ret = idcu_permission_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_permission_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
