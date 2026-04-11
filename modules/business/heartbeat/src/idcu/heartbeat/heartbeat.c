#include <idcu/heartbeat/heartbeat.h>
#include <string.h>
#include <time.h>

int idcu_heartbeat_module_init(idcu_HeartbeatModule* hb, const char* node_id, const char* node_name) {
    if (!hb || !node_id || !node_name) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(hb, 0, sizeof(idcu_HeartbeatModule));
    strncpy(hb->node.node_id, node_id, sizeof(hb->node.node_id) - 1);
    strncpy(hb->node.node_name, node_name, sizeof(hb->node.node_name) - 1);
    hb->node.status = IDCU_HEARTBEAT_STATUS_OFFLINE;
    hb->node.interval_ms = 5000;
    hb->node.timeout_ms = 15000;
    hb->initialized = 0;

    return IDCU_ERR_OK;
}

int idcu_heartbeat_module_start(idcu_HeartbeatModule* hb) {
    if (!hb) {
        return IDCU_ERR_INVALID_ARG;
    }

    hb->node.status = IDCU_HEARTBEAT_STATUS_ALIVE;
    hb->node.last_heartbeat_at = (uint64_t)time(NULL);
    hb->initialized = 1;

    return IDCU_ERR_OK;
}

int idcu_heartbeat_module_stop(idcu_HeartbeatModule* hb) {
    if (!hb) {
        return IDCU_ERR_INVALID_ARG;
    }

    hb->node.status = IDCU_HEARTBEAT_STATUS_OFFLINE;
    hb->initialized = 0;

    return IDCU_ERR_OK;
}

void idcu_heartbeat_module_destroy(idcu_HeartbeatModule* hb) {
    if (!hb) {
        return;
    }

    if (hb->initialized) {
        idcu_heartbeat_module_stop(hb);
    }

    memset(hb, 0, sizeof(idcu_HeartbeatModule));
}

int idcu_heartbeat_module_send(idcu_HeartbeatModule* hb) {
    if (!hb || !hb->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }

    hb->node.last_heartbeat_at = (uint64_t)time(NULL);
    hb->node.status = IDCU_HEARTBEAT_STATUS_ALIVE;

    return IDCU_ERR_OK;
}

idcu_HeartbeatStatus idcu_heartbeat_module_get_status(idcu_HeartbeatModule* hb) {
    if (!hb) {
        return IDCU_HEARTBEAT_STATUS_UNKNOWN;
    }
    return hb->node.status;
}
