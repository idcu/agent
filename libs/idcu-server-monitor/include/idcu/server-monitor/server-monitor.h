#ifndef IDCU_SERVER-MONITOR_SERVER-MONITOR_H
#define IDCU_SERVER-MONITOR_SERVER-MONITOR_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/server-monitor/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_server-monitor_init(idcu_Server-Monitor_Context** ctx);
void idcu_server-monitor_destroy(idcu_Server-Monitor_Context* ctx);
int idcu_server-monitor_is_initialized(idcu_Server-Monitor_Context* ctx);
uint64_t idcu_server-monitor_get_operation_count(idcu_Server-Monitor_Context* ctx);
uint64_t idcu_server-monitor_get_error_count(idcu_Server-Monitor_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
