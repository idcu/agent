#include <idcu/module-isolation/module-isolation.h>
#include <stdlib.h>
#include <string.h>

int idcu_module-isolation_init(idcu_Module-Isolation_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_module-isolation_destroy(idcu_Module-Isolation_Context* ctx) {
    (void)ctx;
}
