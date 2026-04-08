#include "idcu/http_server/http_server.h"
#include "idcu/log/log.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    printf("Testing HTTP server initialization...\n");

    idcu_HttpServer server;
    int ret = idcu_http_server_init(&server, "127.0.0.1", 8888);
    assert(ret == IDCU_ERR_OK);
    printf("✓ HTTP server initialized successfully\n");

    idcu_http_server_destroy(&server);
    printf("✓ HTTP server destroyed successfully\n");

    printf("All tests passed!\n");
    return 0;
}
