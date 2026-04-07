#include "idcu/distributed/distributed.h"
#include "idcu/log/log.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

static uint64_t get_current_time_ms(void) {
#ifdef _WIN32
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

static int serialize_message(const idcu_NodeMessage* msg, uint8_t* buffer, size_t buffer_size, size_t* serialized_size) {
    if (!msg || !buffer || !serialized_size) return IDCU_ERR_INVALID_PARAM;

    size_t required_size = 40 + msg->payload_size;
    if (buffer_size < required_size) return IDCU_ERR_NO_MEMORY;

    size_t offset = 0;

    memcpy(buffer + offset, &msg->magic, sizeof(msg->magic));
    offset += sizeof(msg->magic);

    memcpy(buffer + offset, &msg->version, sizeof(msg->version));
    offset += sizeof(msg->version);

    memcpy(buffer + offset, &msg->from_node, sizeof(msg->from_node));
    offset += sizeof(msg->from_node);

    memcpy(buffer + offset, &msg->to_node, sizeof(msg->to_node));
    offset += sizeof(msg->to_node);

    memcpy(buffer + offset, &msg->type, sizeof(msg->type));
    offset += sizeof(msg->type);

    memcpy(buffer + offset, &msg->payload_size, sizeof(msg->payload_size));
    offset += sizeof(msg->payload_size);

    memcpy(buffer + offset, &msg->sequence_num, sizeof(msg->sequence_num));
    offset += sizeof(msg->sequence_num);

    memcpy(buffer + offset, &msg->timestamp, sizeof(msg->timestamp));
    offset += sizeof(msg->timestamp);

    if (msg->payload_size > 0) {
        memcpy(buffer + offset, msg->payload, msg->payload_size);
        offset += msg->payload_size;
    }

    *serialized_size = offset;
    return IDCU_ERR_OK;
}

static int deserialize_message(const uint8_t* buffer, size_t buffer_size, idcu_NodeMessage* msg) {
    if (!buffer || !msg) return IDCU_ERR_INVALID_PARAM;
    if (buffer_size < 40) return IDCU_ERR_INVALID_PARAM;

    size_t offset = 0;

    memcpy(&msg->magic, buffer + offset, sizeof(msg->magic));
    offset += sizeof(msg->magic);

    if (msg->magic != IDCU_MESSAGE_MAGIC) {
        IDCU_LOG_ERROR("Invalid message magic: 0x%08X", msg->magic);
        return IDCU_ERR_INVALID_PARAM;
    }

    memcpy(&msg->version, buffer + offset, sizeof(msg->version));
    offset += sizeof(msg->version);

    if (msg->version != IDCU_MESSAGE_VERSION) {
        IDCU_LOG_ERROR("Invalid message version: %u", msg->version);
        return IDCU_ERR_INVALID_PARAM;
    }

    memcpy(&msg->from_node, buffer + offset, sizeof(msg->from_node));
    offset += sizeof(msg->from_node);

    memcpy(&msg->to_node, buffer + offset, sizeof(msg->to_node));
    offset += sizeof(msg->to_node);

    memcpy(&msg->type, buffer + offset, sizeof(msg->type));
    offset += sizeof(msg->type);

    memcpy(&msg->payload_size, buffer + offset, sizeof(msg->payload_size));
    offset += sizeof(msg->payload_size);

    memcpy(&msg->sequence_num, buffer + offset, sizeof(msg->sequence_num));
    offset += sizeof(msg->sequence_num);

    memcpy(&msg->timestamp, buffer + offset, sizeof(msg->timestamp));
    offset += sizeof(msg->timestamp);

    if (msg->payload_size > IDCU_MAX_MESSAGE_PAYLOAD_SIZE) {
        IDCU_LOG_ERROR("Payload size too large: %zu", msg->payload_size);
        return IDCU_ERR_INVALID_PARAM;
    }

    if (buffer_size < offset + msg->payload_size) {
        IDCU_LOG_ERROR("Buffer too small for payload");
        return IDCU_ERR_INVALID_PARAM;
    }

    if (msg->payload_size > 0) {
        memcpy(msg->payload, buffer + offset, msg->payload_size);
    }

    return IDCU_ERR_OK;
}

static void update_node_status_counters(idcu_DistributedNode* node) {
    node->total_nodes_online = 0;
    node->total_nodes_offline = 0;
    
    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].node_id == node->self_node_id) continue;
        
        if (node->nodes[i].status == IDCU_NODE_STATUS_ONLINE) {
            node->total_nodes_online++;
        } else {
            node->total_nodes_offline++;
        }
    }
}

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
    node->nodes[0].weight = 100;
    node->nodes[0].last_heartbeat = get_current_time_ms();
    node->nodes[0].connected = 0;
    node->msg_count = 0;
    node->msg_head = 0;
    node->msg_tail = 0;
    node->server_started = 0;
    node->lb_strategy = IDCU_LOAD_BALANCE_LEAST_LOAD;
    node->round_robin_index = 0;
    node->pending_msg_count = 0;
    node->next_sequence_num = 1;
    node->total_nodes_online = 0;
    node->total_nodes_offline = 0;
    node->total_messages_sent = 0;
    node->total_messages_received = 0;
    node->total_bytes_sent = 0;
    node->total_bytes_received = 0;

    int ret = idcu_network_init();
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to initialize network");
        return ret;
    }

    return IDCU_ERR_OK;
}

