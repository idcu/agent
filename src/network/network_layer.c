#include "network/network_layer.h"
#include <string.h>

int idcu_network_socket_create(idcu_NetworkSocket* sock, int protocol) {
    if (!sock) return IDCU_ERR_INVALID_PARAM;
    if (protocol != IDCU_NET_PROTO_TCP && protocol != IDCU_NET_PROTO_UDP) return IDCU_ERR_INVALID_PARAM;
    
    memset(sock, 0, sizeof(idcu_NetworkSocket));
    sock->protocol = protocol;
    sock->fd = -1;
    sock->connected = 0;
    
    return IDCU_ERR_OK;
}

void idcu_network_socket_destroy(idcu_NetworkSocket* sock) {
    if (!sock) return;
    memset(sock, 0, sizeof(idcu_NetworkSocket));
}

int idcu_network_server_create(idcu_NetworkServer* server, int protocol, const char* address, uint16_t port) {
    if (!server || !address) return IDCU_ERR_INVALID_PARAM;
    if (protocol != IDCU_NET_PROTO_TCP && protocol != IDCU_NET_PROTO_UDP) return IDCU_ERR_INVALID_PARAM;
    
    memset(server, 0, sizeof(idcu_NetworkServer));
    server->protocol = protocol;
    server->fd = -1;
    strncpy(server->bind_address, address, IDCU_ADDR_MAX - 1);
    server->bind_address[IDCU_ADDR_MAX - 1] = '\0';
    server->bind_port = port;
    server->listening = 0;
    
    return IDCU_ERR_OK;
}

void idcu_network_server_destroy(idcu_NetworkServer* server) {
    if (!server) return;
    memset(server, 0, sizeof(idcu_NetworkServer));
}
