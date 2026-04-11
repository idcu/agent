#ifndef IDCU_DEVICE_COLLECTOR_DEVICE_COLLECTOR_H
#define IDCU_DEVICE_COLLECTOR_DEVICE_COLLECTOR_H

#include "idcu/common/error_code.h"
#include "idcu/device_collector/types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_device_collector_init(idcu_DeviceCollector_Context** ctx);
int idcu_device_collector_start(idcu_DeviceCollector_Context* ctx);
void idcu_device_collector_stop(idcu_DeviceCollector_Context* ctx);
void idcu_device_collector_destroy(idcu_DeviceCollector_Context* ctx);

int idcu_device_collector_scan(idcu_DeviceCollector_Context* ctx);
int idcu_device_collector_get_device_count(idcu_DeviceCollector_Context* ctx, size_t* count);
int idcu_device_collector_get_device_info(idcu_DeviceCollector_Context* ctx, size_t index, idcu_DeviceInfo* info);
int idcu_device_collector_get_device_by_type(idcu_DeviceCollector_Context* ctx, idcu_DeviceType type, idcu_DeviceInfo* info_list, size_t max_count, size_t* actual_count);

#ifdef __cplusplus
}
#endif

#endif
