#ifndef IDCU_MICROKERNEL_TYPES_H
#define IDCU_MICROKERNEL_TYPES_H

#include <idcu/common/config.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_KERNEL_STOPPED = 0,
    IDCU_KERNEL_STARTING = 1,
    IDCU_KERNEL_RUNNING = 2,
    IDCU_KERNEL_STOPPING = 3
} idcu_KernelState;

typedef struct {
    const char* config_path;
    bool enable_logging;
    int log_level;
    size_t max_modules;
} idcu_KernelConfig;

typedef struct idcu_MicroKernel idcu_MicroKernel;

#ifdef __cplusplus
}
#endif

#endif
