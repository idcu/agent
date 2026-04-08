#include "idcu/network/network_layer.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <fcntl.h>
#endif

#ifdef _WIN32
static int g_winsock_initialized = 0;
#endif

int idcu_network_init(void) {
#ifdef _WIN32
    if (g_winsock_initialized) {
        return IDCU_ERR_OK;
    }
    WSADATA wsa_data;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (ret != 0) {
        IDCU_LOG_ERROR("WSAStartup failed: %d", ret);
        return IDCU_ERR_NETWORK_INIT;
    }
    g_winsock_initialized = 1;
    IDCU_LOG_INFO("Winsock initialized");
#endif
    return IDCU_ERR_OK;
}

void idcu_network_cleanup(void) {
#ifdef _WIN32
    if (g_winsock_initialized) {
        WSACleanup();
        g_winsock_initialized = 0;
        IDCU_LOG_INFO("Winsock cleaned up");
    }
#endif
}

static void sleep_ms(uint32_t ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

int idcu_network_socket_create(idcu_NetworkSocket *sock, int protocol) {
    if (!sock)
        return IDCU_ERR_INVALID_PARAM;
    if (protocol != IDCU_NET_PROTO_TCP && protocol != IDCU_NET_PROTO_UDP)
        return IDCU_ERR_INVALID_PARAM;

    memset(sock, 0, sizeof(idcu_NetworkSocket));
    sock->protocol = protocol;
    sock->fd = IDCU_INVALID_SOCKET;
    sock->connected = 0;

    sock->config.connect_timeout_ms = IDCU_NET_DEFAULT_TIMEOUT_MS;
    sock->config.send_timeout_ms = IDCU_NET_DEFAULT_TIMEOUT_MS;
    sock->config.recv_timeout_ms = IDCU_NET_DEFAULT_TIMEOUT_MS;
    sock->config.max_retries = IDCU_NET_DEFAULT_MAX_RETRIES;
    sock->config.retry_delay_ms = IDCU_NET_DEFAULT_RETRY_DELAY_MS;
    sock->config.reconnect_delay_ms = IDCU_NET_DEFAULT_RECONNECT_DELAY_MS;
    sock->config.retry_count = 0;

    int type = (protocol == IDCU_NET_PROTO_TCP) ? SOCK_STREAM : SOCK_DGRAM;
    int iproto = (protocol == IDCU_NET_PROTO_TCP) ? IPPROTO_TCP : IPPROTO_UDP;

    sock->fd = socket(AF_INET, type, iproto);
    if (sock->fd == IDCU_INVALID_SOCKET) {
#ifdef _WIN32
        IDCU_LOG_ERROR("socket() failed: %d", WSAGetLastError());
#else
        IDCU_LOG_ERROR("socket() failed: %m");
#endif
        return IDCU_ERR_NETWORK_INIT;
    }

    // Set non-blocking mode
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sock->fd, FIONBIO, &mode);
#else
    int flags = fcntl(sock->fd, F_GETFL, 0);
    fcntl(sock->fd, F_SETFL, flags | O_NONBLOCK);
#endif

    return IDCU_ERR_OK;
}

void idcu_network_socket_destroy(idcu_NetworkSocket *sock) {
    if (!sock)
        return;
    idcu_network_socket_close(sock);
}

int idcu_network_socket_connect(idcu_NetworkSocket *sock, const char *address, uint16_t port) {
    if (!sock || !address)
        return IDCU_ERR_INVALID_PARAM;
    if (sock->fd == IDCU_INVALID_SOCKET)
        return IDCU_ERR_NOT_INITIALIZED;
    if (sock->protocol != IDCU_NET_PROTO_TCP)
        return IDCU_ERR_INVALID_PARAM;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0) {
        struct hostent *host = gethostbyname(address);
        if (!host) {
            IDCU_LOG_ERROR("Failed to resolve hostname: %s", address);
            return IDCU_ERR_NETWORK_CONNECT;
        }
        memcpy(&server_addr.sin_addr, host->h_addr_list[0], host->h_length);
    }

    if (connect(sock->fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
#ifdef _WIN32
        IDCU_LOG_ERROR("connect() failed: %d", WSAGetLastError());
#else
        IDCU_LOG_ERROR("connect() failed: %m");
#endif
        return IDCU_ERR_NETWORK_CONNECT;
    }

    sock->connected = 1;
    strncpy(sock->remote_addr, address, IDCU_ADDR_MAX - 1);
    sock->remote_addr[IDCU_ADDR_MAX - 1] = '\0';
    sock->remote_port = port;

    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    if (getsockname(sock->fd, (struct sockaddr *)&local_addr, &addr_len) == 0) {
        inet_ntop(AF_INET, &local_addr.sin_addr, sock->local_addr, IDCU_ADDR_MAX);
        sock->local_port = ntohs(local_addr.sin_port);
    }

    return IDCU_ERR_OK;
}

