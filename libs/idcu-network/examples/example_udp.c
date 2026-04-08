/*
 * UDP 示例
 */

#include "idcu/log/log.h"
#include "idcu/network/network_layer.h"
#include <stdio.h>
#include <string.h>

int main() {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    idcu_network_init();

    printf("UDP Example\n");
    printf("===========\n");

    idcu_NetworkSocket sock1, sock2;
    int ret;

    ret = idcu_network_socket_create(&sock1, IDCU_NET_PROTO_UDP);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to create socket 1: %d\n", ret);
        goto cleanup;
    }

    ret = idcu_network_socket_bind(&sock1, "127.0.0.1", 0);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to bind socket 1: %d\n", ret);
        goto cleanup;
    }
    printf("Socket 1 bound to %s:%d\n", sock1.local_addr, sock1.local_port);

    ret = idcu_network_socket_create(&sock2, IDCU_NET_PROTO_UDP);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to create socket 2: %d\n", ret);
        goto cleanup;
    }

    const char *message = "Hello UDP!";
    size_t sent;
    ret = idcu_network_socket_sendto(&sock2, message, strlen(message), "127.0.0.1",
                                     sock1.local_port, &sent);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to send: %d\n", ret);
        goto cleanup;
    }
    printf("Sent %zu bytes: %s\n", sent, message);

cleanup:
    idcu_network_socket_destroy(&sock1);
    idcu_network_socket_destroy(&sock2);
    idcu_network_cleanup();
    idcu_log_shutdown();

    return 0;
}