void idcu_distributed_node_destroy(idcu_DistributedNode* node) {
    if (!node) return;

    if (node->server_started) {
        idcu_distributed_node_stop_server(node);
    }

    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].connected) {
            idcu_distributed_node_disconnect_from_node(node, node->nodes[i].node_id);
        }
    }

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
    node->nodes[node->node_count].status = IDCU_NODE_STATUS_OFFLINE;
    node->nodes[node->node_count].load = 0;
    node->nodes[node->node_count].weight = 100;
    node->nodes[node->node_count].last_heartbeat = get_current_time_ms();
    node->nodes[node->node_count].latency_ms = 0;
    node->nodes[node->node_count].reconnect_attempts = 0;
    node->nodes[node->node_count].next_reconnect_time = 0;
    node->nodes[node->node_count].connected = 0;
    node->nodes[node->node_count].messages_sent = 0;
    node->nodes[node->node_count].messages_received = 0;
    node->nodes[node->node_count].bytes_sent = 0;
    node->nodes[node->node_count].bytes_received = 0;
    node->node_count++;
    
    update_node_status_counters(node);
    return IDCU_ERR_OK;
}

int idcu_distributed_node_remove_node(idcu_DistributedNode* node, uint64_t node_id) {
    if (!node) return IDCU_ERR_INVALID_PARAM;
    if (node_id == node->self_node_id) return IDCU_ERR_INVALID_PARAM;

    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].node_id == node_id) {
            if (node->nodes[i].connected) {
                idcu_distributed_node_disconnect_from_node(node, node_id);
            }
            for (int j = i; j < node->node_count - 1; j++) {
                node->nodes[j] = node->nodes[j + 1];
            }
            node->node_count--;
            update_node_status_counters(node);
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
    update_node_status_counters(node);
    return IDCU_ERR_OK;
}

int idcu_distributed_node_update_heartbeat(idcu_DistributedNode* node, uint64_t node_id) {
    if (!node) return IDCU_ERR_INVALID_PARAM;

    idcu_NodeInfo* info = idcu_distributed_node_find_node(node, node_id);
    if (!info) return IDCU_ERR_NOT_FOUND;

    info->last_heartbeat = get_current_time_ms();
    info->reconnect_attempts = 0;
    return IDCU_ERR_OK;
}

int idcu_distributed_node_set_load_balance_strategy(idcu_DistributedNode* node, idcu_LoadBalanceStrategy strategy) {
    if (!node) return IDCU_ERR_INVALID_PARAM;
    node->lb_strategy = strategy;
    return IDCU_ERR_OK;
}