int idcu_network_socket_bind(idcu_NetworkSocket *sock, const char *address, uint16_t port) {
    if (!sock)
        return IDCU_ERR_INVALID_PARAM;
    if (sock->fd == IDCU_INVALID_SOCKET)
        return IDCU_ERR_NOT_INITIALIZED;

    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);

    if (address && strlen(address) > 0) {
        if (inet_pton(AF_INET, address, &bind_addr.sin_addr) <= 0) {
            IDCU_LOG_ERROR("Invalid bind address: %s", address);
            return IDCU_ERR_INVALID_PARAM;
        }
    } else {
        bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    if (bind(sock->fd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) == -1) {
#ifdef _WIN32
        IDCU_LOG_ERROR("bind() failed: %d", WSAGetLastError());
#else
        IDCU_LOG_ERROR("bind() failed: %m");
#endif
        return IDCU_ERR_NETWORK_INIT;
    }

    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    if (getsockname(sock->fd, (struct sockaddr *)&local_addr, &addr_len) == 0) {
        inet_ntop(AF_INET, &local_addr.sin_addr, sock->local_addr, IDCU_ADDR_MAX);
        sock->local_port = ntohs(local_addr.sin_port);
    } else {
        strncpy(sock->local_addr, address ? address : "0.0.0.0", IDCU_ADDR_MAX - 1);
        sock->local_addr[IDCU_ADDR_MAX - 1] = '\0';
        sock->local_port = port;
    }

    return IDCU_ERR_OK;
}

int idcu_network_socket_send(idcu_NetworkSocket *sock, const void *data, size_t len, size_t *sent) {
    if (!sock || !data)
        return IDCU_ERR_INVALID_PARAM;
    if (sock->fd == IDCU_INVALID_SOCKET)
        return IDCU_ERR_NOT_INITIALIZED;
    if (sock->protocol == IDCU_NET_PROTO_TCP && !sock->connected)
        return IDCU_ERR_NETWORK_CONNECT;

#ifdef _WIN32
    int result = send(sock->fd, (const char *)data, (int)len, 0);
#else
    ssize_t result = send(sock->fd, data, len, 0);
#endif

    if (result == -1) {
#ifdef _WIN32
        IDCU_LOG_ERROR("send() failed: %d", WSAGetLastError());
#else
        IDCU_LOG_ERROR("send() failed: %m");
#endif
        return IDCU_ERR_NETWORK_SEND;
    }

    if (sent)
        *sent = (size_t)result;
    return IDCU_ERR_OK;
}

int idcu_network_socket_recv(idcu_NetworkSocket *sock, void *data, size_t len, size_t *received) {
    if (!sock || !data)
        return IDCU_ERR_INVALID_PARAM;
    if (sock->fd == IDCU_INVALID_SOCKET)
        return IDCU_ERR_NOT_INITIALIZED;
    if (sock->protocol == IDCU_NET_PROTO_TCP && !sock->connected)
        return IDCU_ERR_NETWORK_CONNECT;

#ifdef _WIN32
    int result = recv(sock->fd, (char *)data, (int)len, 0);
#else
    ssize_t result = recv(sock->fd, data, len, 0);
#endif

    if (result == -1) {
#ifdef _WIN32
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            if (received)
                *received = 0;
            return IDCU_ERR_OK;
        }
        IDCU_LOG_ERROR("recv() failed: %d", err);
#else
        IDCU_LOG_ERROR("recv() failed: %m");
#endif
        return IDCU_ERR_NETWORK_RECV;
    }

    if (received)
        *received = (size_t)result;
    return IDCU_ERR_OK;
}

