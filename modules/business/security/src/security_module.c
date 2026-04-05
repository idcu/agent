#include "module_def.h"
#include "tls.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_initialized = 0;
static idcu_TLSContext* g_default_tls_ctx = NULL;

static int security_module_init() {
    IDCU_LOG_INFO("[security_module] Initializing security module");
    
    int ret = idcu_tls_init();
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("[security_module] Failed to initialize TLS: %s", idcu_tls_get_error());
        return -1;
    }

    ret = idcu_tls_context_create(&g_default_tls_ctx);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("[security_module] Failed to create default TLS context: %s", idcu_tls_get_error());
        idcu_tls_cleanup();
        return -1;
    }

    idcu_tls_context_set_version(g_default_tls_ctx, IDCU_TLS_VERSION_TLS_1_2);
    idcu_tls_context_set_verify_mode(g_default_tls_ctx, IDCU_TLS_VERIFY_PEER);

    g_initialized = 1;
    IDCU_LOG_INFO("[security_module] Security module initialized successfully");
    return 0;
}

static int security_module_run() {
    return 0;
}

static int security_module_stop() {
    if (g_initialized) {
        IDCU_LOG_INFO("[security_module] Stopping security module");
        
        if (g_default_tls_ctx) {
            idcu_tls_context_destroy(g_default_tls_ctx);
            g_default_tls_ctx = NULL;
        }
        
        idcu_tls_cleanup();
        g_initialized = 0;
    }
    return 0;
}

int idcu_security_module_tls_init(void) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    return idcu_tls_init() == IDCU_ERR_OK ? 0 : -1;
}

idcu_TLSContext* idcu_security_module_get_default_tls_context(void) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return NULL;
    }
    return g_default_tls_ctx;
}

int idcu_security_module_create_tls_context(idcu_TLSContext** ctx) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    return idcu_tls_context_create(ctx) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_security_module_destroy_tls_context(idcu_TLSContext* ctx) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    idcu_tls_context_destroy(ctx);
    return 0;
}

int idcu_security_module_create_tls_connection(idcu_TLSConnection** conn, idcu_TLSContext* ctx, int socket_fd) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    return idcu_tls_connection_create(conn, ctx, socket_fd) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_security_module_tls_handshake(idcu_TLSConnection* conn) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    return idcu_tls_connection_handshake(conn) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_security_module_tls_write(idcu_TLSConnection* conn, const void* data, size_t size, size_t* written) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    return idcu_tls_connection_write(conn, data, size, written) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_security_module_tls_read(idcu_TLSConnection* conn, void* buffer, size_t size, size_t* read) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    return idcu_tls_connection_read(conn, buffer, size, read) == IDCU_ERR_OK ? 0 : -1;
}

const char* idcu_security_module_get_tls_error(void) {
    return idcu_tls_get_error();
}

IDCU_REGISTER_MODULE(security_module, IDCU_MODULE_VERSION(1, 0, 0), 
                     security_module_init, security_module_run, security_module_stop);
