#ifndef IDCU_SERVER-MONITOR_SERVER-MONITOR_H
#define IDCU_SERVER-MONITOR_SERVER-MONITOR_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/server-monitor/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_server-monitor_init(idcu_Server-Monitor_Context** ctx);
void idcu_server-monitor_destroy(idcu_Server-Monitor_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
