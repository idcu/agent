#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>
#include <string.h>
#include "common/config.h"

typedef struct {
    uint32_t module_id;
    uint32_t msg_id;
    uint32_t ts;
    uint8_t  data[CONFIG_STACK_CTX_SIZE];
    uint16_t len;
} StackContext;

static inline void ctx_init(StackContext *ctx, uint32_t mid, uint32_t msg)
{
    memset(ctx, 0, sizeof(StackContext));
    ctx->module_id = mid;
    ctx->msg_id = msg;
}

#endif