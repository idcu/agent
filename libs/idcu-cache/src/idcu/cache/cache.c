#include <idcu/cache/cache.h>
#include <stdlib.h>
#include <string.h>

int idcu_cache_init(idcu_Cache_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_cache_destroy(idcu_Cache_Context* ctx) {
    (void)ctx;
}
