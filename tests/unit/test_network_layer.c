#include "test/test_framework.h"
#include "network_layer.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_network_init_cleanup(void) {
    int ret = idcu_network_init();
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Network init should succeed");
    
    idcu_network_cleanup();
    IDCU_TEST_PASS();
}

static void test_network_socket_create_destroy(void) {
    int ret = idcu_network_init();
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Network init should succeed");
    
    idcu_NetworkSocket sock;
    
    ret = idcu_network_socket_create(&sock, IDCU_NET_PROTO_TCP);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Create TCP socket should succeed");
    IDCU_TEST_ASSERT(sock.protocol == IDCU_NET_PROTO_TCP, "Protocol should be TCP");
    IDCU_TEST_ASSERT(sock.fd != IDCU_INVALID_SOCKET, "Socket fd should be valid");
    
    idcu_network_socket_destroy(&sock);
    
    ret = idcu_network_socket_create(&sock, IDCU_NET_PROTO_UDP);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Create UDP socket should succeed");
    IDCU_TEST_ASSERT(sock.protocol == IDCU_NET_PROTO_UDP, "Protocol should be UDP");
    
    idcu_network_socket_destroy(&sock);
    idcu_network_cleanup();
    IDCU_TEST_PASS();
}

static void test_network_server_create_destroy(void) {
    int ret = idcu_network_init();
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Network init should succeed");
    
    idcu_NetworkServer server;
    
    ret = idcu_network_server_create(&server, IDCU_NET_PROTO_TCP, "127.0.0.1", 0);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Create TCP server should succeed");
    IDCU_TEST_ASSERT(server.protocol == IDCU_NET_PROTO_TCP, "Protocol should be TCP");
    IDCU_TEST_ASSERT(server.fd != IDCU_INVALID_SOCKET, "Server fd should be valid");
    
    idcu_network_server_destroy(&server);
    
    ret = idcu_network_server_create(&server, IDCU_NET_PROTO_UDP, "127.0.0.1", 0);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Create UDP server should succeed");
    IDCU_TEST_ASSERT(server.protocol == IDCU_NET_PROTO_UDP, "Protocol should be UDP");
    
    idcu_network_server_destroy(&server);
    idcu_network_cleanup();
    IDCU_TEST_PASS();
}

static void test_network_tcp_server_listen(void) {
    int ret = idcu_network_init();
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Network init should succeed");
    
    idcu_NetworkServer server;
    
    ret = idcu_network_server_create(&server, IDCU_NET_PROTO_TCP, "127.0.0.1", 0);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Create TCP server should succeed");
    
    ret = idcu_network_server_listen(&server);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "TCP server listen should succeed");
    IDCU_TEST_ASSERT(server.listening == 1, "Server should be in listening state");
    
    idcu_network_server_destroy(&server);
    idcu_network_cleanup();
    IDCU_TEST_PASS();
}

static void test_network_socket_bind(void) {
    int ret = idcu_network_init();
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Network init should succeed");
    
    idcu_NetworkSocket sock;
    
    ret = idcu_network_socket_create(&sock, IDCU_NET_PROTO_UDP);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Create UDP socket should succeed");
    
    ret = idcu_network_socket_bind(&sock, "127.0.0.1", 0);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Socket bind should succeed");
    
    idcu_network_socket_destroy(&sock);
    idcu_network_cleanup();
    IDCU_TEST_PASS();
}

static void test_network_udp_sendto_recvfrom(void) {
    int ret = idcu_network_init();
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Network init should succeed");
    
    idcu_NetworkSocket sock1, sock2;
    uint16_t port1 = 0;
    char test_msg[] = "Hello UDP!";
    size_t sent;
    
    ret = idcu_network_socket_create(&sock1, IDCU_NET_PROTO_UDP);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Create UDP socket 1 should succeed");
    
    ret = idcu_network_socket_bind(&sock1, "127.0.0.1", 0);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Socket 1 bind should succeed");
    
    port1 = sock1.local_port;
    IDCU_TEST_ASSERT(port1 != 0, "Bound port should not be 0");
    
    ret = idcu_network_socket_create(&sock2, IDCU_NET_PROTO_UDP);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Create UDP socket 2 should succeed");
    
    ret = idcu_network_socket_sendto(&sock2, test_msg, strlen(test_msg), 
                                      "127.0.0.1", port1, &sent);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "UDP sendto should succeed");
    IDCU_TEST_ASSERT(sent == strlen(test_msg), "Should send all bytes");
    
    idcu_network_socket_destroy(&sock1);
    idcu_network_socket_destroy(&sock2);
    idcu_network_cleanup();
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Network Layer Tests");
    
    idcu_test_suite_add_test(&g_suite, "network_init_cleanup", test_network_init_cleanup);
    idcu_test_suite_add_test(&g_suite, "network_socket_create_destroy", test_network_socket_create_destroy);
    idcu_test_suite_add_test(&g_suite, "network_server_create_destroy", test_network_server_create_destroy);
    idcu_test_suite_add_test(&g_suite, "network_tcp_server_listen", test_network_tcp_server_listen);
    idcu_test_suite_add_test(&g_suite, "network_socket_bind", test_network_socket_bind);
    idcu_test_suite_add_test(&g_suite, "network_udp_sendto_recvfrom", test_network_udp_sendto_recvfrom);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
