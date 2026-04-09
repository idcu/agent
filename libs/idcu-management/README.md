# idcu-management

Management library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/management/management.h>

idcu_Management_Context* ctx;
int ret = idcu_management_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_management_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
