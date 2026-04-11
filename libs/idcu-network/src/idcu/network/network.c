#include <idcu/network/network.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
    static int g_winsock_initialized = 0;
#else
    #include <fcntl.h>
    #include <errno.h>
    #include <unistd.h>
    #include <poll.h>
#endif

static int g_network_initialized = 0;

int idcu_network_init(void) {
    if (g_network_initialized) {
        return IDCU_ERR_OK;
    }
    
#ifdef _WIN32
    WSADATA wsa_data;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (ret != 0) {
        return IDCU_ERR_UNKNOWN;
    }
    g_winsock_initialized = 1;
#endif
    
    g_network_initialized = 1;
    return IDCU_ERR_OK;
}

void idcu_network_cleanup(void) {
    if (!g_network_initialized) {
        return;
    }
    
#ifdef _WIN32
    if (g_winsock_initialized) {
        WSACleanup();
        g_winsock_initialized = 0;
    }
#endif
    
    g_network_initialized = 0;
}

int idcu_net_address_init(idcu_NetAddress* addr, idcu_NetAddrType type, const char* ip, uint16_t port) {
    if (!addr) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(addr, 0, sizeof(*addr));
    addr->type = type;
    addr->port = port;
    
    if (ip) {
        if (type == IDCU_NET_ADDR_IPV4) {
            if (inet_pton(AF_INET, ip, &addr->addr.v4) <= 0) {
                return IDCU_ERR_INVALID_ARG;
            }
        } else {
            if (inet_pton(AF_INET6, ip, &addr->addr.v6) <= 0) {
                return IDCU_ERR_INVALID_ARG;
            }
        }
    } else {
        if (type == IDCU_NET_ADDR_IPV4) {
            addr->addr.v4.s_addr = INADDR_ANY;
        } else {
            addr->addr.v6 = in6addr_any;
        }
    }
    
    return IDCU_ERR_OK;
}

int idcu_net_address_resolve(idcu_NetAddress* addr, const char* hostname, uint16_t port) {
    if (!addr || !hostname) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    char port_str[8];
    snprintf(port_str, sizeof(port_str), "%u", port);
    
    int ret = getaddrinfo(hostname, port_str, &hints, &result);
    if (ret != 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    memset(addr, 0, sizeof(*addr));
    addr->port = port;
    
    if (result->ai_family == AF_INET) {
        addr->type = IDCU_NET_ADDR_IPV4;
        memcpy(&addr->addr.v4, &((struct sockaddr_in*)result->ai_addr)->sin_addr, sizeof(addr->addr.v4));
    } else {
        addr->type = IDCU_NET_ADDR_IPV6;
        memcpy(&addr->addr.v6, &((struct sockaddr_in6*)result->ai_addr)->sin6_addr, sizeof(addr->addr.v6));
    }
    
    freeaddrinfo(result);
    return IDCU_ERR_OK;
}

int idcu_net_address_to_string(const idcu_NetAddress* addr, char* buffer, size_t buffer_size) {
    if (!addr || !buffer) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (addr->type == IDCU_NET_ADDR_IPV4) {
        if (inet_ntop(AF_INET, &addr->addr.v4, buffer, (int)buffer_size) == NULL) {
            return IDCU_ERR_UNKNOWN;
        }
    } else {
        if (inet_ntop(AF_INET6, &addr->addr.v6, buffer, (int)buffer_size) == NULL) {
            return IDCU_ERR_UNKNOWN;
        }
    }
    
    return IDCU_ERR_OK;
}

int idcu_tcp_socket_init(idcu_TcpSocket* sock) {
    if (!sock) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(sock, 0, sizeof(*sock));
    sock->socket = IDCU_INVALID_SOCKET;
    sock->protocol = IDCU_NET_PROTO_TCP;
    sock->timeout_ms = IDCU_NET_DEFAULT_TIMEOUT;
    sock->is_blocking = 1;
    
    return IDCU_ERR_OK;
}

void idcu_tcp_socket_destroy(idcu_TcpSocket* sock) {
    if (!sock) {
        return;
    }
    
    idcu_tcp_socket_disconnect(sock);
}

static int set_socket_timeout(idcu_Socket sock, int timeout_ms) {
    if (timeout_ms <= 0) {
        return IDCU_ERR_OK;
    }
    
#ifdef _WIN32
    DWORD timeout = timeout_ms;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
#else
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
    
    return IDCU_ERR_OK;
}

static int set_socket_blocking(idcu_Socket sock, int blocking) {
#ifdef _WIN32
    u_long mode = blocking ? 0 : 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) return IDCU_ERR_UNKNOWN;
    
    if (blocking) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }
    fcntl(sock, F_SETFL, flags);
