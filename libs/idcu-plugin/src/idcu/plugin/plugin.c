#include <idcu/plugin/plugin.h>
#include <stdlib.h>
#include <string.h>

int idcu_plugin_init(idcu_Plugin_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_plugin_destroy(idcu_Plugin_Context* ctx) {
    (void)ctx;
}
