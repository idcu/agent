#ifndef IDCU_NETWORK_TYPES_H
#define IDCU_NETWORK_TYPES_H

#include <idcu/common/config.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET idcu_Socket;
    #define IDCU_INVALID_SOCKET INVALID_SOCKET
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    typedef int idcu_Socket;
    #define IDCU_INVALID_SOCKET (-1)
#endif

#define IDCU_NET_HOST_MAX        256
#define IDCU_NET_BUFFER_MAX      65536
#define IDCU_NET_DEFAULT_TIMEOUT 30000

typedef enum {
    IDCU_NET_PROTO_TCP = 0,
    IDCU_NET_PROTO_UDP
} idcu_NetProtocol;

typedef enum {
    IDCU_NET_ADDR_IPV4 = 0,
    IDCU_NET_ADDR_IPV6
} idcu_NetAddrType;

typedef struct {
    idcu_NetAddrType type;
    union {
        struct in_addr  v4;
        struct in6_addr v6;
    } addr;
    uint16_t port;
} idcu_NetAddress;

typedef struct {
    idcu_Socket socket;
    idcu_NetProtocol protocol;
    idcu_NetAddress local_addr;
    idcu_NetAddress remote_addr;
    int is_connected;
    int is_blocking;
    int timeout_ms;
    char host[IDCU_NET_HOST_MAX];
} idcu_TcpSocket;

typedef struct {
    idcu_Socket socket;
    idcu_NetAddress local_addr;
    int is_bound;
    int timeout_ms;
} idcu_UdpSocket;

typedef struct {
    idcu_Socket listen_socket;
    idcu_NetAddress listen_addr;
    int is_listening;
    int backlog;
} idcu_TcpServer;

typedef enum {
    IDCU_NET_POLL_IN  = 0x01,
    IDCU_NET_POLL_OUT = 0x02,
    IDCU_NET_POLL_ERR = 0x04,
    IDCU_NET_POLL_HUP = 0x08
} idcu_NetPollEvent;

typedef struct {
    idcu_Socket socket;
    int events;
    int revents;
} idcu_NetPollFd;

#ifdef __cplusplus
}
#endif

#endif
