#include <idcu/network/network.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void example_tcp_server(void) {
    printf("\n--- TCP Server Example ---\n");
    
    idcu_NetAddress addr;
    idcu_TcpServer server;
    idcu_TcpSocket client;
    char buffer[1024];
    size_t received;
    
    int ret = idcu_network_init();
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init network: %s\n", idcu_error_message(ret));
        return;
    }
    
    ret = idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "127.0.0.1", 8080);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init address: %s\n", idcu_error_message(ret));
        idcu_network_cleanup();
        return;
    }
    
    ret = idcu_tcp_server_init(&server);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init server: %s\n", idcu_error_message(ret));
        idcu_network_cleanup();
        return;
    }
    
    ret = idcu_tcp_server_listen(&server, &addr, 5);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to listen: %s\n", idcu_error_message(ret));
        idcu_tcp_server_destroy(&server);
        idcu_network_cleanup();
        return;
    }
    
    printf("Server listening on 127.0.0.1:8080\n");
    printf("Note: This is a demonstration. In real use, you would run server in a separate thread.\n");
    
    idcu_tcp_server_destroy(&server);
    idcu_network_cleanup();
}

static void example_tcp_client(void) {
    printf("\n--- TCP Client Example ---\n");
    
    idcu_NetAddress addr;
    idcu_TcpSocket sock;
    const char* message = "Hello from IDCU Network!";
    char buffer[1024];
    size_t sent, received;
    
    int ret = idcu_network_init();
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init network: %s\n", idcu_error_message(ret));
        return;
    }
    
    ret = idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "127.0.0.1", 8080);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init address: %s\n", idcu_error_message(ret));
        idcu_network_cleanup();
        return;
    }
    
    ret = idcu_tcp_socket_init(&sock);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init socket: %s\n", idcu_error_message(ret));
        idcu_network_cleanup();
        return;
    }
    
    printf("TCP client initialized. Would connect to server and send: %s\n", message);
    
    idcu_tcp_socket_destroy(&sock);
    idcu_network_cleanup();
}

static void example_udp(void) {
    printf("\n--- UDP Example ---\n");
    
    idcu_NetAddress addr;
    idcu_UdpSocket sock;
    const char* message = "UDP Hello!";
    char buffer[1024];
    size_t received;
    
    int ret = idcu_network_init();
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init network: %s\n", idcu_error_message(ret));
        return;
    }
    
    ret = idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "127.0.0.1", 9090);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init address: %s\n", idcu_error_message(ret));
        idcu_network_cleanup();
        return;
    }
    
    ret = idcu_udp_socket_init(&sock);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init UDP socket: %s\n", idcu_error_message(ret));
        idcu_network_cleanup();
        return;
    }
    
    printf("UDP socket initialized. Would bind and send: %s\n", message);
    
    idcu_udp_socket_destroy(&sock);
    idcu_network_cleanup();
}

static void example_address_resolution(void) {
    printf("\n--- Address Resolution Example ---\n");
    
    idcu_NetAddress addr;
    char addr_str[64];
    
    int ret = idcu_network_init();
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init network: %s\n", idcu_error_message(ret));
        return;
    }
    
    printf("Resolving localhost...\n");
    ret = idcu_net_address_resolve(&addr, "localhost", 80);
    if (ret == IDCU_ERR_OK) {
        ret = idcu_net_address_to_string(&addr, addr_str, sizeof(addr_str));
        if (ret == IDCU_ERR_OK) {
            printf("Resolved address: %s\n", addr_str);
        }
    } else {
        printf("Resolution failed (expected in demo): %s\n", idcu_error_message(ret));
    }
    
    idcu_network_cleanup();
}

int main(void) {
    printf("=== IDCU Network Example ===\n");
    
    example_tcp_server();
    example_tcp_client();
    example_udp();
    example_address_resolution();
    
    printf("\n=== Network Example Complete ===\n");
    return 0;
}
