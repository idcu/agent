# idcu-microkernel API Documentation

Microkernel for managing modules and the application lifecycle.

## Microkernel

```c
typedef struct idcu_MicroKernel idcu_MicroKernel;
```

## Kernel Configuration

```c
typedef struct {
    const char* config_path;
    int enable_logging;
    int log_level;
    size_t max_modules;
} idcu_KernelConfig;
```

### Kernel Functions

```c
int idcu_kernel_init(idcu_MicroKernel** out_kernel, const idcu_KernelConfig* config);
void idcu_kernel_destroy(idcu_MicroKernel* kernel);

int idcu_kernel_start(idcu_MicroKernel* kernel);
int idcu_kernel_stop(idcu_MicroKernel* kernel);
int idcu_kernel_run(idcu_MicroKernel* kernel);

idcu_KernelState idcu_kernel_get_state(const idcu_MicroKernel* kernel);
```

## Kernel States

```c
typedef enum {
    IDCU_KERNEL_STOPPED,
    IDCU_KERNEL_STARTING,
    IDCU_KERNEL_RUNNING,
    IDCU_KERNEL_STOPPING,
    IDCU_KERNEL_ERROR
} idcu_KernelState;
```

## Module Management

```c
int idcu_kernel_load_module(idcu_MicroKernel* kernel, const char* module_path);
int idcu_kernel_unload_module(idcu_MicroKernel* kernel, const char* module_name);

int idcu_kernel_get_module_count(idcu_MicroKernel* kernel);
int idcu_kernel_get_module_info(idcu_MicroKernel* kernel, size_t index, idcu_ModuleInfo* out_info);
```

## Configuration

```c
int idcu_kernel_reload_config(idcu_MicroKernel* kernel);
int idcu_kernel_get_config(idcu_MicroKernel* kernel, idcu_KernelConfig* out_config);
```

## Example

```c
#include <idcu/microkernel/kernel.h>
#include <stdio.h>

int main(void) {
    idcu_MicroKernel* kernel = NULL;
    
    idcu_KernelConfig config = {
        .config_path = NULL,
        .enable_logging = 1,
        .log_level = IDCU_LOG_INFO,
        .max_modules = 32
    };
    
    // Initialize the kernel
    int ret = idcu_kernel_init(&kernel, &config);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to initialize kernel\n");
        return 1;
    }
    
    printf("Kernel initialized, state: %d\n", idcu_kernel_get_state(kernel));
    
    // Start the kernel
    ret = idcu_kernel_start(kernel);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to start kernel\n");
        idcu_kernel_destroy(kernel);
        return 1;
    }
    
    printf("Kernel started\n");
    
    // Run the main loop
    ret = idcu_kernel_run(kernel);
    
    // Stop and cleanup
    idcu_kernel_stop(kernel);
    idcu_kernel_destroy(kernel);
    
    printf("Kernel shutdown complete\n");
    return 0;
}
```
