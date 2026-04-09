# idcu-memory

Memory library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/memory/memory.h>

idcu_Memory_Context* ctx;
int ret = idcu_memory_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_memory_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
