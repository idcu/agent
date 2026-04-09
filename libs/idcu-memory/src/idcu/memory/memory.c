#include <idcu/memory/memory.h>
#include <stdlib.h>
#include <string.h>

int idcu_memory_init(idcu_Memory_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_memory_destroy(idcu_Memory_Context* ctx) {
    (void)ctx;
}
