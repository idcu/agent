#include "scheduler/context.h"
#include "utils/log.h"
#include "common/error_code.h"
#include <time.h>

// 这些函数暂时没有被使用
/*
int idcu_ctx_serialize(const idcu_StackContext* ctx, uint8_t* buf, uint32_t buf_len, uint32_t* out_len) {
    if (!ctx || !buf || !out_len || buf_len < sizeof(idcu_StackContext)) {
        IDCU_LOG_ERROR("idcu_ctx_serialize invalid params");
        return IDCU_ERR_INVALID_PARAM;
    }
    memcpy(buf, ctx, sizeof(idcu_StackContext));
    *out_len = sizeof(idcu_StackContext);
    return IDCU_ERR_SUCCESS;
}

int idcu_ctx_deserialize(const uint8_t* buf, uint32_t buf_len, idcu_StackContext* ctx) {
    if (!buf || !ctx || buf_len < sizeof(idcu_StackContext)) {
        IDCU_LOG_ERROR("idcu_ctx_deserialize invalid params");
        return IDCU_ERR_INVALID_PARAM;
    }
    memcpy(ctx, buf, sizeof(idcu_StackContext));
    return IDCU_ERR_SUCCESS;
}
*/