#endif
    return IDCU_ERR_OK;
}

int idcu_tcp_socket_connect(idcu_TcpSocket* sock, const idcu_NetAddress* addr) {
    return idcu_tcp_socket_connect_timeout(sock, addr, sock->timeout_ms);
}

int idcu_tcp_socket_connect_timeout(idcu_TcpSocket* sock, const idcu_NetAddress* addr, int timeout_ms) {
    if (!sock || !addr) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (sock->is_connected) {
        idcu_tcp_socket_disconnect(sock);
    }
    
    int family = (addr->type == IDCU_NET_ADDR_IPV4) ? AF_INET : AF_INET6;
    sock->socket = socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (sock->socket == IDCU_INVALID_SOCKET) {
        return IDCU_ERR_UNKNOWN;
    }
    
    sock->remote_addr = *addr;
    set_socket_timeout(sock->socket, timeout_ms);
    set_socket_blocking(sock->socket, sock->is_blocking);
    
    struct sockaddr_storage sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    
    if (addr->type == IDCU_NET_ADDR_IPV4) {
        struct sockaddr_in* saddr = (struct sockaddr_in*)&sock_addr;
        saddr->sin_family = AF_INET;
        saddr->sin_addr = addr->addr.v4;
        saddr->sin_port = htons(addr->port);
    } else {
        struct sockaddr_in6* saddr = (struct sockaddr_in6*)&sock_addr;
        saddr->sin6_family = AF_INET6;
        saddr->sin6_addr = addr->addr.v6;
        saddr->sin6_port = htons(addr->port);
    }
    
    if (connect(sock->socket, (struct sockaddr*)&sock_addr, 
               (addr->type == IDCU_NET_ADDR_IPV4) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)) < 0) {
#ifdef _WIN32
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            closesocket(sock->socket);
            sock->socket = IDCU_INVALID_SOCKET;
            return IDCU_ERR_UNKNOWN;
        }
#else
        if (errno != EINPROGRESS) {
            close(sock->socket);
            sock->socket = IDCU_INVALID_SOCKET;
            return IDCU_ERR_UNKNOWN;
        }
#endif
    }
    
    sock->is_connected = 1;
    return IDCU_ERR_OK;
}