int idcu_distributed_node_set_node_weight(idcu_DistributedNode* node, uint64_t node_id, uint64_t weight) {
    if (!node) return IDCU_ERR_INVALID_PARAM;
    
    idcu_NodeInfo* info = idcu_distributed_node_find_node(node, node_id);
    if (!info) return IDCU_ERR_NOT_FOUND;
    
    info->weight = weight;
    return IDCU_ERR_OK;
}

uint64_t idcu_distributed_node_select_node_by_load(idcu_DistributedNode* node) {
    if (!node || node->node_count == 0) return 0;

    uint64_t selected = node->self_node_id;
    uint64_t min_load = (uint64_t)-1;

    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].status == IDCU_NODE_STATUS_ONLINE && node->nodes[i].node_id != node->self_node_id) {
            if (node->nodes[i].load < min_load) {
                min_load = node->nodes[i].load;
                selected = node->nodes[i].node_id;
            }
        }
    }

    return selected;
}

uint64_t idcu_distributed_node_select_node(idcu_DistributedNode* node) {
    if (!node || node->node_count <= 1) return node->self_node_id;

    switch (node->lb_strategy) {
        case IDCU_LOAD_BALANCE_ROUND_ROBIN: {
            int online_count = 0;
            int first_online = -1;
            
            for (int i = 0; i < node->node_count; i++) {
                if (node->nodes[i].status == IDCU_NODE_STATUS_ONLINE && node->nodes[i].node_id != node->self_node_id) {
                    online_count++;
                    if (first_online == -1) first_online = i;
                }
            }
            
            if (online_count == 0) return node->self_node_id;
            
            for (;;) {
                node->round_robin_index = (node->round_robin_index + 1) % node->node_count;
                if (node->nodes[node->round_robin_index].status == IDCU_NODE_STATUS_ONLINE && 
                    node->nodes[node->round_robin_index].node_id != node->self_node_id) {
                    return node->nodes[node->round_robin_index].node_id;
                }
            }
        }
        
        case IDCU_LOAD_BALANCE_LEAST_LOAD:
            return idcu_distributed_node_select_node_by_load(node);
            
        case IDCU_LOAD_BALANCE_RANDOM: {
            int online_indices[IDCU_MAX_NODES];
            int online_count = 0;
            
            for (int i = 0; i < node->node_count; i++) {
                if (node->nodes[i].status == IDCU_NODE_STATUS_ONLINE && node->nodes[i].node_id != node->self_node_id) {
                    online_indices[online_count++] = i;
                }
            }
            
            if (online_count == 0) return node->self_node_id;
            
            int selected_idx = online_indices[rand() % online_count];
            return node->nodes[selected_idx].node_id;
        }
        
        case IDCU_LOAD_BALANCE_WEIGHTED: {
            uint64_t total_weight = 0;
            int online_indices[IDCU_MAX_NODES];
            int online_count = 0;
            
            for (int i = 0; i < node->node_count; i++) {
                if (node->nodes[i].status == IDCU_NODE_STATUS_ONLINE && node->nodes[i].node_id != node->self_node_id) {
                    online_indices[online_count] = i;
                    total_weight += node->nodes[i].weight;
                    online_count++;
                }
            }
            
            if (online_count == 0) return node->self_node_id;
            
            uint64_t r = (uint64_t)(rand() % total_weight);
            uint64_t cumulative = 0;
            
            for (int i = 0; i < online_count; i++) {
                int idx = online_indices[i];
                cumulative += node->nodes[idx].weight;
                if (r < cumulative) {
                    return node->nodes[idx].node_id;
                }
            }
            
            return node->nodes[online_indices[0]].node_id;
        }
        
        default:
            return idcu_distributed_node_select_node_by_load(node);
    }
}

