#include "idcu/os/os.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int idcu_sock_close(int sock)
{
    return close(sock);
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
    socklen_t sa_len = sizeof(client_sa);
    int client_sock = accept(sock, (struct sockaddr*)&client_sa, &sa_len);
    if (client_sock >= 0 && client_addr && addr_len > 0) {
        strncpy(client_addr, inet_ntoa(client_sa.sin_addr), addr_len - 1);
        client_addr[addr_len - 1] = '\0';
    }
    if (client_port) {
        *client_port = ntohs(client_sa.sin_port);
    }
    return client_sock;
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
    return (int)send(sock, data, len, 0);
}

int idcu_sock_recv(int sock, void* data, size_t len)
{
    return (int)recv(sock, data, len, 0);
}

int idcu_sock_set_nonblocking(int sock, bool nonblocking)
{
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }
    if (nonblocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    return fcntl(sock, F_SETFL, flags);
}
