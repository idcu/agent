#include <idcu/conn-pool/conn-pool.h>
#include <stdlib.h>
#include <string.h>

int idcu_conn-pool_init(idcu_Conn-Pool_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_conn-pool_destroy(idcu_Conn-Pool_Context* ctx) {
    (void)ctx;
}
