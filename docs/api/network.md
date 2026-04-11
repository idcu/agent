# idcu-network API Documentation

Network programming library for TCP and UDP sockets.

## Library Initialization

```c
int idcu_network_init(void);
void idcu_network_cleanup(void);
```

## Network Address

```c
typedef struct {
    idcu_NetAddrType type;
    char ip[IDCU_NET_IP_MAX];
    uint16_t port;
} idcu_NetAddress;
```

### Address Functions

```c
int idcu_net_address_init(idcu_NetAddress* addr, idcu_NetAddrType type, const char* ip, uint16_t port);
int idcu_net_address_resolve(idcu_NetAddress* addr, const char* hostname, uint16_t port);
int idcu_net_address_to_string(const idcu_NetAddress* addr, char* buffer, size_t buffer_size);
```

## TCP Socket

```c
typedef struct idcu_TcpSocket idcu_TcpSocket;
```

### TCP Socket Functions

```c
int idcu_tcp_socket_init(idcu_TcpSocket* sock);
void idcu_tcp_socket_destroy(idcu_TcpSocket* sock);

int idcu_tcp_socket_connect(idcu_TcpSocket* sock, const idcu_NetAddress* addr);
int idcu_tcp_socket_connect_timeout(idcu_TcpSocket* sock, const idcu_NetAddress* addr, int timeout_ms);
void idcu_tcp_socket_disconnect(idcu_TcpSocket* sock);
int idcu_tcp_socket_is_connected(const idcu_TcpSocket* sock);

int idcu_tcp_socket_send(idcu_TcpSocket* sock, const void* data, size_t length, size_t* out_sent);
int idcu_tcp_socket_send_all(idcu_TcpSocket* sock, const void* data, size_t length);
int idcu_tcp_socket_recv(idcu_TcpSocket* sock, void* buffer, size_t buffer_size, size_t* out_received);

int idcu_tcp_socket_set_timeout(idcu_TcpSocket* sock, int timeout_ms);
int idcu_tcp_socket_set_blocking(idcu_TcpSocket* sock, int blocking);
```

## TCP Server

```c
typedef struct idcu_TcpServer idcu_TcpServer;
```

### TCP Server Functions

```c
int idcu_tcp_server_init(idcu_TcpServer* server);
void idcu_tcp_server_destroy(idcu_TcpServer* server);

int idcu_tcp_server_listen(idcu_TcpServer* server, const idcu_NetAddress* addr, int backlog);
void idcu_tcp_server_stop(idcu_TcpServer* server);

int idcu_tcp_server_accept(idcu_TcpServer* server, idcu_TcpSocket* out_client);
int idcu_tcp_server_accept_timeout(idcu_TcpServer* server, idcu_TcpSocket* out_client, int timeout_ms);
```

## UDP Socket

```c
typedef struct idcu_UdpSocket idcu_UdpSocket;
```

### UDP Socket Functions

```c
int idcu_udp_socket_init(idcu_UdpSocket* sock);
void idcu_udp_socket_destroy(idcu_UdpSocket* sock);

int idcu_udp_socket_bind(idcu_UdpSocket* sock, const idcu_NetAddress* addr);

int idcu_udp_socket_send(idcu_UdpSocket* sock, const idcu_NetAddress* addr, const void* data, size_t length);
int idcu_udp_socket_recv(idcu_UdpSocket* sock, idcu_NetAddress* out_from, void* buffer, size_t buffer_size, size_t* out_received);

int idcu_udp_socket_set_timeout(idcu_UdpSocket* sock, int timeout_ms);
```

## TCP Client Example

```c
#include <idcu/network/network.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    idcu_network_init();
    
    idcu_TcpSocket sock;
    idcu_tcp_socket_init(&sock);
    
    idcu_NetAddress addr;
    idcu_net_address_resolve(&addr, "example.com", 80);
    
    idcu_tcp_socket_connect(&sock, &addr);
    
    const char* request = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
    idcu_tcp_socket_send_all(&sock, request, strlen(request));
    
    char buffer[4096];
    size_t received;
    idcu_tcp_socket_recv(&sock, buffer, sizeof(buffer), &received);
    buffer[received] = '\0';
    printf("%s\n", buffer);
    
    idcu_tcp_socket_destroy(&sock);
    idcu_network_cleanup();
    return 0;
}
```
