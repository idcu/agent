#ifndef IDCU_NETWORK_NETWORK_LAYER_H
#define IDCU_NETWORK_NETWORK_LAYER_H

#include "common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_NET_PROTO_TCP 0
#define IDCU_NET_PROTO_UDP 1
#define IDCU_ADDR_MAX 64
#define IDCU_NET_BACKLOG 10

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET idcu_socket_t;
#define IDCU_INVALID_SOCKET INVALID_SOCKET
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
typedef int idcu_socket_t;
#define IDCU_INVALID_SOCKET (-1)
#endif

typedef struct {
    int protocol;
    idcu_socket_t fd;
    char local_addr[IDCU_ADDR_MAX];
    uint16_t local_port;
    char remote_addr[IDCU_ADDR_MAX];
    uint16_t remote_port;
    int connected;
} idcu_NetworkSocket;

typedef struct {
    int protocol;
    idcu_socket_t fd;
    char bind_address[IDCU_ADDR_MAX];
    uint16_t bind_port;
    int listening;
} idcu_NetworkServer;

int idcu_network_init(void);
void idcu_network_cleanup(void);

int idcu_network_socket_create(idcu_NetworkSocket* sock, int protocol);
void idcu_network_socket_destroy(idcu_NetworkSocket* sock);
int idcu_network_socket_connect(idcu_NetworkSocket* sock, const char* address, uint16_t port);
int idcu_network_socket_bind(idcu_NetworkSocket* sock, const char* address, uint16_t port);
int idcu_network_socket_send(idcu_NetworkSocket* sock, const void* data, size_t len, size_t* sent);
int idcu_network_socket_recv(idcu_NetworkSocket* sock, void* data, size_t len, size_t* received);
int idcu_network_socket_sendto(idcu_NetworkSocket* sock, const void* data, size_t len, 
                               const char* address, uint16_t port, size_t* sent);
int idcu_network_socket_recvfrom(idcu_NetworkSocket* sock, void* data, size_t len,
                                 char* address, size_t addr_len, uint16_t* port, size_t* received);
int idcu_network_socket_close(idcu_NetworkSocket* sock);

int idcu_network_server_create(idcu_NetworkServer* server, int protocol, const char* address, uint16_t port);
void idcu_network_server_destroy(idcu_NetworkServer* server);
int idcu_network_server_listen(idcu_NetworkServer* server);
int idcu_network_server_accept(idcu_NetworkServer* server, idcu_NetworkSocket* client_sock);
int idcu_network_server_close(idcu_NetworkServer* server);

#endif // IDCU_NETWORK_NETWORK_LAYER_H
