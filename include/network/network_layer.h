#ifndef IDCU_NETWORK_NETWORK_LAYER_H
#define IDCU_NETWORK_NETWORK_LAYER_H

#include "common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_NET_PROTO_TCP 0
#define IDCU_NET_PROTO_UDP 1
#define IDCU_ADDR_MAX 64

typedef struct {
    int protocol;
    int fd;
    char local_addr[IDCU_ADDR_MAX];
    uint16_t local_port;
    char remote_addr[IDCU_ADDR_MAX];
    uint16_t remote_port;
    int connected;
} idcu_NetworkSocket;

typedef struct {
    int protocol;
    int fd;
    char bind_address[IDCU_ADDR_MAX];
    uint16_t bind_port;
    int listening;
} idcu_NetworkServer;

int idcu_network_socket_create(idcu_NetworkSocket* sock, int protocol);
void idcu_network_socket_destroy(idcu_NetworkSocket* sock);
int idcu_network_server_create(idcu_NetworkServer* server, int protocol, const char* address, uint16_t port);
void idcu_network_server_destroy(idcu_NetworkServer* server);

#endif // IDCU_NETWORK_NETWORK_LAYER_H
