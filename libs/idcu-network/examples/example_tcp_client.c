/*
 * TCP 客户端示例
 */

#include "idcu/log/log.h"
#include "idcu/network/network_layer.h"
#include <stdio.h>
#include <string.h>

int main() {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    idcu_network_init();

    printf("TCP Client Example\n");
    printf("==================\n");

    idcu_NetworkSocket sock;
    int ret = idcu_network_socket_create(&sock, IDCU_NET_PROTO_TCP);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to create socket: %d\n", ret);
        goto cleanup;
    }
    printf("Socket created\n");

    ret = idcu_network_socket_connect(&sock, "127.0.0.1", 8080);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to connect: %d\n", ret);
        goto cleanup;
    }
    printf("Connected to 127.0.0.1:8080\n");
    printf("Local address: %s:%d\n", sock.local_addr, sock.local_port);

    const char *message = "Hello from TCP client!";
    size_t sent;
    ret = idcu_network_socket_send(&sock, message, strlen(message), &sent);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to send: %d\n", ret);
        goto cleanup;
    }
    printf("Sent %zu bytes: %s\n", sent, message);

cleanup:
    idcu_network_socket_destroy(&sock);
    idcu_network_cleanup();
    idcu_log_shutdown();

    return 0;
}
