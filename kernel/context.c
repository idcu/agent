#include "context.h"
#include "log.h"
#include "error_code.h"
#include <time.h>

int ctx_serialize(const StackContext* ctx, uint8_t* buf, uint32_t buf_len, uint32_t* out_len) {
    if (!ctx || !buf || !out_len || buf_len < sizeof(StackContext)) {
        LOG_ERROR("ctx_serialize invalid params");
        return ERR_INVALID_PARAM;
    }
    memcpy(buf, ctx, sizeof(StackContext));
    *out_len = sizeof(StackContext);
    return ERR_SUCCESS;
}

int ctx_deserialize(const uint8_t* buf, uint32_t buf_len, StackContext* ctx) {
    if (!buf || !ctx || buf_len < sizeof(StackContext)) {
        LOG_ERROR("ctx_deserialize invalid params");
        return ERR_INVALID_PARAM;
    }
    memcpy(ctx, buf, sizeof(StackContext));
    return ERR_SUCCESS;
}