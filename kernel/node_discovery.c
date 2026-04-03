#include "node_discovery.h"
#include <string.h>

int node_discovery_init(NodeDiscovery* disc, DistributedNode* dist_node) {
    if (!disc || !dist_node) return ERR_INVALID_PARAM;
    
    memset(disc, 0, sizeof(NodeDiscovery));
    disc->dist_node = dist_node;
    disc->discovered_handler = NULL;
    disc->discovered_user_data = NULL;
    disc->lost_handler = NULL;
    disc->lost_user_data = NULL;
    
    return ERR_OK;
}

void node_discovery_destroy(NodeDiscovery* disc) {
    if (!disc) return;
    memset(disc, 0, sizeof(NodeDiscovery));
}

int node_discovery_set_discovered_handler(NodeDiscovery* disc, NodeDiscoveredHandler handler, void* user_data) {
    if (!disc) return ERR_INVALID_PARAM;
    
    disc->discovered_handler = handler;
    disc->discovered_user_data = user_data;
    
    return ERR_OK;
}

int node_discovery_set_lost_handler(NodeDiscovery* disc, NodeLostHandler handler, void* user_data) {
    if (!disc) return ERR_INVALID_PARAM;
    
    disc->lost_handler = handler;
    disc->lost_user_data = user_data;
    
    return ERR_OK;
}