static int send_internal_message(idcu_DistributedNode* node, idcu_NodeMessage* msg) {
    idcu_NodeInfo* target = idcu_distributed_node_find_node(node, msg->to_node);

    if (msg->to_node == node->self_node_id || !target) {
        if (node->msg_count >= IDCU_MAX_MESSAGES) return IDCU_ERR_QUEUE_FULL;
        node->messages[node->msg_tail] = *msg;
        node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
        node->msg_count++;
        return IDCU_ERR_OK;
    }

    if (!target->connected) {
        int ret = idcu_distributed_node_connect_to_node(node, msg->to_node);
        if (ret != IDCU_ERR_OK) {
            IDCU_LOG_ERROR("Failed to connect to target node, falling back to local queue");
            if (node->msg_count >= IDCU_MAX_MESSAGES) return IDCU_ERR_QUEUE_FULL;
            node->messages[node->msg_tail] = *msg;
            node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
            node->msg_count++;
            return IDCU_ERR_OK;
        }
    }

    uint8_t buffer[40 + IDCU_MAX_MESSAGE_PAYLOAD_SIZE];
    size_t serialized_size;
    int ret = serialize_message(msg, buffer, sizeof(buffer), &serialized_size);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to serialize message, falling back to local queue");
        if (node->msg_count >= IDCU_MAX_MESSAGES) return IDCU_ERR_QUEUE_FULL;
        node->messages[node->msg_tail] = *msg;
        node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
        node->msg_count++;
        return IDCU_ERR_OK;
    }

    size_t sent;
    ret = idcu_network_socket_send(&target->socket, buffer, serialized_size, &sent);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to send message to node %llu, falling back to local queue", (unsigned long long)msg->to_node);
        idcu_distributed_node_disconnect_from_node(node, msg->to_node);
        if (node->msg_count >= IDCU_MAX_MESSAGES) return IDCU_ERR_QUEUE_FULL;
        node->messages[node->msg_tail] = *msg;
        node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
        node->msg_count++;
        return IDCU_ERR_OK;
    }

    target->messages_sent++;
    target->bytes_sent += sent;
    node->total_messages_sent++;
    node->total_bytes_sent += sent;

    if (sent != serialized_size) {
        IDCU_LOG_WARN("Partial message sent: %zu/%zu bytes", sent, serialized_size);
    }

    return IDCU_ERR_OK;
}

int idcu_distributed_node_send_message(idcu_DistributedNode* node, uint64_t to_node, uint32_t type, const void* payload, size_t payload_size) {    
    if (!node) return IDCU_ERR_INVALID_PARAM;
    if (payload && payload_size > IDCU_MAX_MESSAGE_PAYLOAD_SIZE) return IDCU_ERR_INVALID_PARAM;

    idcu_NodeMessage msg;
    memset(&msg, 0, sizeof(msg));
    msg.magic = IDCU_MESSAGE_MAGIC;
    msg.version = IDCU_MESSAGE_VERSION;
    msg.from_node = node->self_node_id;
    msg.to_node = to_node;
    msg.type = type;
    msg.sequence_num = node->next_sequence_num++;
    msg.timestamp = get_current_time_ms();
    if (payload && payload_size > 0) {
        memcpy(msg.payload, payload, payload_size);
        msg.payload_size = (uint32_t)payload_size;
    }

    return send_internal_message(node, &msg);
}

int idcu_distributed_node_send_message_reliable(idcu_DistributedNode* node, uint64_t to_node, uint32_t type, const void* payload, size_t payload_size) {
    if (!node) return IDCU_ERR_INVALID_PARAM;
    if (payload && payload_size > IDCU_MAX_MESSAGE_PAYLOAD_SIZE) return IDCU_ERR_INVALID_PARAM;
    if (node->pending_msg_count >= IDCU_MAX_PENDING_MESSAGES) return IDCU_ERR_QUEUE_FULL;

    idcu_PendingMessage* pending = &node->pending_msgs[node->pending_msg_count];
    memset(pending, 0, sizeof(idcu_PendingMessage));
    
    pending->msg.magic = IDCU_MESSAGE_MAGIC;
    pending->msg.version = IDCU_MESSAGE_VERSION;
    pending->msg.from_node = node->self_node_id;
    pending->msg.to_node = to_node;
    pending->msg.type = type;
    pending->msg.sequence_num = node->next_sequence_num++;
    pending->msg.timestamp = get_current_time_ms();
    if (payload && payload_size > 0) {
        memcpy(pending->msg.payload, payload, payload_size);
        pending->msg.payload_size = (uint32_t)payload_size;
    }
    
    pending->sent_time = get_current_time_ms();
    pending->retries = 0;
    pending->pending = 1;
    node->pending_msg_count++;
    
    return send_internal_message(node, &pending->msg);
}

