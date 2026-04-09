#include <idcu/metrics/metrics.h>
#include <stdlib.h>
#include <string.h>

int idcu_metrics_init(idcu_Metrics_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_metrics_destroy(idcu_Metrics_Context* ctx) {
    (void)ctx;
}
