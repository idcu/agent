#ifndef IDCU_SCHEDULER_CONTEXT_H
#define IDCU_SCHEDULER_CONTEXT_H

#include <stdint.h>
#include <string.h>
#include "config.h"

typedef struct {
    uint32_t module_id;
    uint32_t msg_id;
    uint32_t ts;
    uint8_t  data[IDCU_CONFIG_STACK_CTX_SIZE];
    uint16_t len;
} idcu_StackContext;

static inline void idcu_ctx_init(idcu_StackContext *ctx, uint32_t mid, uint32_t msg)
{
    memset(ctx, 0, sizeof(idcu_StackContext));
    ctx->module_id = mid;
    ctx->msg_id = msg;
}

#endif // IDCU_SCHEDULER_CONTEXT_H
