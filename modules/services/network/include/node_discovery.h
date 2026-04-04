#ifndef IDCU_NETWORK_NODE_DISCOVERY_H
#define IDCU_NETWORK_NODE_DISCOVERY_H

#include "distributed_node.h"
#include "network_layer.h"
#include "error_code.h"
#include <stdint.h>

#define IDCU_DISCOVERY_PORT 9999
#define IDCU_DISCOVERY_MULTICAST_ADDR "239.255.0.1"
#define IDCU_DISCOVERY_BROADCAST_INTERVAL_MS 3000
#define IDCU_DISCOVERY_TIMEOUT_MS 10000
#define IDCU_DISCOVERY_MAX_PACKET_SIZE 512

typedef void (*idcu_NodeDiscoveredHandler)(void* user_data, idcu_NodeInfo* node);
typedef void (*idcu_NodeLostHandler)(void* user_data, idcu_NodeInfo* node);

typedef struct {
    idcu_DistributedNode* dist_node;
    idcu_NodeDiscoveredHandler discovered_handler;
    void* discovered_user_data;
    idcu_NodeLostHandler lost_handler;
    void* lost_user_data;
    idcu_NetworkSocket socket;
    int running;
    uint64_t last_broadcast_time;
} idcu_NodeDiscovery;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint64_t node_id;
    char name[IDCU_NODE_NAME_MAX];
    char address[IDCU_NODE_ADDR_MAX];
    uint16_t port;
    uint64_t timestamp;
} idcu_DiscoveryPacket;

int idcu_node_discovery_init(idcu_NodeDiscovery* disc, idcu_DistributedNode* dist_node);
void idcu_node_discovery_destroy(idcu_NodeDiscovery* disc);
int idcu_node_discovery_set_discovered_handler(idcu_NodeDiscovery* disc, idcu_NodeDiscoveredHandler handler, void* user_data);
int idcu_node_discovery_set_lost_handler(idcu_NodeDiscovery* disc, idcu_NodeLostHandler handler, void* user_data);
int idcu_node_discovery_start(idcu_NodeDiscovery* disc);
int idcu_node_discovery_stop(idcu_NodeDiscovery* disc);
int idcu_node_discovery_poll(idcu_NodeDiscovery* disc);

#endif // IDCU_NETWORK_NODE_DISCOVERY_H
