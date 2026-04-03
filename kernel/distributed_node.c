#include "distributed_node.h"
#include <string.h>
#include <time.h>

int distributed_node_init(DistributedNode* node, uint64_t node_id, const char* name, const char* address, uint16_t port) {
    if (!node || !name || !address) return ERR_INVALID_PARAM;
    
    memset(node, 0, sizeof(DistributedNode));
    node->self_node_id = node_id;
    node->node_count = 1;
    node->nodes[0].node_id = node_id;
    strncpy(node->nodes[0].name, name, NODE_NAME_MAX - 1);
    node->nodes[0].name[NODE_NAME_MAX - 1] = '\0';
    strncpy(node->nodes[0].address, address, NODE_ADDR_MAX - 1);
    node->nodes[0].address[NODE_ADDR_MAX - 1] = '\0';
    node->nodes[0].port = port;
    node->nodes[0].status = NODE_STATUS_ONLINE;
    node->nodes[0].load = 0;
    node->nodes[0].last_heartbeat = (uint64_t)time(NULL);
    node->msg_count = 0;
    node->msg_head = 0;
    node->msg_tail = 0;
    
    return ERR_OK;
}

void distributed_node_destroy(DistributedNode* node) {
    if (!node) return;
    memset(node, 0, sizeof(DistributedNode));
}

int distributed_node_add_node(DistributedNode* node, uint64_t node_id, const char* name, const char* address, uint16_t port) {
    if (!node || !name || !address) return ERR_INVALID_PARAM;
    if (node->node_count >= MAX_NODES) return ERR_NO_MEMORY;
    if (distributed_node_find_node(node, node_id)) return ERR_ALREADY_EXISTS;
    
    node->nodes[node->node_count].node_id = node_id;
    strncpy(node->nodes[node->node_count].name, name, NODE_NAME_MAX - 1);
    node->nodes[node->node_count].name[NODE_NAME_MAX - 1] = '\0';
    strncpy(node->nodes[node->node_count].address, address, NODE_ADDR_MAX - 1);
    node->nodes[node->node_count].address[NODE_ADDR_MAX - 1] = '\0';
    node->nodes[node->node_count].port = port;
    node->nodes[node->node_count].status = NODE_STATUS_ONLINE;
    node->nodes[node->node_count].load = 0;
    node->nodes[node->node_count].last_heartbeat = (uint64_t)time(NULL);
    node->node_count++;
    
    return ERR_OK;
}

int distributed_node_remove_node(DistributedNode* node, uint64_t node_id) {
    if (!node) return ERR_INVALID_PARAM;
    if (node_id == node->self_node_id) return ERR_INVALID_PARAM;
    
    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].node_id == node_id) {
            for (int j = i; j < node->node_count - 1; j++) {
                node->nodes[j] = node->nodes[j + 1];
            }
            node->node_count--;
            return ERR_OK;
        }
    }
    
    return ERR_NOT_FOUND;
}

NodeInfo* distributed_node_find_node(DistributedNode* node, uint64_t node_id) {
    if (!node) return NULL;
    
    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].node_id == node_id) {
            return &node->nodes[i];
        }
    }
    
    return NULL;
}

int distributed_node_update_node_status(DistributedNode* node, uint64_t node_id, NodeStatus status) {
    if (!node) return ERR_INVALID_PARAM;
    
    NodeInfo* info = distributed_node_find_node(node, node_id);
    if (!info) return ERR_NOT_FOUND;
    
    info->status = status;
    return ERR_OK;
}

int distributed_node_update_heartbeat(DistributedNode* node, uint64_t node_id) {
    if (!node) return ERR_INVALID_PARAM;
    
    NodeInfo* info = distributed_node_find_node(node, node_id);
    if (!info) return ERR_NOT_FOUND;
    
    info->last_heartbeat = (uint64_t)time(NULL);
    return ERR_OK;
}

int distributed_node_send_message(DistributedNode* node, uint64_t to_node, uint32_t type, const void* payload, size_t payload_size) {
    if (!node) return ERR_INVALID_PARAM;
    if (node->msg_count >= MAX_MESSAGES) return ERR_QUEUE_FULL;
    if (payload && payload_size > MAX_MESSAGE_PAYLOAD_SIZE) return ERR_INVALID_PARAM;
    
    node->messages[node->msg_tail].from_node = node->self_node_id;
    node->messages[node->msg_tail].to_node = to_node;
    node->messages[node->msg_tail].type = type;
    if (payload && payload_size > 0) {
        memcpy(node->messages[node->msg_tail].payload, payload, payload_size);
        node->messages[node->msg_tail].payload_size = payload_size;
    } else {
        node->messages[node->msg_tail].payload_size = 0;
    }
    
    node->msg_tail = (node->msg_tail + 1) % MAX_MESSAGES;
    node->msg_count++;
    
    return ERR_OK;
}

int distributed_node_recv_message(DistributedNode* node, NodeMessage* msg) {
    if (!node || !msg) return ERR_INVALID_PARAM;
    if (node->msg_count == 0) return ERR_QUEUE_EMPTY;
    
    *msg = node->messages[node->msg_head];
    node->msg_head = (node->msg_head + 1) % MAX_MESSAGES;
    node->msg_count--;
    
    return ERR_OK;
}

uint64_t distributed_node_select_node_by_load(DistributedNode* node) {
    if (!node || node->node_count == 0) return 0;
    
    uint64_t selected = node->self_node_id;
    uint64_t min_load = (uint64_t)-1;
    
    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].status == NODE_STATUS_ONLINE) {
            if (node->nodes[i].load < min_load) {
                min_load = node->nodes[i].load;
                selected = node->nodes[i].node_id;
            }
        }
    }
    
    return selected;
}
