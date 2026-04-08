#ifndef IDCU_NETWORK_NETWORK_LAYER_H
#define IDCU_NETWORK_NETWORK_LAYER_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "idcu/common/error_code.h"

#include <stddef.h>
#include <stdint.h>

#define IDCU_NET_PROTO_TCP 0
#define IDCU_NET_PROTO_UDP 1
#define IDCU_ADDR_MAX      64
#define IDCU_NET_BACKLOG   10

#define IDCU_NET_DEFAULT_TIMEOUT_MS         30000
#define IDCU_NET_DEFAULT_MAX_RETRIES        3
#define IDCU_NET_DEFAULT_RETRY_DELAY_MS     1000
#define IDCU_NET_DEFAULT_RECONNECT_DELAY_MS 2000

#ifdef _WIN32
typedef SOCKET idcu_socket_t;
#define IDCU_INVALID_SOCKET INVALID_SOCKET
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int idcu_socket_t;
#define IDCU_INVALID_SOCKET (-1)
#endif

typedef struct
{
    uint32_t connect_timeout_ms;
    uint32_t send_timeout_ms;
    uint32_t recv_timeout_ms;
    uint32_t max_retries;
    uint32_t retry_delay_ms;
    uint32_t reconnect_delay_ms;
    uint32_t retry_count;
} idcu_NetworkConfig;

typedef struct
{
    int                protocol;
    idcu_socket_t      fd;
    char               local_addr[IDCU_ADDR_MAX];
    uint16_t           local_port;
    char               remote_addr[IDCU_ADDR_MAX];
    uint16_t           remote_port;
    int                connected;
    idcu_NetworkConfig config;
} idcu_NetworkSocket;

typedef struct
{
    int                protocol;
    idcu_socket_t      fd;
    char               bind_address[IDCU_ADDR_MAX];
    uint16_t           bind_port;
    int                listening;
    idcu_NetworkConfig config;
} idcu_NetworkServer;

int  idcu_network_init(void);
void idcu_network_cleanup(void);

int  idcu_network_socket_create(idcu_NetworkSocket* sock, int protocol);
void idcu_network_socket_destroy(idcu_NetworkSocket* sock);
int  idcu_network_socket_connect(idcu_NetworkSocket* sock, const char* address, uint16_t port);
int  idcu_network_socket_bind(idcu_NetworkSocket* sock, const char* address, uint16_t port);
int  idcu_network_socket_send(idcu_NetworkSocket* sock, const void* data, size_t len, size_t* sent);
int  idcu_network_socket_recv(idcu_NetworkSocket* sock, void* data, size_t len, size_t* received);
int  idcu_network_socket_sendto(idcu_NetworkSocket* sock, const void* data, size_t len,
                                const char* address, uint16_t port, size_t* sent);
int  idcu_network_socket_recvfrom(idcu_NetworkSocket* sock, void* data, size_t len, char* address,
                                  size_t addr_len, uint16_t* port, size_t* received);
int  idcu_network_socket_close(idcu_NetworkSocket* sock);

int  idcu_network_server_create(idcu_NetworkServer* server, int protocol, const char* address,
                                uint16_t port);
void idcu_network_server_destroy(idcu_NetworkServer* server);
int  idcu_network_server_listen(idcu_NetworkServer* server);
int  idcu_network_server_accept(idcu_NetworkServer* server, idcu_NetworkSocket* client_sock);
int  idcu_network_server_close(idcu_NetworkServer* server);

int idcu_network_socket_set_config(idcu_NetworkSocket* sock, const idcu_NetworkConfig* config);
int idcu_network_socket_get_config(idcu_NetworkSocket* sock, idcu_NetworkConfig* config);
int idcu_network_socket_set_timeout(idcu_NetworkSocket* sock, uint32_t timeout_ms);
int idcu_network_socket_reconnect(idcu_NetworkSocket* sock);
int idcu_network_socket_send_with_retry(idcu_NetworkSocket* sock, const void* data, size_t len,
                                        size_t* sent);
int idcu_network_socket_recv_with_retry(idcu_NetworkSocket* sock, void* data, size_t len,
                                        size_t* received);

#endif  // IDCU_NETWORK_NETWORK_LAYER_H
