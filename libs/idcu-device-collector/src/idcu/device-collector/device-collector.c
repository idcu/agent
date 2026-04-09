#include <idcu/device-collector/device-collector.h>
#include <stdlib.h>
#include <string.h>

int idcu_device-collector_init(idcu_Device-Collector_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_device-collector_destroy(idcu_Device-Collector_Context* ctx) {
    (void)ctx;
}
