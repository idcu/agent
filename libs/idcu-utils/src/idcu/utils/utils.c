#include <idcu/utils/utils.h>
#include <stdlib.h>
#include <string.h>

int idcu_utils_init(idcu_Utils_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_utils_destroy(idcu_Utils_Context* ctx) {
    (void)ctx;
}