int idcu_network_socket_sendto(idcu_NetworkSocket *sock, const void *data, size_t len,
                               const char *address, uint16_t port, size_t *sent) {
    if (!sock || !data || !address)
        return IDCU_ERR_INVALID_PARAM;
    if (sock->fd == IDCU_INVALID_SOCKET)
        return IDCU_ERR_NOT_INITIALIZED;

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &dest_addr.sin_addr) <= 0) {
        struct hostent *host = gethostbyname(address);
        if (!host) {
            IDCU_LOG_ERROR("Failed to resolve hostname: %s", address);
            return IDCU_ERR_NETWORK_SEND;
        }
        memcpy(&dest_addr.sin_addr, host->h_addr_list[0], host->h_length);
    }

#ifdef _WIN32
    int result = sendto(sock->fd, (const char *)data, (int)len, 0, (struct sockaddr *)&dest_addr,
                        sizeof(dest_addr));
#else
    ssize_t result =
        sendto(sock->fd, data, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
#endif

    if (result == -1) {
#ifdef _WIN32
        IDCU_LOG_ERROR("sendto() failed: %d", WSAGetLastError());
#else
        IDCU_LOG_ERROR("sendto() failed: %m");
#endif
        return IDCU_ERR_NETWORK_SEND;
    }

    if (sent)
        *sent = (size_t)result;
    return IDCU_ERR_OK;
}

int idcu_network_socket_recvfrom(idcu_NetworkSocket *sock, void *data, size_t len, char *address,
                                 size_t addr_len, uint16_t *port, size_t *received) {
    if (!sock || !data)
        return IDCU_ERR_INVALID_PARAM;
    if (sock->fd == IDCU_INVALID_SOCKET)
        return IDCU_ERR_NOT_INITIALIZED;

    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    memset(&from_addr, 0, sizeof(from_addr));

#ifdef _WIN32
    int result =
        recvfrom(sock->fd, (char *)data, (int)len, 0, (struct sockaddr *)&from_addr, &from_len);
#else
    ssize_t result = recvfrom(sock->fd, data, len, 0, (struct sockaddr *)&from_addr, &from_len);
#endif

    if (result == -1) {
#ifdef _WIN32
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            if (received)
                *received = 0;
            return IDCU_ERR_OK;
        }
        IDCU_LOG_ERROR("recvfrom() failed: %d", err);
#else
        IDCU_LOG_ERROR("recvfrom() failed: %m");
#endif
        return IDCU_ERR_NETWORK_RECV;
    }

    if (address && addr_len > 0) {
        inet_ntop(AF_INET, &from_addr.sin_addr, address, (socklen_t)addr_len);
    }
    if (port)
        *port = ntohs(from_addr.sin_port);
    if (received)
        *received = (size_t)result;

    return IDCU_ERR_OK;
}

int idcu_network_socket_close(idcu_NetworkSocket *sock) {
    if (!sock)
        return IDCU_ERR_INVALID_PARAM;
    if (sock->fd == IDCU_INVALID_SOCKET)
        return IDCU_ERR_OK;

#ifdef _WIN32
    closesocket(sock->fd);
#else
    close(sock->fd);
#endif

    sock->fd = IDCU_INVALID_SOCKET;
    sock->connected = 0;
    return IDCU_ERR_OK;
}

int idcu_network_server_create(idcu_NetworkServer *server, int protocol, const char *address,
                               uint16_t port) {
    if (!server || !address)
        return IDCU_ERR_INVALID_PARAM;
    if (protocol != IDCU_NET_PROTO_TCP && protocol != IDCU_NET_PROTO_UDP)
        return IDCU_ERR_INVALID_PARAM;

    memset(server, 0, sizeof(idcu_NetworkServer));
    server->protocol = protocol;
    server->fd = IDCU_INVALID_SOCKET;
    strncpy(server->bind_address, address, IDCU_ADDR_MAX - 1);
    server->bind_address[IDCU_ADDR_MAX - 1] = '\0';
    server->bind_port = port;
    server->listening = 0;

    int type = (protocol == IDCU_NET_PROTO_TCP) ? SOCK_STREAM : SOCK_DGRAM;
    int iproto = (protocol == IDCU_NET_PROTO_TCP) ? IPPROTO_TCP : IPPROTO_UDP;

    server->fd = socket(AF_INET, type, iproto);
    if (server->fd == IDCU_INVALID_SOCKET) {
#ifdef _WIN32
        IDCU_LOG_ERROR("socket() failed: %d", WSAGetLastError());
#else
        IDCU_LOG_ERROR("socket() failed: %m");
#endif
        return IDCU_ERR_NETWORK_INIT;
    }

    int opt = 1;
    setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);

    if (strcmp(address, "0.0.0.0") == 0 || strcmp(address, "") == 0) {
        bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        if (inet_pton(AF_INET, address, &bind_addr.sin_addr) <= 0) {
            IDCU_LOG_ERROR("Invalid bind address: %s", address);
#ifdef _WIN32
            closesocket(server->fd);
#else
            close(server->fd);
#endif
            server->fd = IDCU_INVALID_SOCKET;
            return IDCU_ERR_INVALID_PARAM;
        }
    }

    if (bind(server->fd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) == -1) {
#ifdef _WIN32
        IDCU_LOG_ERROR("bind() failed: %d", WSAGetLastError());
        closesocket(server->fd);
#else
        IDCU_LOG_ERROR("bind() failed: %m");
        close(server->fd);
#endif
        server->fd = IDCU_INVALID_SOCKET;
        return IDCU_ERR_NETWORK_INIT;
    }

    return IDCU_ERR_OK;
}

