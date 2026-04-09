#include <idcu/http-server/http-server.h>
#include <stdlib.h>
#include <string.h>

int idcu_http-server_init(idcu_Http-Server_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_http-server_destroy(idcu_Http-Server_Context* ctx) {
    (void)ctx;
}
