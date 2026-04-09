#include <idcu/http-client/http-client.h>
#include <stdlib.h>
#include <string.h>

int idcu_http-client_init(idcu_Http-Client_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_http-client_destroy(idcu_Http-Client_Context* ctx) {
    (void)ctx;
}
