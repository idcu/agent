#include <idcu/testframework/testframework.h>
#include <idcu/network/network.h>
#include <idcu/network/types.h>
#include <stdio.h>
#include <string.h>

IDCU_TEST_CASE(network, init_cleanup) {
    int ret = idcu_network_init();
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_network_cleanup();
}

IDCU_TEST_CASE(network, multiple_init) {
    int ret = idcu_network_init();
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    ret = idcu_network_init();
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_network_cleanup();
}

IDCU_TEST_CASE(network, address_init_ipv4) {
    idcu_NetAddress addr;
    int ret = idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "127.0.0.1", 8080);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(IDCU_NET_ADDR_IPV4, addr.type);
    IDCU_TEST_ASSERT_EQUAL(8080, addr.port);
}

IDCU_TEST_CASE(network, address_init_ipv6) {
    idcu_NetAddress addr;
    int ret = idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV6, "::1", 8080);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(IDCU_NET_ADDR_IPV6, addr.type);
    IDCU_TEST_ASSERT_EQUAL(8080, addr.port);
}

IDCU_TEST_CASE(network, address_init_any_ipv4) {
    idcu_NetAddress addr;
    int ret = idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, NULL, 8080);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(IDCU_NET_ADDR_IPV4, addr.type);
    IDCU_TEST_ASSERT_EQUAL(8080, addr.port);
}

IDCU_TEST_CASE(network, address_init_null_addr) {
    int ret = idcu_net_address_init(NULL, IDCU_NET_ADDR_IPV4, "127.0.0.1", 8080);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
}

IDCU_TEST_CASE(network, address_init_invalid_ip) {
    idcu_NetAddress addr;
    int ret = idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "999.999.999.999", 8080);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
}

IDCU_TEST_CASE(network, address_to_string_ipv4) {
    idcu_NetAddress addr;
    idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "127.0.0.1", 8080);
    
    char buffer[64];
    int ret = idcu_net_address_to_string(&addr, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_STRING_EQUAL("127.0.0.1", buffer);
}

IDCU_TEST_CASE(network, address_to_string_null_args) {
    idcu_NetAddress addr;
    idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "127.0.0.1", 8080);
    
    char buffer[64];
    int ret = idcu_net_address_to_string(NULL, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_net_address_to_string(&addr, NULL, sizeof(buffer));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
}

IDCU_TEST_CASE(network, address_resolve_localhost) {
    idcu_NetAddress addr;
    int ret = idcu_net_address_resolve(&addr, "localhost", 8080);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(8080, addr.port);
}

IDCU_TEST_CASE(network, address_resolve_null_args) {
    idcu_NetAddress addr;
    int ret = idcu_net_address_resolve(NULL, "localhost", 8080);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_net_address_resolve(&addr, NULL, 8080);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
}

IDCU_TEST_CASE(network, address_resolve_invalid_host) {
    idcu_NetAddress addr;
    int ret = idcu_net_address_resolve(&addr, "nonexistent.example.invalid", 8080);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_NOT_FOUND, ret);
}

IDCU_TEST_CASE(network, tcp_socket_init_destroy) {
    idcu_TcpSocket sock;
    int ret = idcu_tcp_socket_init(&sock);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(IDCU_NET_PROTO_TCP, sock.protocol);
    
    idcu_tcp_socket_destroy(&sock);
}

IDCU_TEST_CASE(network, tcp_socket_init_null) {
    int ret = idcu_tcp_socket_init(NULL);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
}

IDCU_TEST_CASE(network, tcp_socket_set_timeout) {
    idcu_TcpSocket sock;
    idcu_tcp_socket_init(&sock);
    
    int ret = idcu_tcp_socket_set_timeout(&sock, 5000);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(5000, sock.timeout_ms);
    
    idcu_tcp_socket_destroy(&sock);
}

IDCU_TEST_CASE(network, tcp_socket_set_timeout_null) {
    int ret = idcu_tcp_socket_set_timeout(NULL, 5000);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
}

IDCU_TEST_CASE(network, tcp_socket_set_blocking) {
    idcu_TcpSocket sock;
    idcu_tcp_socket_init(&sock);
    
    int ret = idcu_tcp_socket_set_blocking(&sock, 0);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(0, sock.is_blocking);
    
    ret = idcu_tcp_socket_set_blocking(&sock, 1);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(1, sock.is_blocking);
    
    idcu_tcp_socket_destroy(&sock);
}

IDCU_TEST_CASE(network, tcp_socket_set_blocking_null) {
    int ret = idcu_tcp_socket_set_blocking(NULL, 1);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
}

IDCU_TEST_CASE(network, tcp_socket_disconnect_not_connected) {
    idcu_TcpSocket sock;
    idcu_tcp_socket_init(&sock);
    
    idcu_tcp_socket_disconnect(&sock);
    IDCU_TEST_ASSERT_EQUAL(0, sock.is_connected);
    
    idcu_tcp_socket_destroy(&sock);
}

IDCU_TEST_CASE(network, tcp_socket_destroy_null) {
    idcu_tcp_socket_destroy(NULL);
    IDCU_TEST_PASS();
}

IDCU_TEST_CASE(network, tcp_socket_disconnect_null) {
    idcu_tcp_socket_disconnect(NULL);
    IDCU_TEST_PASS();
}

IDCU_TEST_CASE(network, tcp_server_init_destroy) {
    idcu_TcpServer server;
    int ret = idcu_tcp_server_init(&server);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(0, server.is_listening);
    
    idcu_tcp_server_destroy(&server);
}

IDCU_TEST_CASE(network, tcp_server_init_null) {
    int ret = idcu_tcp_server_init(NULL);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
}

