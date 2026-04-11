/**
 * @file example_network.c
 * @brief Network programming example (TCP client/server)
 */

#include <idcu/network/network.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TCP Server example
int run_tcp_server(void) {
    printf("=== TCP Server Example ===\n\n");

    idcu_NetAddress addr;
    idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, "127.0.0.1", 8888);

    idcu_TcpServer server;
    idcu_tcp_server_init(&server);

    printf("Server listening on 127.0.0.1:8888...\n");
    printf("(This is a simplified example - actual server would run in a loop)\n");

    // In a real server, you would:
    // 1. Listen for connections
    // 2. Accept incoming connections
    // 3. Handle client communication

    idcu_tcp_server_destroy(&server);
    printf("\nServer stopped\n");
    return 0;
}

// TCP Client example
int run_tcp_client(void) {
    printf("\n=== TCP Client Example ===\n\n");

    idcu_TcpSocket sock;
    idcu_tcp_socket_init(&sock);

    // Resolve address
    idcu_NetAddress addr;
    int ret = idcu_net_address_resolve(&addr, "example.com", 80);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to resolve address\n");
        idcu_tcp_socket_destroy(&sock);
        return 1;
    }

    // Print address info
    char addr_str[64];
    idcu_net_address_to_string(&addr, addr_str, sizeof(addr_str));
    printf("Connecting to %s:80\n", addr_str);

    // Set timeout (5 seconds)
    idcu_tcp_socket_set_timeout(&sock, 5000);

    printf("Note: This is a demonstration only\n");
    printf("To actually connect, you would need a running server\n");

    idcu_tcp_socket_destroy(&sock);
    return 0;
}

// UDP example
int run_udp_example(void) {
    printf("\n=== UDP Example ===\n\n");

    idcu_UdpSocket sock;
    idcu_udp_socket_init(&sock);

    printf("UDP socket created\n");
    printf("UDP can be used for datagram-based communication\n");

    // In a real application, you would:
    // 1. Bind the socket to a local address
    // 2. Send/receive datagrams

    idcu_udp_socket_destroy(&sock);
    return 0;
}

int main(void) {
    printf("=== Network Library Examples ===\n\n");

    // Initialize network library
    int ret = idcu_network_init();
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to initialize network library\n");
        return 1;
    }

    // Run examples
    run_tcp_server();
    run_tcp_client();
    run_udp_example();

    // Cleanup
    idcu_network_cleanup();

    printf("\n=== All examples completed ===\n");
    return 0;
}
