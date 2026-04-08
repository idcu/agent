/*
 * TCP 服务器示例
 */

#include "idcu/log/log.h"
#include "idcu/network/network_layer.h"
#include <stdio.h>
#include <string.h>

int main() {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    idcu_network_init();

    printf("TCP Server Example\n");
    printf("==================\n");

    idcu_NetworkServer server;
    int ret = idcu_network_server_create(&server, IDCU_NET_PROTO_TCP, "127.0.0.1", 8080);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to create server: %d\n", ret);
        goto cleanup;
    }
    printf("Server created on 127.0.0.1:8080\n");

    ret = idcu_network_server_listen(&server);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to listen: %d\n", ret);
        goto cleanup;
    }
    printf("Server listening...\n");

    printf("Press Ctrl+C to exit\n");

    idcu_NetworkSocket client;
    while (1) {
        ret = idcu_network_server_accept(&server, &client);
        if (ret != IDCU_ERR_OK) {
            printf("Accept failed: %d\n", ret);
            break;
        }
        if (client.fd == IDCU_INVALID_SOCKET) {
            continue;
        }
        printf("Client connected: %s:%d\n", client.remote_addr, client.remote_port);
        idcu_network_socket_close(&client);
    }

cleanup:
    idcu_network_server_destroy(&server);
    idcu_network_cleanup();
    idcu_log_shutdown();

    return 0;
}