int idcu_distributed_node_recv_message(idcu_DistributedNode* node, idcu_NodeMessage* msg) {
    if (!node || !msg) return IDCU_ERR_INVALID_PARAM;
    if (node->msg_count == 0) return IDCU_ERR_QUEUE_EMPTY;

    *msg = node->messages[node->msg_head];
    node->msg_head = (node->msg_head + 1) % IDCU_MAX_MESSAGES;
    node->msg_count--;

    return IDCU_ERR_OK;
}

int idcu_distributed_node_start_server(idcu_DistributedNode* node) {
    if (!node) return IDCU_ERR_INVALID_PARAM;
    if (node->server_started) return IDCU_ERR_OK;

    idcu_NodeInfo* self = idcu_distributed_node_find_node(node, node->self_node_id);
    if (!self) return IDCU_ERR_NOT_FOUND;

    int ret = idcu_network_server_create(&node->server, IDCU_NET_PROTO_TCP, self->address, self->port);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to create network server");
        return ret;
    }

    ret = idcu_network_server_listen(&node->server);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to start listening");
        idcu_network_server_destroy(&node->server);
        return ret;
    }

    node->server_started = 1;
    IDCU_LOG_INFO("Server started on %s:%u", self->address, self->port);
    return IDCU_ERR_OK;
}

int idcu_distributed_node_stop_server(idcu_DistributedNode* node) {
    if (!node) return IDCU_ERR_INVALID_PARAM;
    if (!node->server_started) return IDCU_ERR_OK;

    idcu_network_server_destroy(&node->server);
    node->server_started = 0;
    IDCU_LOG_INFO("Server stopped");
    return IDCU_ERR_OK;
}

int idcu_distributed_node_connect_to_node(idcu_DistributedNode* node, uint64_t node_id) {
    if (!node) return IDCU_ERR_INVALID_PARAM;

    idcu_NodeInfo* target = idcu_distributed_node_find_node(node, node_id);
    if (!target) return IDCU_ERR_NOT_FOUND;
    if (target->connected) return IDCU_ERR_OK;
    if (node_id == node->self_node_id) return IDCU_ERR_INVALID_PARAM;

    int ret = idcu_network_socket_create(&target->socket, IDCU_NET_PROTO_TCP);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to create socket for node %llu", (unsigned long long)node_id);
        return ret;
    }

    ret = idcu_network_socket_connect(&target->socket, target->address, target->port);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to connect to node %llu at %s:%u",
                     (unsigned long long)node_id, target->address, target->port);
        idcu_network_socket_destroy(&target->socket);
        return ret;
    }

    target->connected = 1;
    target->status = IDCU_NODE_STATUS_ONLINE;
    target->reconnect_attempts = 0;
    IDCU_LOG_INFO("Connected to node %llu", (unsigned long long)node_id);
    update_node_status_counters(node);
    return IDCU_ERR_OK;
}

int idcu_distributed_node_disconnect_from_node(idcu_DistributedNode* node, uint64_t node_id) {
    if (!node) return IDCU_ERR_INVALID_PARAM;

    idcu_NodeInfo* target = idcu_distributed_node_find_node(node, node_id);
    if (!target) return IDCU_ERR_NOT_FOUND;
    if (!target->connected) return IDCU_ERR_OK;

    idcu_network_socket_destroy(&target->socket);
    target->connected = 0;
    target->status = IDCU_NODE_STATUS_OFFLINE;
    IDCU_LOG_INFO("Disconnected from node %llu", (unsigned long long)node_id);
    update_node_status_counters(node);
    return IDCU_ERR_OK;
}

