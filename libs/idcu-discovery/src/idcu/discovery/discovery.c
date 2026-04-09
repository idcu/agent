#include <idcu/discovery/discovery.h>
#include <stdlib.h>
#include <string.h>

int idcu_discovery_init(idcu_Discovery_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_discovery_destroy(idcu_Discovery_Context* ctx) {
    (void)ctx;
}