IDCU_TEST_CASE(network, tcp_server_destroy_null) {
    idcu_tcp_server_destroy(NULL);
    IDCU_TEST_PASS();
}

IDCU_TEST_CASE(network, udp_socket_init_destroy) {
    idcu_UdpSocket sock;
    int ret = idcu_udp_socket_init(&sock);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_udp_socket_destroy(&sock);
}

IDCU_TEST_CASE(network, udp_socket_init_null) {
    int ret = idcu_udp_socket_init(NULL);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
}

IDCU_TEST_CASE(network, udp_socket_destroy_null) {
    idcu_udp_socket_destroy(NULL);
    IDCU_TEST_PASS();
}

IDCU_TEST_CASE(network, udp_socket_set_timeout) {
    idcu_UdpSocket sock;
    idcu_udp_socket_init(&sock);
    
    int ret = idcu_udp_socket_set_timeout(&sock, 3000);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(3000, sock.timeout_ms);
    
    idcu_udp_socket_destroy(&sock);
}

IDCU_TEST_CASE(network, udp_socket_set_timeout_null) {
    int ret = idcu_udp_socket_set_timeout(NULL, 3000);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
}

IDCU_TEST_CASE(network, udp_socket_bind_null_args) {
    idcu_UdpSocket sock;
    idcu_udp_socket_init(&sock);
    
    idcu_NetAddress addr;
    idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "127.0.0.1", 9090);
    
    int ret = idcu_udp_socket_bind(NULL, &addr);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_udp_socket_bind(&sock, NULL);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    idcu_udp_socket_destroy(&sock);
}

IDCU_TEST_CASE(network, udp_socket_send_null_args) {
    idcu_UdpSocket sock;
    idcu_udp_socket_init(&sock);
    
    idcu_NetAddress addr;
    idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "127.0.0.1", 9090);
    
    const char* data = "test";
    
    int ret = idcu_udp_socket_send(NULL, &addr, data, strlen(data));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_udp_socket_send(&sock, NULL, data, strlen(data));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_udp_socket_send(&sock, &addr, NULL, strlen(data));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_udp_socket_send(&sock, &addr, data, 0);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    idcu_udp_socket_destroy(&sock);
}

IDCU_TEST_CASE(network, udp_socket_recv_null_args) {
    idcu_UdpSocket sock;
    idcu_udp_socket_init(&sock);
    
    char buffer[128];
    size_t received;
    
    int ret = idcu_udp_socket_recv(NULL, NULL, buffer, sizeof(buffer), &received);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_udp_socket_recv(&sock, NULL, NULL, sizeof(buffer), &received);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_udp_socket_recv(&sock, NULL, buffer, 0, &received);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    idcu_udp_socket_destroy(&sock);
}

IDCU_TEST_CASE(network, tcp_socket_send_null_args) {
    idcu_TcpSocket sock;
    idcu_tcp_socket_init(&sock);
    
    const char* data = "test";
    size_t sent;
    
    int ret = idcu_tcp_socket_send(NULL, data, strlen(data), &sent);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_tcp_socket_send(&sock, NULL, strlen(data), &sent);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_tcp_socket_send(&sock, data, 0, &sent);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    idcu_tcp_socket_destroy(&sock);
}

IDCU_TEST_CASE(network, tcp_socket_send_all_null_args) {
    idcu_TcpSocket sock;
    idcu_tcp_socket_init(&sock);
    
    const char* data = "test";
    
    int ret = idcu_tcp_socket_send_all(NULL, data, strlen(data));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_tcp_socket_send_all(&sock, NULL, strlen(data));
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_tcp_socket_send_all(&sock, data, 0);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    idcu_tcp_socket_destroy(&sock);
}

IDCU_TEST_CASE(network, tcp_socket_recv_null_args) {
    idcu_TcpSocket sock;
    idcu_tcp_socket_init(&sock);
    
    char buffer[128];
    size_t received;
    
    int ret = idcu_tcp_socket_recv(NULL, buffer, sizeof(buffer), &received);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_tcp_socket_recv(&sock, NULL, sizeof(buffer), &received);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_tcp_socket_recv(&sock, buffer, 0, &received);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    idcu_tcp_socket_destroy(&sock);
}

IDCU_TEST_CASE(network, tcp_server_listen_null_args) {
    idcu_TcpServer server;
    idcu_tcp_server_init(&server);
    
    idcu_NetAddress addr;
    idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "127.0.0.1", 9090);
    
    int ret = idcu_tcp_server_listen(NULL, &addr, 10);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_tcp_server_listen(&server, NULL, 10);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    idcu_tcp_server_destroy(&server);
}

IDCU_TEST_CASE(network, tcp_server_accept_null_args) {
    idcu_TcpServer server;
    idcu_tcp_server_init(&server);
    
    idcu_TcpSocket client;
    
    int ret = idcu_tcp_server_accept(NULL, &client);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_tcp_server_accept(&server, NULL);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    idcu_tcp_server_destroy(&server);
}

IDCU_TEST_CASE(network, tcp_server_accept_timeout_null_args) {
    idcu_TcpServer server;
    idcu_tcp_server_init(&server);
    
    idcu_TcpSocket client;
    
    int ret = idcu_tcp_server_accept_timeout(NULL, &client, 1000);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    ret = idcu_tcp_server_accept_timeout(&server, NULL, 1000);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
    
    idcu_tcp_server_destroy(&server);
}

int main(void) {
    int ret = idcu_network_init();
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    int test_result = idcu_test_run_all();
    
    idcu_network_cleanup();
    return test_result;
}
