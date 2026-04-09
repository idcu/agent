# idcu-distributed

Distributed library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/distributed/distributed.h>

idcu_Distributed_Context* ctx;
int ret = idcu_distributed_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_distributed_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
