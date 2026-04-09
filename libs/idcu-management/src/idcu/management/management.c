#include <idcu/management/management.h>
#include <stdlib.h>
#include <string.h>

int idcu_management_init(idcu_Management_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_management_destroy(idcu_Management_Context* ctx) {
    (void)ctx;
}
