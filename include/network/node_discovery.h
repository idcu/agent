#ifndef IDCU_NETWORK_NODE_DISCOVERY_H
#define IDCU_NETWORK_NODE_DISCOVERY_H

#include "network/distributed_node.h"
#include "common/error_code.h"

typedef void (*idcu_NodeDiscoveredHandler)(void* user_data, idcu_NodeInfo* node);
typedef void (*idcu_NodeLostHandler)(void* user_data, idcu_NodeInfo* node);

typedef struct {
    idcu_DistributedNode* dist_node;
    idcu_NodeDiscoveredHandler discovered_handler;
    void* discovered_user_data;
    idcu_NodeLostHandler lost_handler;
    void* lost_user_data;
} idcu_NodeDiscovery;

int idcu_node_discovery_init(idcu_NodeDiscovery* disc, idcu_DistributedNode* dist_node);
void idcu_node_discovery_destroy(idcu_NodeDiscovery* disc);
int idcu_node_discovery_set_discovered_handler(idcu_NodeDiscovery* disc, idcu_NodeDiscoveredHandler handler, void* user_data);
int idcu_node_discovery_set_lost_handler(idcu_NodeDiscovery* disc, idcu_NodeLostHandler handler, void* user_data);

#endif // IDCU_NETWORK_NODE_DISCOVERY_H
