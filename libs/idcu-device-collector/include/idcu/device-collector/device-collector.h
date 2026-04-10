#ifndef IDCU_DEVICE-COLLECTOR_DEVICE-COLLECTOR_H
#define IDCU_DEVICE-COLLECTOR_DEVICE-COLLECTOR_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/device-collector/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_device-collector_init(idcu_Device-Collector_Context** ctx);
void idcu_device-collector_destroy(idcu_Device-Collector_Context* ctx);
int idcu_device-collector_is_initialized(idcu_Device-Collector_Context* ctx);
uint64_t idcu_device-collector_get_operation_count(idcu_Device-Collector_Context* ctx);
uint64_t idcu_device-collector_get_error_count(idcu_Device-Collector_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
