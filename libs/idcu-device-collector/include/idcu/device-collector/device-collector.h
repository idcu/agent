#ifndef IDCU_DEVICE-COLLECTOR_DEVICE-COLLECTOR_H
#define IDCU_DEVICE-COLLECTOR_DEVICE-COLLECTOR_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/device-collector/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_device-collector_init(idcu_Device-Collector_Context** ctx);
void idcu_device-collector_destroy(idcu_Device-Collector_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
