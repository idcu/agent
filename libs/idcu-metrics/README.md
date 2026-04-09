# idcu-metrics

Metrics library for IDCU Agent.

## Features

- Basic initialization and cleanup
- [To be implemented]

## Usage

`c
#include <idcu/metrics/metrics.h>

idcu_Metrics_Context* ctx;
int ret = idcu_metrics_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Use the library
    idcu_metrics_destroy(ctx);
}
`

## Build

`ash
cmake -B build && cmake --build build
`

## License

MIT
