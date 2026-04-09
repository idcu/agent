#include <idcu/permission/permission.h>
#include <stdlib.h>
#include <string.h>

int idcu_permission_init(idcu_Permission_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_permission_destroy(idcu_Permission_Context* ctx) {
    (void)ctx;
}
