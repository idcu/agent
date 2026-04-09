#include <idcu/server-monitor/server-monitor.h>
#include <stdlib.h>
#include <string.h>

int idcu_server-monitor_init(idcu_Server-Monitor_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_server-monitor_destroy(idcu_Server-Monitor_Context* ctx) {
    (void)ctx;
}
