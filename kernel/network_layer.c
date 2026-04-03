#include "network_layer.h"
#include <string.h>

int network_socket_create(NetworkSocket* sock, int protocol) {
    if (!sock) return ERR_INVALID_PARAM;
    if (protocol != NET_PROTO_TCP && protocol != NET_PROTO_UDP) return ERR_INVALID_PARAM;
    
    memset(sock, 0, sizeof(NetworkSocket));
    sock->protocol = protocol;
    sock->fd = -1;
    sock->connected = 0;
    
    return ERR_OK;
}

void network_socket_destroy(NetworkSocket* sock) {
    if (!sock) return;
    memset(sock, 0, sizeof(NetworkSocket));
}

int network_server_create(NetworkServer* server, int protocol, const char* address, uint16_t port) {
    if (!server || !address) return ERR_INVALID_PARAM;
    if (protocol != NET_PROTO_TCP && protocol != NET_PROTO_UDP) return ERR_INVALID_PARAM;
    
    memset(server, 0, sizeof(NetworkServer));
    server->protocol = protocol;
    server->fd = -1;
    strncpy(server->bind_address, address, ADDR_MAX - 1);
    server->bind_address[ADDR_MAX - 1] = '\0';
    server->bind_port = port;
    server->listening = 0;
    
    return ERR_OK;
}

void network_server_destroy(NetworkServer* server) {
    if (!server) return;
    memset(server, 0, sizeof(NetworkServer));
}
