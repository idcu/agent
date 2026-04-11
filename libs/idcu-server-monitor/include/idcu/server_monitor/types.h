#ifndef IDCU_SERVER_MONITOR_TYPES_H
#define IDCU_SERVER_MONITOR_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_SERVER_MONITOR_MAX_ITEMS 1024

typedef struct idcu_ServerStats {
    double cpu_usage;
    uint64_t memory_total;
    uint64_t memory_used;
    uint64_t disk_total;
    uint64_t disk_used;
    uint64_t network_rx;
    uint64_t network_tx;
    uint32_t process_count;
    uint32_t thread_count;
    uint64_t uptime;
} idcu_ServerStats;

typedef struct idcu_ServerMonitor_Context {
    int initialized;
    idcu_Mutex lock;
    uint64_t operation_count;
    uint64_t error_count;
} idcu_ServerMonitor_Context;

#ifdef __cplusplus
}
#endif

#endif
