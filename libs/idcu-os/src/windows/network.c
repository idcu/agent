#include "idcu/os/os.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

static int g_winsock_initialized = 0;

int idcu_sock_init(void)
{
    if (g_winsock_initialized) {
        return 0;
    }
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        return -1;
    }
    g_winsock_initialized = 1;
    return 0;
}

void idcu_sock_cleanup(void)
{
    if (g_winsock_initialized) {
        WSACleanup();
        g_winsock_initialized = 0;
    }
}

int idcu_sock_close(int sock)
{
    return closesocket(sock) == 0 ? 0 : -1;
}

int idcu_sock_bind(int sock, const char* addr, uint16_t port)
{
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    
    if (addr) {
        if (inet_pton(AF_INET, addr, &saddr.sin_addr) <= 0) {
            return -1;
        }
    } else {
        saddr.sin_addr.s_addr = INADDR_ANY;
    }
    
    return bind(sock, (struct sockaddr*)&saddr, sizeof(saddr)) == 0 ? 0 : -1;
}

int idcu_sock_listen(int sock, int backlog)
{
    return listen(sock, backlog) == 0 ? 0 : -1;
}

int idcu_sock_accept(int sock, char* client_addr, size_t addr_len, uint16_t* client_port)
{
    struct sockaddr_in saddr;
    int saddr_len = sizeof(saddr);
    SOCKET client_sock = accept(sock, (struct sockaddr*)&saddr, &saddr_len);
    if (client_sock == INVALID_SOCKET) {
        return -1;
    }
    
    if (client_addr && addr_len > 0) {
        inet_ntop(AF_INET, &saddr.sin_addr, client_addr, (int)addr_len);
    }
    if (client_port) {
        *client_port = ntohs(saddr.sin_port);
    }
    
    return (int)client_sock;
}

int idcu_sock_connect(int sock, const char* addr, uint16_t port)
{
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, addr, &saddr.sin_addr) <= 0) {
        return -1;
    }
    
    return connect(sock, (struct sockaddr*)&saddr, sizeof(saddr)) == 0 ? 0 : -1;
}

int idcu_sock_send(int sock, const void* data, size_t len)
{
    int sent = send(sock, (const char*)data, (int)len, 0);
    return sent != SOCKET_ERROR ? sent : -1;
}

int idcu_sock_recv(int sock, void* data, size_t len)
{
    int received = recv(sock, (char*)data, (int)len, 0);
    return received != SOCKET_ERROR ? received : -1;
}

int idcu_sock_set_nonblocking(int sock, bool nonblocking)
{
    u_long mode = nonblocking ? 1 : 0;
    return ioctlsocket(sock, FIONBIO, &mode) == 0 ? 0 : -1;
}

int idcu_poll(idcu_pollfd_t* fds, size_t nfds, int timeout_ms)
{
    WSAPOLLFD* wsa_fds = (WSAPOLLFD*)malloc(nfds * sizeof(WSAPOLLFD));
    if (!wsa_fds) {
        return -1;
    }
    
    for (size_t i = 0; i < nfds; i++) {
        wsa_fds[i].fd = fds[i].fd;
        wsa_fds[i].events = fds[i].events;
        wsa_fds[i].revents = 0;
    }
    
    int result = WSAPoll(wsa_fds, (ULONG)nfds, timeout_ms);
    
    for (size_t i = 0; i < nfds; i++) {
        fds[i].revents = wsa_fds[i].revents;
    }
    
    free(wsa_fds);
    return result != SOCKET_ERROR ? result : -1;
}
