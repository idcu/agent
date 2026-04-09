#include <idcu/module-verifier/module-verifier.h>
#include <stdlib.h>
#include <string.h>

int idcu_module-verifier_init(idcu_Module-Verifier_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_module-verifier_destroy(idcu_Module-Verifier_Context* ctx) {
    (void)ctx;
}