void idcu_network_server_destroy(idcu_NetworkServer *server) {
    if (!server)
        return;
    idcu_network_server_close(server);
}

int idcu_network_server_listen(idcu_NetworkServer *server) {
    if (!server)
        return IDCU_ERR_INVALID_PARAM;
    if (server->fd == IDCU_INVALID_SOCKET)
        return IDCU_ERR_NOT_INITIALIZED;
    if (server->protocol != IDCU_NET_PROTO_TCP)
        return IDCU_ERR_INVALID_PARAM;

    if (listen(server->fd, IDCU_NET_BACKLOG) == -1) {
#ifdef _WIN32
        IDCU_LOG_ERROR("listen() failed: %d", WSAGetLastError());
#else
        IDCU_LOG_ERROR("listen() failed: %m");
#endif
        return IDCU_ERR_NETWORK_INIT;
    }

    server->listening = 1;
    return IDCU_ERR_OK;
}

int idcu_network_server_accept(idcu_NetworkServer *server, idcu_NetworkSocket *client_sock) {
    if (!server || !client_sock)
        return IDCU_ERR_INVALID_PARAM;
    if (server->fd == IDCU_INVALID_SOCKET)
        return IDCU_ERR_NOT_INITIALIZED;
    if (!server->listening)
        return IDCU_ERR_NETWORK_INIT;

    memset(client_sock, 0, sizeof(idcu_NetworkSocket));
    client_sock->protocol = IDCU_NET_PROTO_TCP;
    client_sock->connected = 1;

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    client_sock->fd = accept(server->fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_sock->fd == IDCU_INVALID_SOCKET) {
#ifdef _WIN32
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            client_sock->fd = IDCU_INVALID_SOCKET;
            client_sock->connected = 0;
            return IDCU_ERR_OK;
        }
        IDCU_LOG_ERROR("accept() failed: %d", err);
#else
        IDCU_LOG_ERROR("accept() failed: %m");
#endif
        client_sock->fd = IDCU_INVALID_SOCKET;
        client_sock->connected = 0;
        return IDCU_ERR_NETWORK_CONNECT;
    }

    inet_ntop(AF_INET, &client_addr.sin_addr, client_sock->remote_addr, IDCU_ADDR_MAX);
    client_sock->remote_port = ntohs(client_addr.sin_port);

    struct sockaddr_in local_addr;
    socklen_t local_len = sizeof(local_addr);
    if (getsockname(client_sock->fd, (struct sockaddr *)&local_addr, &local_len) == 0) {
        inet_ntop(AF_INET, &local_addr.sin_addr, client_sock->local_addr, IDCU_ADDR_MAX);
        client_sock->local_port = ntohs(local_addr.sin_port);
    }

    return IDCU_ERR_OK;
}

int idcu_network_server_close(idcu_NetworkServer *server) {
    if (!server)
        return IDCU_ERR_INVALID_PARAM;
    if (server->fd == IDCU_INVALID_SOCKET)
        return IDCU_ERR_OK;

#ifdef _WIN32
    closesocket(server->fd);
#else
    close(server->fd);
#endif

    server->fd = IDCU_INVALID_SOCKET;
    server->listening = 0;
    return IDCU_ERR_OK;
}

