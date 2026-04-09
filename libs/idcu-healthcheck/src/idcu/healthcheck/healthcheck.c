#include <idcu/healthcheck/healthcheck.h>
#include <stdlib.h>
#include <string.h>

int idcu_healthcheck_init(idcu_Healthcheck_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_healthcheck_destroy(idcu_Healthcheck_Context* ctx) {
    (void)ctx;
}