int idcu_distributed_node_reconnect_to_node(idcu_DistributedNode* node, uint64_t node_id) {
    if (!node) return IDCU_ERR_INVALID_PARAM;

    idcu_NodeInfo* target = idcu_distributed_node_find_node(node, node_id);
    if (!target) return IDCU_ERR_NOT_FOUND;
    if (node_id == node->self_node_id) return IDCU_ERR_INVALID_PARAM;

    uint64_t current_time = get_current_time_ms();
    
    if (target->reconnect_attempts >= IDCU_RECONNECT_MAX_ATTEMPTS) {
        IDCU_LOG_ERROR("Max reconnect attempts reached for node %llu", (unsigned long long)node_id);
        target->status = IDCU_NODE_STATUS_OFFLINE;
        update_node_status_counters(node);
        return IDCU_ERR_NETWORK;
    }
    
    if (target->next_reconnect_time > current_time) {
        return IDCU_ERR_OK;
    }
    
    IDCU_LOG_INFO("Attempting to reconnect to node %llu (attempt %u/%u)", 
                 (unsigned long long)node_id, 
                 (unsigned int)target->reconnect_attempts + 1,
                 (unsigned int)IDCU_RECONNECT_MAX_ATTEMPTS);
    
    target->status = IDCU_NODE_STATUS_RECONNECTING;
    
    if (target->connected) {
        idcu_distributed_node_disconnect_from_node(node, node_id);
    }
    
    int ret = idcu_distributed_node_connect_to_node(node, node_id);
    if (ret == IDCU_ERR_OK) {
        IDCU_LOG_INFO("Successfully reconnected to node %llu", (unsigned long long)node_id);
        return IDCU_ERR_OK;
    }
    
    target->reconnect_attempts++;
    uint64_t delay = IDCU_RECONNECT_DELAY_MS;
    for (uint64_t i = 1; i < target->reconnect_attempts; i++) {
        delay *= IDCU_RECONNECT_BACKOFF_FACTOR;
    }
    target->next_reconnect_time = current_time + delay;
    
    IDCU_LOG_WARN("Reconnect attempt failed for node %llu, next attempt in %llu ms", 
                 (unsigned long long)node_id, (unsigned long long)delay);
    
    return ret;
}

static void process_heartbeat(idcu_DistributedNode* node, const idcu_NodeMessage* msg) {
    idcu_NodeInfo* sender = idcu_distributed_node_find_node(node, msg->from_node);
    if (sender) {
        idcu_distributed_node_update_heartbeat(node, msg->from_node);
        uint64_t current_time = get_current_time_ms();
        if (msg->timestamp > 0 && msg->timestamp < current_time) {
            sender->latency_ms = current_time - msg->timestamp;
        }
    }
}

static void process_ack(idcu_DistributedNode* node, const idcu_NodeMessage* msg) {
    for (int i = 0; i < node->pending_msg_count; i++) {
        if (node->pending_msgs[i].pending && 
            node->pending_msgs[i].msg.sequence_num == msg->sequence_num) {
            node->pending_msgs[i].pending = 0;
            break;
        }
    }
}

static void cleanup_pending_messages(idcu_DistributedNode* node) {
    uint64_t current_time = get_current_time_ms();
    
    for (int i = 0; i < node->pending_msg_count; i++) {
        if (!node->pending_msgs[i].pending) continue;
        
        if (current_time - node->pending_msgs[i].sent_time > 5000) {
            if (node->pending_msgs[i].retries < 3) {
                node->pending_msgs[i].retries++;
                node->pending_msgs[i].sent_time = current_time;
                send_internal_message(node, &node->pending_msgs[i].msg);
                IDCU_LOG_DEBUG("Retransmitting message seq=%u", node->pending_msgs[i].msg.sequence_num);
            } else {
                node->pending_msgs[i].pending = 0;
                IDCU_LOG_ERROR("Message delivery failed after retries, seq=%u", node->pending_msgs[i].msg.sequence_num);
            }
        }
    }
    
    int write_idx = 0;
    for (int i = 0; i < node->pending_msg_count; i++) {
        if (node->pending_msgs[i].pending) {
            if (write_idx != i) {
                node->pending_msgs[write_idx] = node->pending_msgs[i];
            }
            write_idx++;
        }
    }
    node->pending_msg_count = write_idx;
}

