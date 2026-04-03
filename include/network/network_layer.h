#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H

#include "common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define NET_PROTO_TCP 0
#define NET_PROTO_UDP 1
#define ADDR_MAX 64

typedef struct {
    int protocol;
    int fd;
    char local_addr[ADDR_MAX];
    uint16_t local_port;
    char remote_addr[ADDR_MAX];
    uint16_t remote_port;
    int connected;
} NetworkSocket;

typedef struct {
    int protocol;
    int fd;
    char bind_address[ADDR_MAX];
    uint16_t bind_port;
    int listening;
} NetworkServer;

int network_socket_create(NetworkSocket* sock, int protocol);
void network_socket_destroy(NetworkSocket* sock);
int network_server_create(NetworkServer* server, int protocol, const char* address, uint16_t port);
void network_server_destroy(NetworkServer* server);

#endif
