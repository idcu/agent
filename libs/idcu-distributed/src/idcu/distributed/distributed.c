#include "idcu/distributed/distributed.h"
#include <stdlib.h>
#include <string.h>

int idcu_distributed_node_init(idcu_Distributed_Context** ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    *ctx = (idcu_Distributed_Context*)calloc(1, sizeof(idcu_Distributed_Context));
    if (!*ctx) {
        return IDCU_ERR_MEMORY;
    }

    idcu_mutex_init(&(*ctx)->lock);
    (*ctx)->initialized = 1;
    (*ctx)->node_count = 0;
    (*ctx)->self_id = 0;
    (*ctx)->operation_count = 0;
    (*ctx)->error_count = 0;

    return IDCU_ERR_OK;
}

int idcu_distributed_node_start(idcu_Distributed_Context* ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_distributed_node_stop(idcu_Distributed_Context* ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

void idcu_distributed_node_destroy(idcu_Distributed_Context* ctx)
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

int idcu_distributed_send_to(idcu_Distributed_Context* ctx, idcu_NodeId target_id, const void* data, size_t data_size)
{
    if (!ctx || !data || data_size == 0) {
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

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_distributed_broadcast(idcu_Distributed_Context* ctx, const void* data, size_t data_size)
{
    if (!ctx || !data || data_size == 0) {
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

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_distributed_get_leader(idcu_Distributed_Context* ctx, idcu_NodeId* leader_id)
{
    if (!ctx || !leader_id) {
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

    *leader_id = 0;
    for (int i = 0; i < ctx->node_count; i++) {
        if (ctx->nodes[i].state == IDCU_NODE_STATE_LEADER) {
            *leader_id = ctx->nodes[i].id;
            break;
        }
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_distributed_get_node_info(idcu_Distributed_Context* ctx, idcu_NodeId node_id, idcu_NodeInfo* info)
{
    if (!ctx || !info) {
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

    int found = 0;
    for (int i = 0; i < ctx->node_count; i++) {
        if (ctx->nodes[i].id == node_id) {
            *info = ctx->nodes[i];
            found = 1;
            break;
        }
    }

    idcu_mutex_unlock(&ctx->lock);

    if (!found) {
        return IDCU_ERR_NOT_FOUND;
    }

    return IDCU_ERR_OK;
}

int idcu_distributed_list_nodes(idcu_Distributed_Context* ctx, idcu_NodeInfo* nodes, size_t max_nodes, size_t* actual_count)
{
    if (!ctx || !nodes || !actual_count) {
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

    *actual_count = (size_t)ctx->node_count < max_nodes ? (size_t)ctx->node_count : max_nodes;
    for (size_t i = 0; i < *actual_count; i++) {
        nodes[i] = ctx->nodes[i];
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}
