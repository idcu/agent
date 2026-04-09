# idcu-scheduler

Scheduler library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/scheduler/scheduler.h>

idcu_Scheduler_Context* ctx;
int ret = idcu_scheduler_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_scheduler_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
