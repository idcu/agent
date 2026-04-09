# idcu-http-server

Http-Server library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/http-server/http-server.h>

idcu_Http-Server_Context* ctx;
int ret = idcu_http-server_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_http-server_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
