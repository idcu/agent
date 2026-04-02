#include "context.h"
#include <string.h>

void context_init(Context *ctx, const char *task) {
    memset(ctx, 0, sizeof(Context));
    if (task)
        strncpy(ctx->task, task, sizeof(ctx->task) - 1);
}
