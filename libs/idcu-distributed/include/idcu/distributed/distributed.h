#ifndef IDCU_DISTRIBUTED_H
#define IDCU_DISTRIBUTED_H

#include "idcu/common/error_code.h"
#include "idcu/network/network_layer.h"

#include <stddef.h>
#include <stdint.h>

#define IDCU_MAX_NODES                32
#define IDCU_MAX_MESSAGES             64
#define IDCU_MAX_MESSAGE_PAYLOAD_SIZE 1024
#define IDCU_NODE_NAME_MAX            64
#define IDCU_NODE_ADDR_MAX            64
#define IDCU_MAX_CONNECTIONS          16
#define IDCU_MESSAGE_MAGIC            0x49444355  // "IDCU" in hex
#define IDCU_MESSAGE_VERSION          1

#define IDCU_RECONNECT_MAX_ATTEMPTS   5
#define IDCU_RECONNECT_DELAY_MS       2000
#define IDCU_RECONNECT_BACKOFF_FACTOR 2
#define IDCU_HEARTBEAT_INTERVAL_MS    3000
#define IDCU_HEARTBEAT_TIMEOUT_MS     10000

typedef enum
{
    IDCU_NODE_STATUS_ONLINE = 0,
    IDCU_NODE_STATUS_OFFLINE,
    IDCU_NODE_STATUS_BUSY,
    IDCU_NODE_STATUS_UNHEALTHY,
    IDCU_NODE_STATUS_RECONNECTING
} idcu_NodeStatus;

typedef enum
{
    IDCU_LOAD_BALANCE_ROUND_ROBIN = 0,
    IDCU_LOAD_BALANCE_LEAST_LOAD,
    IDCU_LOAD_BALANCE_RANDOM,
    IDCU_LOAD_BALANCE_WEIGHTED
} idcu_LoadBalanceStrategy;

typedef struct
{
    uint64_t           node_id;
    char               name[IDCU_NODE_NAME_MAX];
    char               address[IDCU_NODE_ADDR_MAX];
    uint16_t           port;
    idcu_NodeStatus    status;
    uint64_t           load;
    uint64_t           weight;
    uint64_t           last_heartbeat;
    uint64_t           latency_ms;
    uint64_t           reconnect_attempts;
    uint64_t           next_reconnect_time;
    idcu_NetworkSocket socket;
    int                connected;
    uint64_t           messages_sent;
    uint64_t           messages_received;
    uint64_t           bytes_sent;
    uint64_t           bytes_received;
} idcu_NodeInfo;

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint64_t from_node;
    uint64_t to_node;
    uint32_t type;
    uint32_t payload_size;
    uint32_t sequence_num;
    uint64_t timestamp;
    uint8_t  payload[IDCU_MAX_MESSAGE_PAYLOAD_SIZE];
} idcu_NodeMessage;

typedef enum
{
    IDCU_MSG_TYPE_DATA = 0,
    IDCU_MSG_TYPE_HEARTBEAT,
    IDCU_MSG_TYPE_ACK,
    IDCU_MSG_TYPE_RETRANSMIT
} idcu_MessageType;

typedef struct
{
    idcu_NodeMessage msg;
    uint64_t         sent_time;
    int              retries;
    int              pending;
} idcu_PendingMessage;

#define IDCU_MAX_PENDING_MESSAGES 32

typedef struct
{
    uint64_t                 self_node_id;
    idcu_NodeInfo            nodes[IDCU_MAX_NODES];
    int                      node_count;
    idcu_NodeMessage         messages[IDCU_MAX_MESSAGES];
    int                      msg_count;
    int                      msg_head;
    int                      msg_tail;
    idcu_NetworkServer       server;
    int                      server_started;
    idcu_LoadBalanceStrategy lb_strategy;
    uint64_t                 round_robin_index;
    idcu_PendingMessage      pending_msgs[IDCU_MAX_PENDING_MESSAGES];
    int                      pending_msg_count;
    uint32_t                 next_sequence_num;
    uint64_t                 total_nodes_online;
    uint64_t                 total_nodes_offline;
    uint64_t                 total_messages_sent;
    uint64_t                 total_messages_received;
    uint64_t                 total_bytes_sent;
    uint64_t                 total_bytes_received;
} idcu_DistributedNode;

typedef struct
{
    uint64_t        node_id;
    char            name[IDCU_NODE_NAME_MAX];
    char            address[IDCU_NODE_ADDR_MAX];
    uint16_t        port;
    idcu_NodeStatus status;
    uint64_t        load;
    uint64_t        latency_ms;
    uint64_t        messages_sent;
    uint64_t        messages_received;
    uint64_t        bytes_sent;
    uint64_t        bytes_received;
} idcu_NodeMetrics;

typedef struct
{
    uint64_t total_nodes;
    uint64_t nodes_online;
    uint64_t nodes_offline;
    double   online_rate;
    uint64_t total_messages_sent;
    uint64_t total_messages_received;
    uint64_t total_bytes_sent;
    uint64_t total_bytes_received;
    uint64_t avg_latency_ms;
} idcu_DistributedMetrics;

int  idcu_distributed_node_init(idcu_DistributedNode* node, uint64_t node_id, const char* name,
                                const char* address, uint16_t port);
void idcu_distributed_node_destroy(idcu_DistributedNode* node);
int  idcu_distributed_node_add_node(idcu_DistributedNode* node, uint64_t node_id, const char* name,
                                    const char* address, uint16_t port);
int  idcu_distributed_node_remove_node(idcu_DistributedNode* node, uint64_t node_id);
idcu_NodeInfo* idcu_distributed_node_find_node(idcu_DistributedNode* node, uint64_t node_id);
int idcu_distributed_node_update_node_status(idcu_DistributedNode* node, uint64_t node_id,
                                             idcu_NodeStatus status);
int idcu_distributed_node_update_heartbeat(idcu_DistributedNode* node, uint64_t node_id);
int idcu_distributed_node_send_message(idcu_DistributedNode* node, uint64_t to_node, uint32_t type,
                                       const void* payload, size_t payload_size);
int idcu_distributed_node_send_message_reliable(idcu_DistributedNode* node, uint64_t to_node,
                                                uint32_t type, const void* payload,
                                                size_t payload_size);
int idcu_distributed_node_recv_message(idcu_DistributedNode* node, idcu_NodeMessage* msg);
uint64_t idcu_distributed_node_select_node_by_load(idcu_DistributedNode* node);
uint64_t idcu_distributed_node_select_node(idcu_DistributedNode* node);
int      idcu_distributed_node_set_load_balance_strategy(idcu_DistributedNode*    node,
                                                         idcu_LoadBalanceStrategy strategy);
int      idcu_distributed_node_set_node_weight(idcu_DistributedNode* node, uint64_t node_id,
                                               uint64_t weight);
int      idcu_distributed_node_start_server(idcu_DistributedNode* node);
int      idcu_distributed_node_stop_server(idcu_DistributedNode* node);
int      idcu_distributed_node_connect_to_node(idcu_DistributedNode* node, uint64_t node_id);
int      idcu_distributed_node_disconnect_from_node(idcu_DistributedNode* node, uint64_t node_id);
int      idcu_distributed_node_reconnect_to_node(idcu_DistributedNode* node, uint64_t node_id);
int      idcu_distributed_node_process(idcu_DistributedNode* node);
int idcu_distributed_node_get_metrics(idcu_DistributedNode* node, idcu_DistributedMetrics* metrics);
int idcu_distributed_node_get_node_metrics(idcu_DistributedNode* node, uint64_t node_id,
                                           idcu_NodeMetrics* metrics);

#endif
