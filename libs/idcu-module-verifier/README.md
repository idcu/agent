# idcu-module-verifier

Module-Verifier library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/module-verifier/module-verifier.h>

idcu_Module-Verifier_Context* ctx;
int ret = idcu_module-verifier_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_module-verifier_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