static void check_node_timeouts(idcu_DistributedNode* node) {
    uint64_t current_time = get_current_time_ms();
    
    for (int i = 0; i < node->node_count; i++) {
        idcu_NodeInfo* node_info = &node->nodes[i];
        if (node_info->node_id == node->self_node_id) continue;
        
        if (node_info->status == IDCU_NODE_STATUS_ONLINE && 
            current_time - node_info->last_heartbeat > IDCU_HEARTBEAT_TIMEOUT_MS) {
            IDCU_LOG_WARN("Node %llu heartbeat timeout, marking as offline", (unsigned long long)node_info->node_id);
            node_info->status = IDCU_NODE_STATUS_OFFLINE;
            node_info->next_reconnect_time = current_time;
            update_node_status_counters(node);
        }
        
        if ((node_info->status == IDCU_NODE_STATUS_OFFLINE || node_info->status == IDCU_NODE_STATUS_RECONNECTING) &&
            node_info->reconnect_attempts < IDCU_RECONNECT_MAX_ATTEMPTS) {
            idcu_distributed_node_reconnect_to_node(node, node_info->node_id);
        }
    }
}

static void send_heartbeats(idcu_DistributedNode* node) {
    static uint64_t last_heartbeat_send = 0;
    uint64_t current_time = get_current_time_ms();
    
    if (current_time - last_heartbeat_send < IDCU_HEARTBEAT_INTERVAL_MS) return;
    last_heartbeat_send = current_time;
    
    for (int i = 0; i < node->node_count; i++) {
        idcu_NodeInfo* node_info = &node->nodes[i];
        if (node_info->node_id == node->self_node_id) continue;
        if (node_info->status != IDCU_NODE_STATUS_ONLINE) continue;
        
        idcu_distributed_node_send_message(node, node_info->node_id, IDCU_MSG_TYPE_HEARTBEAT, NULL, 0);
    }
}

int idcu_distributed_node_process(idcu_DistributedNode* node) {
    if (!node) return IDCU_ERR_INVALID_PARAM;

    if (node->server_started) {
        idcu_NetworkSocket client_sock;
        int ret = idcu_network_server_accept(&node->server, &client_sock);
        if (ret == IDCU_ERR_OK && client_sock.connected) {
            uint8_t buffer[40 + IDCU_MAX_MESSAGE_PAYLOAD_SIZE];
            size_t received;
            ret = idcu_network_socket_recv(&client_sock, buffer, sizeof(buffer), &received);
            if (ret == IDCU_ERR_OK && received > 0) {
                idcu_NodeMessage msg;
                ret = deserialize_message(buffer, received, &msg);
                if (ret == IDCU_ERR_OK) {
                    if (msg.to_node == node->self_node_id) {
                        node->total_messages_received++;
                        node->total_bytes_received += received;
                        
                        idcu_NodeInfo* sender = idcu_distributed_node_find_node(node, msg.from_node);
                        if (sender) {
                            sender->messages_received++;
                            sender->bytes_received += received;
                        }
                        
                        switch (msg.type) {
                            case IDCU_MSG_TYPE_HEARTBEAT:
                                process_heartbeat(node, &msg);
                                break;
                            case IDCU_MSG_TYPE_ACK:
                                process_ack(node, &msg);
                                break;
                            default:
                                if (node->msg_count < IDCU_MAX_MESSAGES) {
                                    node->messages[node->msg_tail] = msg;
                                    node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
                                    node->msg_count++;
                                }
                                break;
                        }
                    }
                }
            }
            idcu_network_socket_destroy(&client_sock);
        }
    }

    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].connected && node->nodes[i].node_id != node->self_node_id) {
            uint8_t buffer[40 + IDCU_MAX_MESSAGE_PAYLOAD_SIZE];
            size_t received;
            int ret = idcu_network_socket_recv(&node->nodes[i].socket, buffer, sizeof(buffer), &received);
            if (ret == IDCU_ERR_OK && received > 0) {
                idcu_NodeMessage msg;
                ret = deserialize_message(buffer, received, &msg);
                if (ret == IDCU_ERR_OK) {
                    if (msg.to_node == node->self_node_id) {
                        node->total_messages_received++;
                        node->total_bytes_received += received;
                        node->nodes[i].messages_received++;
                        node->nodes[i].bytes_received += received;
                        
                        switch (msg.type) {
                            case IDCU_MSG_TYPE_HEARTBEAT:
                                process_heartbeat(node, &msg);
                                break;
                            case IDCU_MSG_TYPE_ACK:
                                process_ack(node, &msg);
                                break;
                            default:
                                if (node->msg_count < IDCU_MAX_MESSAGES) {
                                    node->messages[node->msg_tail] = msg;
                                    node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
                                    node->msg_count++;
                                }
                                break;
                        }
                    }
                }
            } else if (ret != IDCU_ERR_OK) {
                idcu_distributed_node_disconnect_from_node(node, node->nodes[i].node_id);
            }
        }
    }
    
    send_heartbeats(node);
    check_node_timeouts(node);
    cleanup_pending_messages(node);

    return IDCU_ERR_OK;
}

