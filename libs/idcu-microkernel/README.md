# idcu-microkernel

Microkernel core library for IDCU Agent, integrating all core components including module system, coroutine scheduler, and message bus.

## Features

- Kernel lifecycle management (init, start, run, stop, destroy)
- Integration of module system, coroutine scheduler, and message bus
- Main event loop
- Thread-safe operations

## Usage

```c
#include <idcu/microkernel/kernel.h>

int main() {
    idcu_KernelConfig config = {
        .config_path = "config.yaml",
        .enable_logging = true,
        .log_level = 1,
        .max_modules = 32
    };

    idcu_MicroKernel* kernel = NULL;
    idcu_kernel_init(&kernel, &config);
    idcu_kernel_start(kernel);
    idcu_kernel_run(kernel);
    idcu_kernel_stop(kernel);
    idcu_kernel_destroy(kernel);

    return 0;
}
```

## License

MIT
