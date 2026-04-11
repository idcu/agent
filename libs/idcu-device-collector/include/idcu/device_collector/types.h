#ifndef IDCU_DEVICE_COLLECTOR_TYPES_H
#define IDCU_DEVICE_COLLECTOR_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_DEVICE_COLLECTOR_MAX_ITEMS 1024

typedef enum {
    IDCU_DEVICE_TYPE_UNKNOWN,
    IDCU_DEVICE_TYPE_CPU,
    IDCU_DEVICE_TYPE_MEMORY,
    IDCU_DEVICE_TYPE_DISK,
    IDCU_DEVICE_TYPE_NETWORK,
    IDCU_DEVICE_TYPE_GPU,
    IDCU_DEVICE_TYPE_SENSOR
} idcu_DeviceType;

typedef struct idcu_DeviceInfo {
    char name[256];
    char model[256];
    char serial[128];
    idcu_DeviceType type;
    uint64_t capacity;
    uint32_t status;
    uint32_t health_score;
} idcu_DeviceInfo;

typedef struct idcu_DeviceCollector_Context {
    int initialized;
    idcu_Mutex lock;
    uint64_t operation_count;
    uint64_t error_count;
} idcu_DeviceCollector_Context;

#ifdef __cplusplus
}
#endif

#endif
