#ifndef IDCU_HEARTBEAT_HEARTBEAT_H
#define IDCU_HEARTBEAT_HEARTBEAT_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_HEARTBEAT_STATUS_UNKNOWN = 0,
    IDCU_HEARTBEAT_STATUS_ALIVE,
    IDCU_HEARTBEAT_STATUS_WARNING,
    IDCU_HEARTBEAT_STATUS_DEAD,
    IDCU_HEARTBEAT_STATUS_OFFLINE
} idcu_HeartbeatStatus;

typedef struct {
    char node_id[128];
    char node_name[128];
    idcu_HeartbeatStatus status;
    uint64_t last_heartbeat_at;
    uint64_t interval_ms;
    uint64_t timeout_ms;
} idcu_HeartbeatNode;

typedef struct {
    idcu_HeartbeatNode node;
    int initialized;
    void* user_data;
} idcu_HeartbeatModule;

int idcu_heartbeat_module_init(idcu_HeartbeatModule* hb, const char* node_id, const char* node_name);
int idcu_heartbeat_module_start(idcu_HeartbeatModule* hb);
int idcu_heartbeat_module_stop(idcu_HeartbeatModule* hb);
void idcu_heartbeat_module_destroy(idcu_HeartbeatModule* hb);
int idcu_heartbeat_module_send(idcu_HeartbeatModule* hb);
idcu_HeartbeatStatus idcu_heartbeat_module_get_status(idcu_HeartbeatModule* hb);

#ifdef __cplusplus
}
#endif

#endif
