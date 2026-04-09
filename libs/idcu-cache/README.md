# idcu-cache

Cache library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/cache/cache.h>

idcu_Cache_Context* ctx;
int ret = idcu_cache_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_cache_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
