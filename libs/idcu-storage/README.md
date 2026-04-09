# idcu-storage

Storage library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/storage/storage.h>

idcu_Storage_Context* ctx;
int ret = idcu_storage_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_storage_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
