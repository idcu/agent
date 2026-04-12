#include "idcu/os.h"
#include <winsock2.h>
#include <ws2tcpip.h>

int idcu_sock_close(int sock)
{
    return closesocket(sock);
}

int idcu_sock_bind(int sock, const char* addr, uint16_t port)
{
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = addr ? inet_addr(addr) : INADDR_ANY;
    return bind(sock, (struct sockaddr*)&sa, sizeof(sa));
}

int idcu_sock_listen(int sock, int backlog)
{
    return listen(sock, backlog);
}

int idcu_sock_accept(int sock, char* client_addr, size_t addr_len, uint16_t* client_port)
{
    struct sockaddr_in client_sa;
    int sa_len = sizeof(client_sa);
    SOCKET client_sock = accept(sock, (struct sockaddr*)&client_sa, &sa_len);
    if (client_sock != INVALID_SOCKET && client_addr && addr_len > 0) {
        strncpy(client_addr, inet_ntoa(client_sa.sin_addr), addr_len - 1);
        client_addr[addr_len - 1] = '\0';
    }
    if (client_port) {
        *client_port = ntohs(client_sa.sin_port);
    }
    return (int)client_sock;
}

int idcu_sock_connect(int sock, const char* addr, uint16_t port)
{
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = inet_addr(addr);
    return connect(sock, (struct sockaddr*)&sa, sizeof(sa));
}

int idcu_sock_send(int sock, const void* data, size_t len)
{
    return (int)send(sock, (const char*)data, (int)len, 0);
}

int idcu_sock_recv(int sock, void* data, size_t len)
{
    return (int)recv(sock, (char*)data, (int)len, 0);
}

int idcu_sock_set_nonblocking(int sock, bool nonblocking)
{
    u_long mode = nonblocking ? 1 : 0;
    return ioctlsocket(sock, FIONBIO, &mode);
}
