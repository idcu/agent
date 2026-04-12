#ifndef IDCU_NETWORK_NETWORK_H
#define IDCU_NETWORK_NETWORK_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/network/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int  idcu_network_init(void);
void idcu_network_cleanup(void);

int  idcu_net_address_init(idcu_NetAddress* addr, idcu_NetAddrType type, const char* ip, uint16_t port);
int  idcu_net_address_resolve(idcu_NetAddress* addr, const char* hostname, uint16_t port);
int  idcu_net_address_to_string(const idcu_NetAddress* addr, char* buffer, size_t buffer_size);

int  idcu_tcp_socket_init(idcu_TcpSocket* sock);
void idcu_tcp_socket_destroy(idcu_TcpSocket* sock);
int  idcu_tcp_socket_connect(idcu_TcpSocket* sock, const idcu_NetAddress* addr);
int  idcu_tcp_socket_connect_timeout(idcu_TcpSocket* sock, const idcu_NetAddress* addr, int timeout_ms);
int  idcu_tcp_socket_send(idcu_TcpSocket* sock, const void* data, size_t len, size_t* sent);
int  idcu_tcp_socket_send_all(idcu_TcpSocket* sock, const void* data, size_t len);
int  idcu_tcp_socket_recv(idcu_TcpSocket* sock, void* buffer, size_t len, size_t* received);
int  idcu_tcp_socket_set_timeout(idcu_TcpSocket* sock, int timeout_ms);
int  idcu_tcp_socket_set_blocking(idcu_TcpSocket* sock, int blocking);
void idcu_tcp_socket_disconnect(idcu_TcpSocket* sock);

int  idcu_tcp_server_init(idcu_TcpServer* server);
void idcu_tcp_server_destroy(idcu_TcpServer* server);
int  idcu_tcp_server_listen(idcu_TcpServer* server, const idcu_NetAddress* addr, int backlog);
int  idcu_tcp_server_accept(idcu_TcpServer* server, idcu_TcpSocket* client);
int  idcu_tcp_server_accept_timeout(idcu_TcpServer* server, idcu_TcpSocket* client, int timeout_ms);

int  idcu_udp_socket_init(idcu_UdpSocket* sock);
void idcu_udp_socket_destroy(idcu_UdpSocket* sock);
int  idcu_udp_socket_bind(idcu_UdpSocket* sock, const idcu_NetAddress* addr);
int  idcu_udp_socket_send(idcu_UdpSocket* sock, const idcu_NetAddress* dest, const void* data, size_t len);
int  idcu_udp_socket_recv(idcu_UdpSocket* sock, idcu_NetAddress* source, void* buffer, size_t len, size_t* received);
int  idcu_udp_socket_set_timeout(idcu_UdpSocket* sock, int timeout_ms);

int  idcu_net_poll(idcu_NetPollFd* fds, size_t nfds, int timeout_ms);
int  idcu_net_select(idcu_Socket max_fd, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
