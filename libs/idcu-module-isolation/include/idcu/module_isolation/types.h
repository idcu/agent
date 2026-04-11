#ifndef IDCU_MODULE_ISOLATION_TYPES_H
#define IDCU_MODULE_ISOLATION_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_MODULE_ISOLATION_MAX_ITEMS 1024

typedef enum {
    IDCU_ISOLATION_LEVEL_NONE,
    IDCU_ISOLATION_LEVEL_PROCESS,
    IDCU_ISOLATION_LEVEL_THREAD,
    IDCU_ISOLATION_LEVEL_MEMORY,
    IDCU_ISOLATION_LEVEL_CGROUP,
    IDCU_ISOLATION_LEVEL_CONTAINER
} idcu_IsolationLevel;

typedef struct idcu_ModuleIsolation_Context {
    int initialized;
    idcu_Mutex lock;
    uint64_t operation_count;
    uint64_t error_count;
} idcu_ModuleIsolation_Context;

#ifdef __cplusplus
}
#endif

#endif
