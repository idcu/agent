#include "test/test_framework.h"
#include "network/network_layer.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_network_socket_create_destroy(void) {
    idcu_NetworkSocket sock;
    int ret = idcu_network_socket_create(&sock, IDCU_NET_PROTO_TCP);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Create TCP socket should succeed");
    IDCU_TEST_ASSERT(sock.protocol == IDCU_NET_PROTO_TCP, "Protocol should be TCP");
    
    idcu_network_socket_destroy(&sock);
    
    ret = idcu_network_socket_create(&sock, IDCU_NET_PROTO_UDP);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Create UDP socket should succeed");
    IDCU_TEST_ASSERT(sock.protocol == IDCU_NET_PROTO_UDP, "Protocol should be UDP");
    
    idcu_network_socket_destroy(&sock);
    IDCU_TEST_PASS();
}

static void test_network_server_create_destroy(void) {
    idcu_NetworkServer server;
    int ret = idcu_network_server_create(&server, IDCU_NET_PROTO_TCP, "127.0.0.1", 9000);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Create TCP server should succeed");
    IDCU_TEST_ASSERT(server.protocol == IDCU_NET_PROTO_TCP, "Protocol should be TCP");
    IDCU_TEST_ASSERT(strcmp(server.bind_address, "127.0.0.1") == 0, "Bind address should be 127.0.0.1");
    IDCU_TEST_ASSERT(server.bind_port == 9000, "Bind port should be 9000");
    
    idcu_network_server_destroy(&server);
    
    ret = idcu_network_server_create(&server, IDCU_NET_PROTO_UDP, "127.0.0.1", 9001);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Create UDP server should succeed");
    IDCU_TEST_ASSERT(server.protocol == IDCU_NET_PROTO_UDP, "Protocol should be UDP");
    
    idcu_network_server_destroy(&server);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Network Layer Tests");
    
    idcu_test_suite_add_test(&g_suite, "network_socket_create_destroy", test_network_socket_create_destroy);
    idcu_test_suite_add_test(&g_suite, "network_server_create_destroy", test_network_server_create_destroy);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
