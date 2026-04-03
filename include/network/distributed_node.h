#ifndef IDCU_NETWORK_DISTRIBUTED_NODE_H
#define IDCU_NETWORK_DISTRIBUTED_NODE_H

#include "common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_MAX_NODES 32
#define IDCU_MAX_MESSAGES 64
#define IDCU_MAX_MESSAGE_PAYLOAD_SIZE 1024
#define IDCU_NODE_NAME_MAX 64
#define IDCU_NODE_ADDR_MAX 64

typedef enum {
    IDCU_NODE_STATUS_ONLINE = 0,
    IDCU_NODE_STATUS_OFFLINE,
    IDCU_NODE_STATUS_BUSY,
    IDCU_NODE_STATUS_UNHEALTHY
} idcu_NodeStatus;

typedef struct {
    uint64_t node_id;
    char name[IDCU_NODE_NAME_MAX];
    char address[IDCU_NODE_ADDR_MAX];
    uint16_t port;
    idcu_NodeStatus status;
    uint64_t load;
    uint64_t last_heartbeat;
} idcu_NodeInfo;

typedef struct {
    uint64_t from_node;
    uint64_t to_node;
    uint32_t type;
    uint8_t payload[IDCU_MAX_MESSAGE_PAYLOAD_SIZE];
    size_t payload_size;
} idcu_NodeMessage;

typedef struct {
    uint64_t self_node_id;
    idcu_NodeInfo nodes[IDCU_MAX_NODES];
    int node_count;
    idcu_NodeMessage messages[IDCU_MAX_MESSAGES];
    int msg_count;
    int msg_head;
    int msg_tail;
} idcu_DistributedNode;

int idcu_distributed_node_init(idcu_DistributedNode* node, uint64_t node_id, const char* name, const char* address, uint16_t port);
void idcu_distributed_node_destroy(idcu_DistributedNode* node);
int idcu_distributed_node_add_node(idcu_DistributedNode* node, uint64_t node_id, const char* name, const char* address, uint16_t port);
int idcu_distributed_node_remove_node(idcu_DistributedNode* node, uint64_t node_id);
idcu_NodeInfo* idcu_distributed_node_find_node(idcu_DistributedNode* node, uint64_t node_id);
int idcu_distributed_node_update_node_status(idcu_DistributedNode* node, uint64_t node_id, idcu_NodeStatus status);
int idcu_distributed_node_update_heartbeat(idcu_DistributedNode* node, uint64_t node_id);
int idcu_distributed_node_send_message(idcu_DistributedNode* node, uint64_t to_node, uint32_t type, const void* payload, size_t payload_size);
int idcu_distributed_node_recv_message(idcu_DistributedNode* node, idcu_NodeMessage* msg);
uint64_t idcu_distributed_node_select_node_by_load(idcu_DistributedNode* node);

#endif // IDCU_NETWORK_DISTRIBUTED_NODE_H
