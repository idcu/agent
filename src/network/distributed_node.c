#include "network/distributed_node.h"
#include <string.h>
#include <time.h>

int idcu_distributed_node_init(idcu_DistributedNode* node, uint64_t node_id, const char* name, const char* address, uint16_t port) {
    if (!node || !name || !address) return IDCU_ERR_INVALID_PARAM;
    
    memset(node, 0, sizeof(idcu_DistributedNode));
    node->self_node_id = node_id;
    node->node_count = 1;
    node->nodes[0].node_id = node_id;
    strncpy(node->nodes[0].name, name, IDCU_NODE_NAME_MAX - 1);
    node->nodes[0].name[IDCU_NODE_NAME_MAX - 1] = '\0';
    strncpy(node->nodes[0].address, address, IDCU_NODE_ADDR_MAX - 1);
    node->nodes[0].address[IDCU_NODE_ADDR_MAX - 1] = '\0';
    node->nodes[0].port = port;
    node->nodes[0].status = IDCU_NODE_STATUS_ONLINE;
    node->nodes[0].load = 0;
    node->nodes[0].last_heartbeat = (uint64_t)time(NULL);
    node->msg_count = 0;
    node->msg_head = 0;
    node->msg_tail = 0;
    
    return IDCU_ERR_OK;
}

void idcu_distributed_node_destroy(idcu_DistributedNode* node) {
    if (!node) return;
    memset(node, 0, sizeof(idcu_DistributedNode));
}

int idcu_distributed_node_add_node(idcu_DistributedNode* node, uint64_t node_id, const char* name, const char* address, uint16_t port) {
    if (!node || !name || !address) return IDCU_ERR_INVALID_PARAM;
    if (node->node_count >= IDCU_MAX_NODES) return IDCU_ERR_NO_MEMORY;
    if (idcu_distributed_node_find_node(node, node_id)) return IDCU_ERR_ALREADY_EXISTS;
    
    node->nodes[node->node_count].node_id = node_id;
    strncpy(node->nodes[node->node_count].name, name, IDCU_NODE_NAME_MAX - 1);
    node->nodes[node->node_count].name[IDCU_NODE_NAME_MAX - 1] = '\0';
    strncpy(node->nodes[node->node_count].address, address, IDCU_NODE_ADDR_MAX - 1);
    node->nodes[node->node_count].address[IDCU_NODE_ADDR_MAX - 1] = '\0';
    node->nodes[node->node_count].port = port;
    node->nodes[node->node_count].status = IDCU_NODE_STATUS_ONLINE;
    node->nodes[node->node_count].load = 0;
    node->nodes[node->node_count].last_heartbeat = (uint64_t)time(NULL);
    node->node_count++;
    
    return IDCU_ERR_OK;
}

int idcu_distributed_node_remove_node(idcu_DistributedNode* node, uint64_t node_id) {
    if (!node) return IDCU_ERR_INVALID_PARAM;
    if (node_id == node->self_node_id) return IDCU_ERR_INVALID_PARAM;
    
    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].node_id == node_id) {
            for (int j = i; j < node->node_count - 1; j++) {
                node->nodes[j] = node->nodes[j + 1];
            }
            node->node_count--;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NOT_FOUND;
}

idcu_NodeInfo* idcu_distributed_node_find_node(idcu_DistributedNode* node, uint64_t node_id) {
    if (!node) return NULL;
    
    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].node_id == node_id) {
            return &node->nodes[i];
        }
    }
    
    return NULL;
}

int idcu_distributed_node_update_node_status(idcu_DistributedNode* node, uint64_t node_id, idcu_NodeStatus status) {
    if (!node) return IDCU_ERR_INVALID_PARAM;
    
    idcu_NodeInfo* info = idcu_distributed_node_find_node(node, node_id);
    if (!info) return IDCU_ERR_NOT_FOUND;
    
    info->status = status;
    return IDCU_ERR_OK;
}

int idcu_distributed_node_update_heartbeat(idcu_DistributedNode* node, uint64_t node_id) {
    if (!node) return IDCU_ERR_INVALID_PARAM;
    
    idcu_NodeInfo* info = idcu_distributed_node_find_node(node, node_id);
    if (!info) return IDCU_ERR_NOT_FOUND;
    
    info->last_heartbeat = (uint64_t)time(NULL);
    return IDCU_ERR_OK;
}

int idcu_distributed_node_send_message(idcu_DistributedNode* node, uint64_t to_node, uint32_t type, const void* payload, size_t payload_size) {
    if (!node) return IDCU_ERR_INVALID_PARAM;
    if (node->msg_count >= IDCU_MAX_MESSAGES) return IDCU_ERR_QUEUE_FULL;
    if (payload && payload_size > IDCU_MAX_MESSAGE_PAYLOAD_SIZE) return IDCU_ERR_INVALID_PARAM;
    
    node->messages[node->msg_tail].from_node = node->self_node_id;
    node->messages[node->msg_tail].to_node = to_node;
    node->messages[node->msg_tail].type = type;
    if (payload && payload_size > 0) {
        memcpy(node->messages[node->msg_tail].payload, payload, payload_size);
        node->messages[node->msg_tail].payload_size = payload_size;
    } else {
        node->messages[node->msg_tail].payload_size = 0;
    }
    
    node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
    node->msg_count++;
    
    return IDCU_ERR_OK;
}

int idcu_distributed_node_recv_message(idcu_DistributedNode* node, idcu_NodeMessage* msg) {
    if (!node || !msg) return IDCU_ERR_INVALID_PARAM;
    if (node->msg_count == 0) return IDCU_ERR_QUEUE_EMPTY;
    
    *msg = node->messages[node->msg_head];
    node->msg_head = (node->msg_head + 1) % IDCU_MAX_MESSAGES;
    node->msg_count--;
    
    return IDCU_ERR_OK;
}

uint64_t idcu_distributed_node_select_node_by_load(idcu_DistributedNode* node) {
    if (!node || node->node_count == 0) return 0;
    
    uint64_t selected = node->self_node_id;
    uint64_t min_load = (uint64_t)-1;
    
    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].status == IDCU_NODE_STATUS_ONLINE) {
            if (node->nodes[i].load < min_load) {
                min_load = node->nodes[i].load;
                selected = node->nodes[i].node_id;
            }
        }
    }
    
    return selected;
}
