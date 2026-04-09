#include <idcu/sandbox/sandbox.h>
#include <stdlib.h>
#include <string.h>

int idcu_sandbox_init(idcu_Sandbox_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_sandbox_destroy(idcu_Sandbox_Context* ctx) {
    (void)ctx;
}