int idcu_network_socket_set_config(idcu_NetworkSocket *sock, const idcu_NetworkConfig *config) {
    if (!sock || !config)
        return IDCU_ERR_INVALID_PARAM;
    memcpy(&sock->config, config, sizeof(idcu_NetworkConfig));
    return IDCU_ERR_OK;
}

int idcu_network_socket_get_config(idcu_NetworkSocket *sock, idcu_NetworkConfig *config) {
    if (!sock || !config)
        return IDCU_ERR_INVALID_PARAM;
    memcpy(config, &sock->config, sizeof(idcu_NetworkConfig));
    return IDCU_ERR_OK;
}

int idcu_network_socket_set_timeout(idcu_NetworkSocket *sock, uint32_t timeout_ms) {
    if (!sock)
        return IDCU_ERR_INVALID_PARAM;
    sock->config.send_timeout_ms = timeout_ms;
    sock->config.recv_timeout_ms = timeout_ms;
    sock->config.connect_timeout_ms = timeout_ms;
    return IDCU_ERR_OK;
}

int idcu_network_socket_reconnect(idcu_NetworkSocket *sock) {
    if (!sock)
        return IDCU_ERR_INVALID_PARAM;
    if (sock->fd == IDCU_INVALID_SOCKET)
        return IDCU_ERR_NOT_INITIALIZED;
    if (sock->protocol != IDCU_NET_PROTO_TCP)
        return IDCU_ERR_INVALID_PARAM;
    if (strlen(sock->remote_addr) == 0 || sock->remote_port == 0)
        return IDCU_ERR_INVALID_PARAM;

    idcu_network_socket_close(sock);

    int ret = idcu_network_socket_create(sock, sock->protocol);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    return idcu_network_socket_connect(sock, sock->remote_addr, sock->remote_port);
}

int idcu_network_socket_send_with_retry(idcu_NetworkSocket *sock, const void *data, size_t len,
                                        size_t *sent) {
    if (!sock || !data)
        return IDCU_ERR_INVALID_PARAM;

    int ret;
    uint32_t retry_count = 0;
    size_t total_sent = 0;
    const uint8_t *ptr = (const uint8_t *)data;

    while (retry_count <= sock->config.max_retries) {
        size_t current_sent = 0;
        ret = idcu_network_socket_send(sock, ptr + total_sent, len - total_sent, &current_sent);

        if (ret == IDCU_ERR_OK) {
            total_sent += current_sent;
            if (total_sent >= len) {
                if (sent)
                    *sent = total_sent;
                sock->config.retry_count = retry_count;
                return IDCU_ERR_OK;
            }
        } else {
            if (retry_count >= sock->config.max_retries) {
                if (sent)
                    *sent = total_sent;
                sock->config.retry_count = retry_count;
                return ret;
            }

            IDCU_LOG_WARNING("Send failed, retrying %u/%u...", retry_count + 1,
                             sock->config.max_retries);
            sleep_ms(sock->config.retry_delay_ms);
        }

        retry_count++;
    }

    if (sent)
        *sent = total_sent;
    sock->config.retry_count = retry_count;
    return IDCU_ERR_NETWORK_SEND;
}

int idcu_network_socket_recv_with_retry(idcu_NetworkSocket *sock, void *data, size_t len,
                                        size_t *received) {
    if (!sock || !data)
        return IDCU_ERR_INVALID_PARAM;

    int ret;
    uint32_t retry_count = 0;
    size_t total_received = 0;
    uint8_t *ptr = (uint8_t *)data;

    while (retry_count <= sock->config.max_retries) {
        size_t current_received = 0;
        ret = idcu_network_socket_recv(sock, ptr + total_received, len - total_received,
                                       &current_received);

        if (ret == IDCU_ERR_OK) {
            total_received += current_received;
            if (total_received > 0 || retry_count > 0) {
                if (received)
                    *received = total_received;
                sock->config.retry_count = retry_count;
                return IDCU_ERR_OK;
            }
        } else {
            if (retry_count >= sock->config.max_retries) {
                if (received)
                    *received = total_received;
                sock->config.retry_count = retry_count;
                return ret;
            }

            IDCU_LOG_WARNING("Recv failed, retrying %u/%u...", retry_count + 1,
                             sock->config.max_retries);
            sleep_ms(sock->config.retry_delay_ms);
        }

        retry_count++;
    }

    if (received)
        *received = total_received;
    sock->config.retry_count = retry_count;
    return IDCU_ERR_NETWORK_RECV;
}
