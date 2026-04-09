#include <idcu/storage/storage.h>
#include <stdlib.h>
#include <string.h>

int idcu_storage_init(idcu_Storage_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_storage_destroy(idcu_Storage_Context* ctx) {
    (void)ctx;
}
