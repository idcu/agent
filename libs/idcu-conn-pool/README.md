# idcu-conn-pool

Conn-Pool library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/conn-pool/conn-pool.h>

idcu_Conn-Pool_Context* ctx;
int ret = idcu_conn-pool_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_conn-pool_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
