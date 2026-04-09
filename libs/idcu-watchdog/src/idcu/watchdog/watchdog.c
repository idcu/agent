#include <idcu/watchdog/watchdog.h>
#include <stdlib.h>
#include <string.h>

int idcu_watchdog_init(idcu_Watchdog_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_watchdog_destroy(idcu_Watchdog_Context* ctx) {
    (void)ctx;
}