int idcu_tcp_socket_send(idcu_TcpSocket* sock, const void* data, size_t len, size_t* sent) {
    if (!sock || !sock->is_connected || !data || len == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    int result = send(sock->socket, (const char*)data, (int)len, 0);
    if (result < 0) {
        return IDCU_ERR_UNKNOWN;
    }
    
    if (sent) {
        *sent = (size_t)result;
    }
    
    return IDCU_ERR_OK;
}

int idcu_tcp_socket_send_all(idcu_TcpSocket* sock, const void* data, size_t len) {
    if (!sock || !sock->is_connected || !data || len == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t sent = 0;
    while (sent < len) {
        int result = send(sock->socket, (const char*)data + sent, (int)(len - sent), 0);
        if (result < 0) {
            return IDCU_ERR_UNKNOWN;
        }
        sent += (size_t)result;
    }
    
    return IDCU_ERR_OK;
}

int idcu_tcp_socket_recv(idcu_TcpSocket* sock, void* buffer, size_t len, size_t* received) {
    if (!sock || !sock->is_connected || !buffer || len == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    int result = recv(sock->socket, (char*)buffer, (int)len, 0);
    if (result < 0) {
        return IDCU_ERR_UNKNOWN;
    }
    if (result == 0) {
        idcu_tcp_socket_disconnect(sock);
        return IDCU_ERR_NOT_FOUND;
    }
    
    if (received) {
        *received = (size_t)result;
    }
    
    return IDCU_ERR_OK;
}

int idcu_tcp_socket_set_timeout(idcu_TcpSocket* sock, int timeout_ms) {
    if (!sock) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    sock->timeout_ms = timeout_ms;
    if (sock->socket != IDCU_INVALID_SOCKET) {
        set_socket_timeout(sock->socket, timeout_ms);
    }
    return IDCU_ERR_OK;
}

int idcu_tcp_socket_set_blocking(idcu_TcpSocket* sock, int blocking) {
    if (!sock) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    sock->is_blocking = blocking;
    if (sock->socket != IDCU_INVALID_SOCKET) {
        set_socket_blocking(sock->socket, blocking);
    }
    return IDCU_ERR_OK;
}

void idcu_tcp_socket_disconnect(idcu_TcpSocket* sock) {
    if (!sock) {
        return;
    }
    
    if (sock->socket != IDCU_INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(sock->socket);
#else
        close(sock->socket);
#endif
        sock->socket = IDCU_INVALID_SOCKET;
    }
    
    sock->is_connected = 0;
}

int idcu_tcp_server_init(idcu_TcpServer* server) {
    if (!server) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(server, 0, sizeof(*server));
    server->listen_socket = IDCU_INVALID_SOCKET;
    server->backlog = 10;
    
    return IDCU_ERR_OK;
}

void idcu_tcp_server_destroy(idcu_TcpServer* server) {
    if (!server) {
        return;
    }
    
    if (server->listen_socket != IDCU_INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(server->listen_socket);
#else
        close(server->listen_socket);
#endif
        server->listen_socket = IDCU_INVALID_SOCKET;
    }
    
    server->is_listening = 0;
}

int idcu_tcp_server_listen(idcu_TcpServer* server, const idcu_NetAddress* addr, int backlog) {
    if (!server || !addr) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (server->is_listening) {
        idcu_tcp_server_destroy(server);
    }
    
    int family = (addr->type == IDCU_NET_ADDR_IPV4) ? AF_INET : AF_INET6;
    server->listen_socket = socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (server->listen_socket == IDCU_INVALID_SOCKET) {
        return IDCU_ERR_UNKNOWN;
    }
    
    int opt = 1;
    setsockopt(server->listen_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    struct sockaddr_storage sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    
    if (addr->type == IDCU_NET_ADDR_IPV4) {
        struct sockaddr_in* saddr = (struct sockaddr_in*)&sock_addr;
        saddr->sin_family = AF_INET;
        saddr->sin_addr = addr->addr.v4;
        saddr->sin_port = htons(addr->port);
    } else {
        struct sockaddr_in6* saddr = (struct sockaddr_in6*)&sock_addr;
        saddr->sin6_family = AF_INET6;
        saddr->sin6_addr = addr->addr.v6;
        saddr->sin6_port = htons(addr->port);
    }
    
    if (bind(server->listen_socket, (struct sockaddr*)&sock_addr, 
             (addr->type == IDCU_NET_ADDR_IPV4) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)) < 0) {
        idcu_tcp_server_destroy(server);
        return IDCU_ERR_UNKNOWN;
    }
    
    if (listen(server->listen_socket, backlog > 0 ? backlog : 10) < 0) {
        idcu_tcp_server_destroy(server);
        return IDCU_ERR_UNKNOWN;
    }
    
    server->listen_addr = *addr;
    server->backlog = backlog > 0 ? backlog : 10;
    server->is_listening = 1;
    
    return IDCU_ERR_OK;
}

int idcu_tcp_server_accept(idcu_TcpServer* server, idcu_TcpSocket* client) {
    return idcu_tcp_server_accept_timeout(server, client, 0);
}

int idcu_tcp_server_accept_timeout(idcu_TcpServer* server, idcu_TcpSocket* client, int timeout_ms) {
    if (!server || !server->is_listening || !client) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_tcp_socket_init(client);
    
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    idcu_Socket sock = accept(server->listen_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (sock == IDCU_INVALID_SOCKET) {
        return IDCU_ERR_UNKNOWN;
    }
    
    client->socket = sock;
    client->protocol = IDCU_NET_PROTO_TCP;
    client->is_connected = 1;
    
    if (client_addr.ss_family == AF_INET) {
        client->remote_addr.type = IDCU_NET_ADDR_IPV4;
        client->remote_addr.addr.v4 = ((struct sockaddr_in*)&client_addr)->sin_addr;
        client->remote_addr.port = ntohs(((struct sockaddr_in*)&client_addr)->sin_port);
    } else {
        client->remote_addr.type = IDCU_NET_ADDR_IPV6;
        client->remote_addr.addr.v6 = ((struct sockaddr_in6*)&client_addr)->sin6_addr;
        client->remote_addr.port = ntohs(((struct sockaddr_in6*)&client_addr)->sin6_port);
    }
    
    return IDCU_ERR_OK;
}

int idcu_udp_socket_init(idcu_UdpSocket* sock) {
    if (!sock) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(sock, 0, sizeof(*sock));
    sock->socket = IDCU_INVALID_SOCKET;
    sock->timeout_ms = IDCU_NET_DEFAULT_TIMEOUT;
    
    return IDCU_ERR_OK;
}

void idcu_udp_socket_destroy(idcu_UdpSocket* sock) {
    if (!sock) {
        return;
    }
    
    if (sock->socket != IDCU_INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(sock->socket);
#else
        close(sock->socket);
#endif
        sock->socket = IDCU_INVALID_SOCKET;
    }
    
    sock->is_bound = 0;
}

int idcu_udp_socket_bind(idcu_UdpSocket* sock, const idcu_NetAddress* addr) {
    if (!sock || !addr) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (sock->is_bound) {
        idcu_udp_socket_destroy(sock);
    }
    
    int family = (addr->type == IDCU_NET_ADDR_IPV4) ? AF_INET : AF_INET6;
    sock->socket = socket(family, SOCK_DGRAM, IPPROTO_UDP);
    if (sock->socket == IDCU_INVALID_SOCKET) {
        return IDCU_ERR_UNKNOWN;
    }
    
    int opt = 1;
    setsockopt(sock->socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    set_socket_timeout(sock->socket, sock->timeout_ms);
    
    struct sockaddr_storage sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    
    if (addr->type == IDCU_NET_ADDR_IPV4) {
        struct sockaddr_in* saddr = (struct sockaddr_in*)&sock_addr;
        saddr->sin_family = AF_INET;
        saddr->sin_addr = addr->addr.v4;
        saddr->sin_port = htons(addr->port);
    } else {
        struct sockaddr_in6* saddr = (struct sockaddr_in6*)&sock_addr;
        saddr->sin6_family = AF_INET6;
        saddr->sin6_addr = addr->addr.v6;
        saddr->sin6_port = htons(addr->port);
    }
    
    if (bind(sock->socket, (struct sockaddr*)&sock_addr, 
             (addr->type == IDCU_NET_ADDR_IPV4) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)) < 0) {
        idcu_udp_socket_destroy(sock);
        return IDCU_ERR_UNKNOWN;
    }
    
    sock->local_addr = *addr;
    sock->is_bound = 1;
    
    return IDCU_ERR_OK;
}

int idcu_udp_socket_send(idcu_UdpSocket* sock, const idcu_NetAddress* dest, const void* data, size_t len) {
    if (!sock || !dest || !data || len == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (sock->socket == IDCU_INVALID_SOCKET) {
        int family = (dest->type == IDCU_NET_ADDR_IPV4) ? AF_INET : AF_INET6;
        sock->socket = socket(family, SOCK_DGRAM, IPPROTO_UDP);
        if (sock->socket == IDCU_INVALID_SOCKET) {
            return IDCU_ERR_UNKNOWN;
        }
    }
    
    struct sockaddr_storage sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    
    if (dest->type == IDCU_NET_ADDR_IPV4) {
        struct sockaddr_in* saddr = (struct sockaddr_in*)&sock_addr;
        saddr->sin_family = AF_INET;
        saddr->sin_addr = dest->addr.v4;
        saddr->sin_port = htons(dest->port);
    } else {
        struct sockaddr_in6* saddr = (struct sockaddr_in6*)&sock_addr;
        saddr->sin6_family = AF_INET6;
        saddr->sin6_addr = dest->addr.v6;
        saddr->sin6_port = htons(dest->port);
    }
    
    int result = sendto(sock->socket, (const char*)data, (int)len, 0,
                       (struct sockaddr*)&sock_addr,
                       (dest->type == IDCU_NET_ADDR_IPV4) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6));
    if (result < 0) {
        return IDCU_ERR_UNKNOWN;
    }
    
    return IDCU_ERR_OK;
}

int idcu_udp_socket_recv(idcu_UdpSocket* sock, idcu_NetAddress* source, void* buffer, size_t len, size_t* received) {
    if (!sock || !buffer || len == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (!sock->is_bound) {
        return IDCU_ERR_INVALID_STATE;
    }
    
    struct sockaddr_storage source_addr;
    socklen_t addr_len = sizeof(source_addr);
    
    int result = recvfrom(sock->socket, (char*)buffer, (int)len, 0,
                         (struct sockaddr*)&source_addr, &addr_len);
    if (result < 0) {
        return IDCU_ERR_UNKNOWN;
    }
    
    if (received) {
        *received = (size_t)result;
    }
    
    if (source) {
        memset(source, 0, sizeof(*source));
        if (source_addr.ss_family == AF_INET) {
            source->type = IDCU_NET_ADDR_IPV4;
            source->addr.v4 = ((struct sockaddr_in*)&source_addr)->sin_addr;
            source->port = ntohs(((struct sockaddr_in*)&source_addr)->sin_port);
        } else {
            source->type = IDCU_NET_ADDR_IPV6;
            source->addr.v6 = ((struct sockaddr_in6*)&source_addr)->sin6_addr;
            source->port = ntohs(((struct sockaddr_in6*)&source_addr)->sin6_port);
        }
    }
    
    return IDCU_ERR_OK;
}

int idcu_udp_socket_set_timeout(idcu_UdpSocket* sock, int timeout_ms) {
    if (!sock) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    sock->timeout_ms = timeout_ms;
    if (sock->socket != IDCU_INVALID_SOCKET) {
        set_socket_timeout(sock->socket, timeout_ms);
    }
    return IDCU_ERR_OK;
}
