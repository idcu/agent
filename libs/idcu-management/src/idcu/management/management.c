#include "idcu/management/management.h"
#include <stdlib.h>
#include <string.h>

int idcu_mgmt_init(idcu_Management_Context** ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    *ctx = (idcu_Management_Context*)calloc(1, sizeof(idcu_Management_Context));
    if (!*ctx) {
        return IDCU_ERR_MEMORY;
    }

    idcu_mutex_init(&(*ctx)->lock);
    (*ctx)->initialized = 1;
    (*ctx)->operation_count = 0;
    (*ctx)->error_count = 0;

    return IDCU_ERR_OK;
}

void idcu_mgmt_destroy(idcu_Management_Context* ctx)
{
    if (!ctx) {
        return;
    }

    idcu_mutex_lock(&ctx->lock);
    idcu_Mutex lock_copy = ctx->lock;
    ctx->initialized = 0;
    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);

    free(ctx);
}

int idcu_mgmt_execute(idcu_Management_Context* ctx, idcu_MgmtCommand cmd, const char* args, idcu_MgmtResponse* response)
{
    if (!ctx || !response) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    response->code = IDCU_ERR_OK;
    response->message = NULL;
    response->data = NULL;
    response->data_size = 0;

    switch (cmd) {
        case IDCU_MGMT_CMD_STATUS:
            response->message = "System status: OK";
            break;
        case IDCU_MGMT_CMD_MODULE_LIST:
            response->message = "Module list: [not implemented]";
            break;
        case IDCU_MGMT_CMD_MODULE_START:
            response->message = "Module start: [not implemented]";
            break;
        case IDCU_MGMT_CMD_MODULE_STOP:
            response->message = "Module stop: [not implemented]";
            break;
        case IDCU_MGMT_CMD_CONFIG_GET:
            response->message = "Config get: [not implemented]";
            break;
        case IDCU_MGMT_CMD_CONFIG_SET:
            response->message = "Config set: [not implemented]";
            break;
        case IDCU_MGMT_CMD_METRICS_GET:
            response->message = "Metrics get: [not implemented]";
            break;
        case IDCU_MGMT_CMD_HEALTH_CHECK:
            response->message = "Health check: [not implemented]";
            break;
        default:
            response->code = IDCU_ERR_INVALID_ARG;
            response->message = "Unknown command";
            ctx->error_count++;
            break;
    }

    idcu_mutex_unlock(&ctx->lock);
    return response->code;
}

void idcu_mgmt_response_destroy(idcu_MgmtResponse* response)
{
    if (!response) {
        return;
    }

    if (response->message) {
        free(response->message);
        response->message = NULL;
    }

    if (response->data) {
        free(response->data);
        response->data = NULL;
    }

    response->code = 0;
    response->data_size = 0;
}

int idcu_mgmt_cli_start(int port)
{
    (void)port;
    return IDCU_ERR_OK;
}

void idcu_mgmt_cli_stop(void)
{
}

int idcu_mgmt_http_start(int port)
{
    (void)port;
    return IDCU_ERR_OK;
}

void idcu_mgmt_http_stop(void)
{
}
