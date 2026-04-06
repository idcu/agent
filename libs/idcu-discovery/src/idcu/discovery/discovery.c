#include "idcu/discovery/discovery.h"
#include "idcu/log/log.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

static uint64_t get_current_time_ms(void) {
#ifdef _WIN32
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

int idcu_node_discovery_init(idcu_NodeDiscovery* disc, idcu_DistributedNode* dist_node) {
    if (!disc || !dist_node) return IDCU_ERR_INVALID_PARAM;

    memset(disc, 0, sizeof(idcu_NodeDiscovery));
    disc->dist_node = dist_node;
    disc->discovered_handler = NULL;
    disc->discovered_user_data = NULL;
    disc->lost_handler = NULL;
    disc->lost_user_data = NULL;
    disc->running = 0;
    disc->last_broadcast_time = 0;

    int ret = idcu_network_init();
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to initialize network");
        return ret;
    }

    return IDCU_ERR_OK;
}

void idcu_node_discovery_destroy(idcu_NodeDiscovery* disc) {
    if (!disc) return;

    idcu_node_discovery_stop(disc);
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

int idcu_node_discovery_start(idcu_NodeDiscovery* disc) {
    if (!disc || !disc->dist_node) return IDCU_ERR_INVALID_PARAM;
    if (disc->running) return IDCU_ERR_OK;

    int ret = idcu_network_socket_create(&disc->socket, IDCU_NET_PROTO_UDP);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to create discovery socket");
        return ret;
    }

    int broadcast = 1;
    setsockopt(disc->socket.fd, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast));

    int reuse = 1;
    setsockopt(disc->socket.fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

#ifdef SO_REUSEPORT
    setsockopt(disc->socket.fd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse));
#endif

    ret = idcu_network_socket_bind(&disc->socket, "0.0.0.0", IDCU_DISCOVERY_PORT);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("Failed to bind discovery socket");
        idcu_network_socket_destroy(&disc->socket);
        return ret;
    }

    disc->running = 1;
    disc->last_broadcast_time = get_current_time_ms();

    IDCU_LOG_INFO("Node discovery started on port %d", IDCU_DISCOVERY_PORT);
    return IDCU_ERR_OK;
}

int idcu_node_discovery_stop(idcu_NodeDiscovery* disc) {
    if (!disc) return IDCU_ERR_INVALID_PARAM;
    if (!disc->running) return IDCU_ERR_OK;

    disc->running = 0;
    idcu_network_socket_destroy(&disc->socket);

    IDCU_LOG_INFO("Node discovery stopped");
    return IDCU_ERR_OK;
}

static int send_discovery_packet(idcu_NodeDiscovery* disc) {
    idcu_DiscoveryPacket packet;
    memset(&packet, 0, sizeof(packet));

    packet.magic = IDCU_DISCOVERY_MAGIC;
    packet.version = IDCU_DISCOVERY_VERSION;
    packet.node_id = disc->dist_node->self_node_id;
    strncpy(packet.name, disc->dist_node->nodes[0].name, IDCU_NODE_NAME_MAX - 1);
    strncpy(packet.address, disc->dist_node->nodes[0].address, IDCU_NODE_ADDR_MAX - 1);
    packet.port = disc->dist_node->nodes[0].port;
    packet.timestamp = get_current_time_ms();

    size_t sent;
    int ret = idcu_network_socket_sendto(&disc->socket, &packet, sizeof(packet),
                                          "255.255.255.255", IDCU_DISCOVERY_PORT, &sent);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_WARN("Failed to send discovery broadcast");
    }

    return ret;
}

static int process_discovery_packet(idcu_NodeDiscovery* disc, const idcu_DiscoveryPacket* packet) {
    if (packet->magic != IDCU_DISCOVERY_MAGIC) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (packet->version != IDCU_DISCOVERY_VERSION) {
        IDCU_LOG_WARN("Discovery packet version mismatch: %d vs %d", packet->version, IDCU_DISCOVERY_VERSION);
        return IDCU_ERR_INVALID_PARAM;
    }

    if (packet->node_id == disc->dist_node->self_node_id) {
        return IDCU_ERR_OK;
    }

    idcu_NodeInfo* existing_node = idcu_distributed_node_find_node(disc->dist_node, packet->node_id);

    if (existing_node) {
        idcu_distributed_node_update_heartbeat(disc->dist_node, packet->node_id);
    } else {
        idcu_NodeInfo new_node;
        memset(&new_node, 0, sizeof(new_node));
        new_node.node_id = packet->node_id;
        strncpy(new_node.name, packet->name, IDCU_NODE_NAME_MAX - 1);
        strncpy(new_node.address, packet->address, IDCU_NODE_ADDR_MAX - 1);
        new_node.port = packet->port;
        new_node.status = IDCU_NODE_STATUS_ONLINE;
        new_node.last_heartbeat = get_current_time_ms();

        int ret = idcu_distributed_node_add_node(disc->dist_node, new_node.node_id,
                                                  new_node.name, new_node.address, new_node.port);
        if (ret == IDCU_ERR_OK && disc->discovered_handler) {
            disc->discovered_handler(disc->discovered_user_data, &new_node);
            IDCU_LOG_INFO("Discovered new node: %s (ID: %llu)", new_node.name, (unsigned long long)new_node.node_id);
        }
    }

    return IDCU_ERR_OK;
}

static void check_for_lost_nodes(idcu_NodeDiscovery* disc) {
    uint64_t current_time = get_current_time_ms();

    for (int i = 1; i < disc->dist_node->node_count; i++) {
        idcu_NodeInfo* node = &disc->dist_node->nodes[i];
        if (current_time - node->last_heartbeat > IDCU_DISCOVERY_TIMEOUT_MS) {
            if (disc->lost_handler) {
                disc->lost_handler(disc->lost_user_data, node);
            }
            IDCU_LOG_INFO("Node lost: %s (ID: %llu)", node->name, (unsigned long long)node->node_id);
            idcu_distributed_node_remove_node(disc->dist_node, node->node_id);
            i--;
        }
    }
}

int idcu_node_discovery_poll(idcu_NodeDiscovery* disc) {
    if (!disc || !disc->running) return IDCU_ERR_INVALID_PARAM;

    uint64_t current_time = get_current_time_ms();

    if (current_time - disc->last_broadcast_time >= IDCU_DISCOVERY_BROADCAST_INTERVAL_MS) {
        send_discovery_packet(disc);
        disc->last_broadcast_time = current_time;
    }

    uint8_t buffer[IDCU_DISCOVERY_MAX_PACKET_SIZE];
    char sender_addr[IDCU_ADDR_MAX];
    uint16_t sender_port;
    size_t received;

    int ret = idcu_network_socket_recvfrom(&disc->socket, buffer, sizeof(buffer),
                                            sender_addr, sizeof(sender_addr), &sender_port, &received);
    if (ret == IDCU_ERR_OK && received >= sizeof(idcu_DiscoveryPacket)) {
        process_discovery_packet(disc, (const idcu_DiscoveryPacket*)buffer);
    }

    check_for_lost_nodes(disc);

    return IDCU_ERR_OK;
}
