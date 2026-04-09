# idcu-http-client

Http-Client library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/http-client/http-client.h>

idcu_Http-Client_Context* ctx;
int ret = idcu_http-client_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_http-client_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
