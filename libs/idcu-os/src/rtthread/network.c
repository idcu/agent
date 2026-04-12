#include "idcu/os/os.h"
#include <rtthread.h>
#include <sal/socket.h>

int idcu_sock_init(void)
{
    return 0;
}

int idcu_sock_socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

int idcu_sock_close(int sock)
{
    return closesocket(sock);
}

int idcu_sock_bind(int sock, const struct sockaddr* addr, socklen_t addrlen)
{
    return bind(sock, addr, addrlen);
}

int idcu_sock_listen(int sock, int backlog)
{
    return listen(sock, backlog);
}

int idcu_sock_accept(int sock, struct sockaddr* addr, socklen_t* addrlen)
{
    return accept(sock, addr, addrlen);
}

int idcu_sock_connect(int sock, const struct sockaddr* addr, socklen_t addrlen)
{
    return connect(sock, addr, addrlen);
}

ssize_t idcu_sock_send(int sock, const void* buf, size_t len, int flags)
{
    return send(sock, buf, len, flags);
}

ssize_t idcu_sock_recv(int sock, void* buf, size_t len, int flags)
{
    return recv(sock, buf, len, flags);
}

int idcu_sock_setsockopt(int sock, int level, int optname, const void* optval, socklen_t optlen)
{
    return setsockopt(sock, level, optname, optval, optlen);
}

int idcu_sock_getsockopt(int sock, int level, int optname, void* optval, socklen_t* optlen)
{
    return getsockopt(sock, level, optname, optval, optlen);
}
