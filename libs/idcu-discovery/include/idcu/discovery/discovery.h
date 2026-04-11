#ifndef IDCU_DISCOVERY_DISCOVERY_H
#define IDCU_DISCOVERY_DISCOVERY_H

#include <idcu/common/error_code.h>
#include <idcu/common/vector.h>
#include <idcu/common/hash_map.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_NODE_STATUS_ONLINE = 0,
    IDCU_NODE_STATUS_OFFLINE,
    IDCU_NODE_STATUS_UNHEALTHY,
    IDCU_NODE_STATUS_UNKNOWN
} idcu_NodeStatus;

typedef struct
{
    char id[64];
    char name[128];
    char address[256];
    uint16_t port;
    idcu_NodeStatus status;
    uint64_t last_seen_ms;
    char metadata[1024];
    char tags[512];
} idcu_Node;

typedef struct
{
    idcu_Vector nodes;
    idcu_HashMap nodes_by_id;
    idcu_Mutex lock;
    int initialized;
    int running;
    uint64_t discovery_interval_ms;
    uint64_t node_timeout_ms;
} idcu_Discovery;

int  idcu_discovery_init(idcu_Discovery* discovery, uint64_t discovery_interval_ms, uint64_t node_timeout_ms);
void idcu_discovery_destroy(idcu_Discovery* discovery);

int  idcu_discovery_register_node(idcu_Discovery* discovery, const idcu_Node* node);
int  idcu_discovery_unregister_node(idcu_Discovery* discovery, const char* node_id);
idcu_Node* idcu_discovery_get_node(idcu_Discovery* discovery, const char* node_id);

int  idcu_discovery_heartbeat(idcu_Discovery* discovery, const char* node_id);
int  idcu_discovery_update_node_status(idcu_Discovery* discovery, const char* node_id, idcu_NodeStatus status);

int  idcu_discovery_refresh(idcu_Discovery* discovery);
int  idcu_discovery_get_nodes_by_status(idcu_Discovery* discovery, idcu_NodeStatus status, idcu_Vector* results);
int  idcu_discovery_get_all_nodes(idcu_Discovery* discovery, idcu_Vector* results);

int  idcu_discovery_find_nodes_by_tag(idcu_Discovery* discovery, const char* tag, idcu_Vector* results);

const char* idcu_node_status_to_string(idcu_NodeStatus status);

#ifdef __cplusplus
}
#endif

#endif
