#include "network/distributed_node.h"
#include "utils/log.h"
#include <string.h>
#include <time.h>

static int serialize_message(const idcu_NodeMessage* msg, uint8_t* buffer, size_t buffer_size, size_t* serialized_size) {
    if (!msg || !buffer || !serialized_size) return IDCU_ERR_INVALID_PARAM;
    
    size_t required_size = 24 + msg->payload_size;
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
    
    if (msg->payload_size > 0) {
        memcpy(buffer + offset, msg->payload, msg->payload_size);
        offset += msg->payload_size;
    }
    
    *serialized_size = offset;
    return IDCU_ERR_OK;
}

static int deserialize_message(const uint8_t* buffer, size_t buffer_size, idcu_NodeMessage* msg) {
    if (!buffer || !msg) return IDCU_ERR_INVALID_PARAM;
    if (buffer_size < 24) return IDCU_ERR_INVALID_PARAM;
    
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
    node->nodes[0].connected = 0;
    node->msg_count = 0;
    node->msg_head = 0;
    node->msg_tail = 0;
    node->server_started = 0;
    
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
    node->nodes[node->node_count].last_heartbeat = (uint64_t)time(NULL);
    node->nodes[node->node_count].connected = 0;
    node->node_count++;
    
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
    IDCU_LOG_INFO("Connected to node %llu", (unsigned long long)node_id);
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
    return IDCU_ERR_OK;
}

int idcu_distributed_node_send_message(idcu_DistributedNode* node, uint64_t to_node, uint32_t type, const void* payload, size_t payload_size) {
    if (!node) return IDCU_ERR_INVALID_PARAM;
    if (node->msg_count >= IDCU_MAX_MESSAGES) return IDCU_ERR_QUEUE_FULL;
    if (payload && payload_size > IDCU_MAX_MESSAGE_PAYLOAD_SIZE) return IDCU_ERR_INVALID_PARAM;
    
    idcu_NodeMessage msg;
    memset(&msg, 0, sizeof(msg));
    msg.magic = IDCU_MESSAGE_MAGIC;
    msg.version = IDCU_MESSAGE_VERSION;
    msg.from_node = node->self_node_id;
    msg.to_node = to_node;
    msg.type = type;
    if (payload && payload_size > 0) {
        memcpy(msg.payload, payload, payload_size);
        msg.payload_size = (uint32_t)payload_size;
    }
    
    idcu_NodeInfo* target = idcu_distributed_node_find_node(node, to_node);
    
    if (to_node == node->self_node_id || !target) {
        node->messages[node->msg_tail] = msg;
        node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
        node->msg_count++;
        return IDCU_ERR_OK;
    }
    
    if (!target->connected) {
        int ret = idcu_distributed_node_connect_to_node(node, to_node);
        if (ret != IDCU_ERR_OK) {
            IDCU_LOG_ERROR("Failed to connect to target node, falling back to local queue");
            node->messages[node->msg_tail] = msg;
            node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
            node->msg_count++;
            return IDCU_ERR_OK;
        }
    }
    
    uint8_t buffer[24 + IDCU_MAX_MESSAGE_PAYLOAD_SIZE];
    size_t serialized_size;
    int ret = serialize_message(&msg, buffer, sizeof(buffer), &serialized_size);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to serialize message, falling back to local queue");
        node->messages[node->msg_tail] = msg;
        node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
        node->msg_count++;
        return IDCU_ERR_OK;
    }
    
    size_t sent;
    ret = idcu_network_socket_send(&target->socket, buffer, serialized_size, &sent);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to send message to node %llu, falling back to local queue", (unsigned long long)to_node);
        idcu_distributed_node_disconnect_from_node(node, to_node);
        node->messages[node->msg_tail] = msg;
        node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
        node->msg_count++;
        return IDCU_ERR_OK;
    }
    
    if (sent != serialized_size) {
        IDCU_LOG_WARN("Partial message sent: %zu/%zu bytes", sent, serialized_size);
    }
    
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

int idcu_distributed_node_process(idcu_DistributedNode* node) {
    if (!node) return IDCU_ERR_INVALID_PARAM;
    
    if (node->server_started) {
        idcu_NetworkSocket client_sock;
        int ret = idcu_network_server_accept(&node->server, &client_sock);
        if (ret == IDCU_ERR_OK && client_sock.connected) {
            uint8_t buffer[24 + IDCU_MAX_MESSAGE_PAYLOAD_SIZE];
            size_t received;
            ret = idcu_network_socket_recv(&client_sock, buffer, sizeof(buffer), &received);
            if (ret == IDCU_ERR_OK && received > 0) {
                idcu_NodeMessage msg;
                ret = deserialize_message(buffer, received, &msg);
                if (ret == IDCU_ERR_OK) {
                    if (msg.to_node == node->self_node_id && node->msg_count < IDCU_MAX_MESSAGES) {
                        node->messages[node->msg_tail] = msg;
                        node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
                        node->msg_count++;
                    }
                }
            }
            idcu_network_socket_destroy(&client_sock);
        }
    }
    
    for (int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].connected && node->nodes[i].node_id != node->self_node_id) {
            uint8_t buffer[24 + IDCU_MAX_MESSAGE_PAYLOAD_SIZE];
            size_t received;
            int ret = idcu_network_socket_recv(&node->nodes[i].socket, buffer, sizeof(buffer), &received);
            if (ret == IDCU_ERR_OK && received > 0) {
                idcu_NodeMessage msg;
                ret = deserialize_message(buffer, received, &msg);
                if (ret == IDCU_ERR_OK) {
                    if (msg.to_node == node->self_node_id && node->msg_count < IDCU_MAX_MESSAGES) {
                        node->messages[node->msg_tail] = msg;
                        node->msg_tail = (node->msg_tail + 1) % IDCU_MAX_MESSAGES;
                        node->msg_count++;
                    }
                }
            } else if (ret != IDCU_ERR_OK) {
                idcu_distributed_node_disconnect_from_node(node, node->nodes[i].node_id);
            }
        }
    }
    
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
