#ifndef IDCU_MANAGEMENT_TYPES_H
#define IDCU_MANAGEMENT_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_MANAGEMENT_MAX_ITEMS 1024

typedef enum {
    IDCU_MGMT_CMD_STATUS,
    IDCU_MGMT_CMD_MODULE_LIST,
    IDCU_MGMT_CMD_MODULE_START,
    IDCU_MGMT_CMD_MODULE_STOP,
    IDCU_MGMT_CMD_CONFIG_GET,
    IDCU_MGMT_CMD_CONFIG_SET,
    IDCU_MGMT_CMD_METRICS_GET,
    IDCU_MGMT_CMD_HEALTH_CHECK
} idcu_MgmtCommand;

typedef struct {
    int code;
    char* message;
    char* data;
    size_t data_size;
} idcu_MgmtResponse;

typedef struct idcu_Management_Context {
    int initialized;
    idcu_Mutex lock;
    uint64_t operation_count;
    uint64_t error_count;
} idcu_Management_Context;

#ifdef __cplusplus
}
#endif

#endif
