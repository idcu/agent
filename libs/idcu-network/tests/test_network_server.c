#include "idcu/log/log.h"
#include "idcu/network/network_layer.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    printf("Testing network server operations...\n");

    idcu_network_init();

    idcu_NetworkServer server;

    int ret = idcu_network_server_create(&server, IDCU_NET_PROTO_TCP, "127.0.0.1", 0);
    assert(ret == IDCU_ERR_OK);
    assert(server.protocol == IDCU_NET_PROTO_TCP);
    assert(server.fd != IDCU_INVALID_SOCKET);
    printf("✓ TCP server created\n");

    ret = idcu_network_server_listen(&server);
    assert(ret == IDCU_ERR_OK);
    assert(server.listening == 1);
    printf("✓ TCP server listening\n");

    idcu_network_server_destroy(&server);
    printf("✓ TCP server destroyed\n");

    ret = idcu_network_server_create(&server, IDCU_NET_PROTO_UDP, "127.0.0.1", 0);
    assert(ret == IDCU_ERR_OK);
    assert(server.protocol == IDCU_NET_PROTO_UDP);
    assert(server.fd != IDCU_INVALID_SOCKET);
    printf("✓ UDP server created\n");

    idcu_network_server_destroy(&server);
    printf("✓ UDP server destroyed\n");

    idcu_network_cleanup();
    printf("All tests passed!\n");
    return 0;
}
