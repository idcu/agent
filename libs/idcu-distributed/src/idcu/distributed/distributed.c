#include <idcu/distributed/distributed.h>
#include <stdlib.h>
#include <string.h>

int idcu_distributed_init(idcu_Distributed_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_distributed_destroy(idcu_Distributed_Context* ctx) {
    (void)ctx;
}
