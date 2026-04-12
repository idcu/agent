#include "idcu/os/os.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int idcu_sock_init(void)
{
    return 0;
}

void idcu_sock_cleanup(void)
{
}

int idcu_sock_close(int sock)
{
    return close(sock) == 0 ? 0 : -1;
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
    socklen_t saddr_len = sizeof(saddr);
    int client_sock = accept(sock, (struct sockaddr*)&saddr, &saddr_len);
    if (client_sock < 0) {
        return -1;
    }
    
    if (client_addr && addr_len > 0) {
        inet_ntop(AF_INET, &saddr.sin_addr, client_addr, addr_len);
    }
    if (client_port) {
        *client_port = ntohs(saddr.sin_port);
    }
    
    return client_sock;
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
    ssize_t sent = send(sock, data, len, 0);
    return sent >= 0 ? (int)sent : -1;
}

int idcu_sock_recv(int sock, void* data, size_t len)
{
    ssize_t received = recv(sock, data, len, 0);
    return received >= 0 ? (int)received : -1;
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
    
    return fcntl(sock, F_SETFL, flags) == 0 ? 0 : -1;
}

int idcu_poll(idcu_pollfd_t* fds, size_t nfds, int timeout_ms)
{
    struct pollfd* poll_fds = (struct pollfd*)malloc(nfds * sizeof(struct pollfd));
    if (!poll_fds) {
        return -1;
    }
    
    for (size_t i = 0; i < nfds; i++) {
        poll_fds[i].fd = fds[i].fd;
        poll_fds[i].events = fds[i].events;
        poll_fds[i].revents = 0;
    }
    
    int result = poll(poll_fds, nfds, timeout_ms);
    
    for (size_t i = 0; i < nfds; i++) {
        fds[i].revents = poll_fds[i].revents;
    }
    
    free(poll_fds);
    return result;
}
