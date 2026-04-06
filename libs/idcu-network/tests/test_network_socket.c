#include "idcu/network/network_layer.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("Testing network socket operations...\n");

    idcu_network_init();

    idcu_NetworkSocket sock;

    int ret = idcu_network_socket_create(&sock, IDCU_NET_PROTO_TCP);
    assert(ret == IDCU_ERR_OK);
    assert(sock.protocol == IDCU_NET_PROTO_TCP);
    assert(sock.fd != IDCU_INVALID_SOCKET);
    printf("✓ TCP socket created\n");

    idcu_network_socket_destroy(&sock);
    printf("✓ TCP socket destroyed\n");

    ret = idcu_network_socket_create(&sock, IDCU_NET_PROTO_UDP);
    assert(ret == IDCU_ERR_OK);
    assert(sock.protocol == IDCU_NET_PROTO_UDP);
    assert(sock.fd != IDCU_INVALID_SOCKET);
    printf("✓ UDP socket created\n");

    idcu_network_socket_destroy(&sock);
    printf("✓ UDP socket destroyed\n");

    idcu_network_cleanup();
    printf("All tests passed!\n");
    return 0;
}
