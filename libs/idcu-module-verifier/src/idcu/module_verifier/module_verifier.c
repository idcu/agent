#include "idcu/module_verifier/module_verifier.h"
#include <stdlib.h>
#include <string.h>

int idcu_module_verifier_init(idcu_ModuleVerifier_Context** ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    *ctx = (idcu_ModuleVerifier_Context*)calloc(1, sizeof(idcu_ModuleVerifier_Context));
    if (!*ctx) {
        return IDCU_ERR_MEMORY;
    }

    idcu_mutex_init(&(*ctx)->lock);
    (*ctx)->initialized = 1;
    (*ctx)->operation_count = 0;
    (*ctx)->error_count = 0;

    return IDCU_ERR_OK;
}

void idcu_module_verifier_destroy(idcu_ModuleVerifier_Context* ctx)
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

int idcu_module_verifier_verify(idcu_ModuleVerifier_Context* ctx, const char* module_path, uint32_t verify_types)
{
    (void)module_path;
    (void)verify_types;

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

int idcu_module_verifier_get_status(idcu_ModuleVerifier_Context* ctx, const char* module_path, idcu_VerifyStatus* status)
{
    (void)module_path;

    if (!ctx || !status) {
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

    *status = IDCU_VERIFY_STATUS_UNKNOWN;
    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_module_verifier_get_checksum(idcu_ModuleVerifier_Context* ctx, const char* module_path, char* checksum, size_t checksum_size)
{
    (void)module_path;

    if (!ctx || !checksum) {
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

    if (checksum_size > 0) {
        checksum[0] = '\0';
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_module_verifier_verify_checksum(idcu_ModuleVerifier_Context* ctx, const char* module_path, const char* expected_checksum)
{
    (void)module_path;
    (void)expected_checksum;

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

int idcu_module_verifier_verify_signature(idcu_ModuleVerifier_Context* ctx, const char* module_path, const char* public_key_path)
{
    (void)module_path;
    (void)public_key_path;

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

int idcu_module_verifier_verify_dependencies(idcu_ModuleVerifier_Context* ctx, const char* module_path)
{
    (void)module_path;

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
