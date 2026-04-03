#ifndef DISTRIBUTED_NODE_H
#define DISTRIBUTED_NODE_H

#include "include/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define MAX_NODES 32
#define MAX_MESSAGES 64
#define MAX_MESSAGE_PAYLOAD_SIZE 1024
#define NODE_NAME_MAX 64
#define NODE_ADDR_MAX 64

typedef enum {
    NODE_STATUS_ONLINE = 0,
    NODE_STATUS_OFFLINE,
    NODE_STATUS_BUSY,
    NODE_STATUS_UNHEALTHY
} NodeStatus;

typedef struct {
    uint64_t node_id;
    char name[NODE_NAME_MAX];
    char address[NODE_ADDR_MAX];
    uint16_t port;
    NodeStatus status;
    uint64_t load;
    uint64_t last_heartbeat;
} NodeInfo;

typedef struct {
    uint64_t from_node;
    uint64_t to_node;
    uint32_t type;
    uint8_t payload[MAX_MESSAGE_PAYLOAD_SIZE];
    size_t payload_size;
} NodeMessage;

typedef struct {
    uint64_t self_node_id;
    NodeInfo nodes[MAX_NODES];
    int node_count;
    NodeMessage messages[MAX_MESSAGES];
    int msg_count;
    int msg_head;
    int msg_tail;
} DistributedNode;

int distributed_node_init(DistributedNode* node, uint64_t node_id, const char* name, const char* address, uint16_t port);
void distributed_node_destroy(DistributedNode* node);
int distributed_node_add_node(DistributedNode* node, uint64_t node_id, const char* name, const char* address, uint16_t port);
int distributed_node_remove_node(DistributedNode* node, uint64_t node_id);
NodeInfo* distributed_node_find_node(DistributedNode* node, uint64_t node_id);
int distributed_node_update_node_status(DistributedNode* node, uint64_t node_id, NodeStatus status);
int distributed_node_update_heartbeat(DistributedNode* node, uint64_t node_id);
int distributed_node_send_message(DistributedNode* node, uint64_t to_node, uint32_t type, const void* payload, size_t payload_size);
int distributed_node_recv_message(DistributedNode* node, NodeMessage* msg);
uint64_t distributed_node_select_node_by_load(DistributedNode* node);

#endif
