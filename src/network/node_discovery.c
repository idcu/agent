#include "network/node_discovery.h"
#include <string.h>

int idcu_node_discovery_init(idcu_NodeDiscovery* disc, idcu_DistributedNode* dist_node) {
    if (!disc || !dist_node) return IDCU_ERR_INVALID_PARAM;
    
    memset(disc, 0, sizeof(idcu_NodeDiscovery));
    disc->dist_node = dist_node;
    disc->discovered_handler = NULL;
    disc->discovered_user_data = NULL;
    disc->lost_handler = NULL;
    disc->lost_user_data = NULL;
    
    return IDCU_ERR_OK;
}

void idcu_node_discovery_destroy(idcu_NodeDiscovery* disc) {
    if (!disc) return;
    memset(disc, 0, sizeof(idcu_NodeDiscovery));
}

int idcu_node_discovery_set_discovered_handler(idcu_NodeDiscovery* disc, idcu_NodeDiscoveredHandler handler, void* user_data) {
    if (!disc) return IDCU_ERR_INVALID_PARAM;
    
    disc->discovered_handler = handler;
    disc->discovered_user_data = user_data;
    
    return IDCU_ERR_OK;
}

int idcu_node_discovery_set_lost_handler(idcu_NodeDiscovery* disc, idcu_NodeLostHandler handler, void* user_data) {
    if (!disc) return IDCU_ERR_INVALID_PARAM;
    
    disc->lost_handler = handler;
    disc->lost_user_data = user_data;
    
    return IDCU_ERR_OK;
}
