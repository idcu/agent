#ifndef IDCU_SERVER_MONITOR_SERVER_MONITOR_H
#define IDCU_SERVER_MONITOR_SERVER_MONITOR_H

#include "idcu/common/error_code.h"
#include "idcu/server_monitor/types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_server_monitor_init(idcu_ServerMonitor_Context** ctx);
int idcu_server_monitor_start(idcu_ServerMonitor_Context* ctx);
void idcu_server_monitor_stop(idcu_ServerMonitor_Context* ctx);
void idcu_server_monitor_destroy(idcu_ServerMonitor_Context* ctx);

int idcu_server_monitor_collect(idcu_ServerMonitor_Context* ctx);
int idcu_server_monitor_get_stats(idcu_ServerMonitor_Context* ctx, idcu_ServerStats* stats);
int idcu_server_monitor_get_cpu_usage(idcu_ServerMonitor_Context* ctx, double* usage);
int idcu_server_monitor_get_memory_info(idcu_ServerMonitor_Context* ctx, uint64_t* total, uint64_t* used);
int idcu_server_monitor_get_disk_info(idcu_ServerMonitor_Context* ctx, uint64_t* total, uint64_t* used);

#ifdef __cplusplus
}
#endif

#endif
