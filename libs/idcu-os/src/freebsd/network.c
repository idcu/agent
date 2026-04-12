#include "idcu/os/os.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/event.h>
#include <sys/time.h>
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
    int kq = kqueue();
    if (kq == -1) {
        return -1;
    }
    
    struct kevent* changelist = (struct kevent*)malloc(nfds * sizeof(struct kevent));
    struct kevent* eventlist = (struct kevent*)malloc(nfds * sizeof(struct kevent));
    
    if (!changelist || !eventlist) {
        free(changelist);
        free(eventlist);
        close(kq);
        return -1;
    }
    
    int change_count = 0;
    for (size_t i = 0; i < nfds; i++) {
        if (fds[i].events & POLLIN) {
            EV_SET(&changelist[change_count], fds[i].fd, EVFILT_READ, EV_ADD, 0, 0, (void*)(uintptr_t)i);
            change_count++;
        }
        if (fds[i].events & POLLOUT) {
            EV_SET(&changelist[change_count], fds[i].fd, EVFILT_WRITE, EV_ADD, 0, 0, (void*)(uintptr_t)i);
            change_count++;
        }
        fds[i].revents = 0;
    }
    
    struct timespec ts;
    struct timespec* ts_ptr = NULL;
    if (timeout_ms >= 0) {
        ts.tv_sec = timeout_ms / 1000;
        ts.tv_nsec = (timeout_ms % 1000) * 1000000;
        ts_ptr = &ts;
    }
    
    int result = kevent(kq, changelist, change_count, eventlist, (int)nfds, ts_ptr);
    
    if (result > 0) {
        for (int i = 0; i < result; i++) {
            size_t idx = (size_t)(uintptr_t)eventlist[i].udata;
            if (eventlist[i].filter == EVFILT_READ) {
                fds[idx].revents |= POLLIN;
            }
            if (eventlist[i].filter == EVFILT_WRITE) {
                fds[idx].revents |= POLLOUT;
            }
            if (eventlist[i].flags & EV_ERROR) {
                fds[idx].revents |= POLLERR;
            }
        }
    }
    
    free(changelist);
    free(eventlist);
    close(kq);
    
    return result;
}
