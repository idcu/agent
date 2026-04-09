#include <idcu/alert/alert.h>
#include <stdlib.h>
#include <string.h>

int idcu_alert_init(idcu_Alert_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_alert_destroy(idcu_Alert_Context* ctx) {
    (void)ctx;
}
