#ifndef IDCU_NETWORK_TYPES_H
#define IDCU_NETWORK_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_NETWORK_MAX_ITEMS 1024

typedef struct idcu_Network_Context {
    int initialized;
    idcu_Mutex lock;
    uint64_t operation_count;
    uint64_t error_count;
} idcu_Network_Context;

#ifdef __cplusplus
}
#endif

#endif