int idcu_distributed_node_get_metrics(idcu_DistributedNode* node, idcu_DistributedMetrics* metrics) {
    if (!node || !metrics) return IDCU_ERR_INVALID_PARAM;
    
    memset(metrics, 0, sizeof(idcu_DistributedMetrics));
    metrics->total_nodes = node->node_count > 0 ? node->node_count - 1 : 0;
    metrics->nodes_online = node->total_nodes_online;
    metrics->nodes_offline = node->total_nodes_offline;
    metrics->total_messages_sent = node->total_messages_sent;
    metrics->total_messages_received = node->total_messages_received;
    metrics->total_bytes_sent = node->total_bytes_sent;
    metrics->total_bytes_received = node->total_bytes_received;
    
    if (metrics->total_nodes > 0) {
        metrics->online_rate = (double)metrics->nodes_online / (double)metrics->total_nodes;
    }
    
    uint64_t total_latency = 0;
    int online_count = 0;
    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].node_id != node->self_node_id && node->nodes[i].status == IDCU_NODE_STATUS_ONLINE) {
            total_latency += node->nodes[i].latency_ms;
            online_count++;
        }
    }
    if (online_count > 0) {
        metrics->avg_latency_ms = total_latency / online_count;
    }
    
    return IDCU_ERR_OK;
}

int idcu_distributed_node_get_node_metrics(idcu_DistributedNode* node, uint64_t node_id, idcu_NodeMetrics* node_metrics) {
    if (!node || !node_metrics) return IDCU_ERR_INVALID_PARAM;
    
    idcu_NodeInfo* info = idcu_distributed_node_find_node(node, node_id);
    if (!info) return IDCU_ERR_NOT_FOUND;
    
    memset(node_metrics, 0, sizeof(idcu_NodeMetrics));
    node_metrics->node_id = info->node_id;
    strncpy(node_metrics->name, info->name, IDCU_NODE_NAME_MAX - 1);
    strncpy(node_metrics->address, info->address, IDCU_NODE_ADDR_MAX - 1);
    node_metrics->port = info->port;
    node_metrics->status = info->status;
    node_metrics->load = info->load;
    node_metrics->latency_ms = info->latency_ms;
    node_metrics->messages_sent = info->messages_sent;
    node_metrics->messages_received = info->messages_received;
    node_metrics->bytes_sent = info->bytes_sent;
    node_metrics->bytes_received = info->bytes_received;
    
    return IDCU_ERR_OK;
}
