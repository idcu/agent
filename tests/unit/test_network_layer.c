#include "test_framework.h"
#include "network_layer.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static void test_network_socket_create_destroy(void) {
    NetworkSocket sock;
    int ret = network_socket_create(&sock, NET_PROTO_TCP);
    TEST_ASSERT(ret == ERR_OK, "Create TCP socket should succeed");
    TEST_ASSERT(sock.protocol == NET_PROTO_TCP, "Protocol should be TCP");
    
    network_socket_destroy(&sock);
    
    ret = network_socket_create(&sock, NET_PROTO_UDP);
    TEST_ASSERT(ret == ERR_OK, "Create UDP socket should succeed");
    TEST_ASSERT(sock.protocol == NET_PROTO_UDP, "Protocol should be UDP");
    
    network_socket_destroy(&sock);
    TEST_PASS();
}

static void test_network_server_create_destroy(void) {
    NetworkServer server;
    int ret = network_server_create(&server, NET_PROTO_TCP, "127.0.0.1", 9000);
    TEST_ASSERT(ret == ERR_OK, "Create TCP server should succeed");
    TEST_ASSERT(server.protocol == NET_PROTO_TCP, "Protocol should be TCP");
    TEST_ASSERT(strcmp(server.bind_address, "127.0.0.1") == 0, "Bind address should be 127.0.0.1");
    TEST_ASSERT(server.bind_port == 9000, "Bind port should be 9000");
    
    network_server_destroy(&server);
    
    ret = network_server_create(&server, NET_PROTO_UDP, "127.0.0.1", 9001);
    TEST_ASSERT(ret == ERR_OK, "Create UDP server should succeed");
    TEST_ASSERT(server.protocol == NET_PROTO_UDP, "Protocol should be UDP");
    
    network_server_destroy(&server);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Network Layer Tests");
    
    test_suite_add_test(&g_suite, "network_socket_create_destroy", test_network_socket_create_destroy);
    test_suite_add_test(&g_suite, "network_server_create_destroy", test_network_server_create_destroy);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
