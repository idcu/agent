#ifndef NODE_DISCOVERY_H
#define NODE_DISCOVERY_H

#include "network/distributed_node.h"
#include "common/error_code.h"

typedef void (*NodeDiscoveredHandler)(void* user_data, NodeInfo* node);
typedef void (*NodeLostHandler)(void* user_data, NodeInfo* node);

typedef struct {
    DistributedNode* dist_node;
    NodeDiscoveredHandler discovered_handler;
    void* discovered_user_data;
    NodeLostHandler lost_handler;
    void* lost_user_data;
} NodeDiscovery;

int node_discovery_init(NodeDiscovery* disc, DistributedNode* dist_node);
void node_discovery_destroy(NodeDiscovery* disc);
int node_discovery_set_discovered_handler(NodeDiscovery* disc, NodeDiscoveredHandler handler, void* user_data);
int node_discovery_set_lost_handler(NodeDiscovery* disc, NodeLostHandler handler, void* user_data);

#endif
