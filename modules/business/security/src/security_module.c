#include "audit_log.h"
#include "idcu/log/log.h"
#include "module_def.h"
#include "tls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_initialized = 0;
static idcu_TLSContext *g_default_tls_ctx = NULL;
static idcu_AuditLog *g_audit_log = NULL;

static int security_module_init() {
    IDCU_LOG_INFO("[security_module] Initializing security module");

    int ret = idcu_tls_init();
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("[security_module] Failed to initialize TLS: %s", idcu_tls_get_error());
        return -1;
    }

    ret = idcu_tls_context_create(&g_default_tls_ctx);
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_ERROR("[security_module] Failed to create default TLS context: %s",
                       idcu_tls_get_error());
        idcu_tls_cleanup();
        return -1;
    }

    idcu_tls_context_set_version(g_default_tls_ctx, IDCU_TLS_VERSION_TLS_1_2);
    idcu_tls_context_set_verify_mode(g_default_tls_ctx, IDCU_TLS_VERIFY_PEER);

    ret = idcu_audit_log_init(&g_audit_log, "logs/audit.log");
    if (ret != IDCU_ERR_OK) {
        IDCU_LOG_WARN("[security_module] Failed to initialize audit log");
    }

    idcu_audit_log_log_simple(g_audit_log, IDCU_AUDIT_LEVEL_INFO, IDCU_AUDIT_ACTION_MODULE_LOAD,
                              "security_module_init", "system", "security_module",
                              "Security module initialized", 1);

    g_initialized = 1;
    IDCU_LOG_INFO("[security_module] Security module initialized successfully");
    return 0;
}

static int security_module_run() { return 0; }

static int security_module_stop() {
    if (g_initialized) {
        IDCU_LOG_INFO("[security_module] Stopping security module");

        if (g_audit_log) {
            idcu_audit_log_log_simple(g_audit_log, IDCU_AUDIT_LEVEL_INFO,
                                      IDCU_AUDIT_ACTION_MODULE_UNLOAD, "security_module_stop",
                                      "system", "security_module", "Security module stopped", 1);
            idcu_audit_log_flush(g_audit_log);
            idcu_audit_log_destroy(g_audit_log);
            g_audit_log = NULL;
        }

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

idcu_TLSContext *idcu_security_module_get_default_tls_context(void) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return NULL;
    }
    return g_default_tls_ctx;
}

int idcu_security_module_create_tls_context(idcu_TLSContext **ctx) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    return idcu_tls_context_create(ctx) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_security_module_destroy_tls_context(idcu_TLSContext *ctx) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    idcu_tls_context_destroy(ctx);
    return 0;
}

int idcu_security_module_create_tls_connection(idcu_TLSConnection **conn, idcu_TLSContext *ctx,
                                               int socket_fd) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    return idcu_tls_connection_create(conn, ctx, socket_fd) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_security_module_tls_handshake(idcu_TLSConnection *conn) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    return idcu_tls_connection_handshake(conn) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_security_module_tls_write(idcu_TLSConnection *conn, const void *data, size_t size,
                                   size_t *written) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    return idcu_tls_connection_write(conn, data, size, written) == IDCU_ERR_OK ? 0 : -1;
}

int idcu_security_module_tls_read(idcu_TLSConnection *conn, void *buffer, size_t size,
                                  size_t *read) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return -1;
    }
    return idcu_tls_connection_read(conn, buffer, size, read) == IDCU_ERR_OK ? 0 : -1;
}

const char *idcu_security_module_get_tls_error(void) { return idcu_tls_get_error(); }

int idcu_security_module_audit_log(idcu_AuditLevel level, idcu_AuditAction action,
                                   const char *event_name, const char *user_name,
                                   const char *resource, const char *details, int success) {
    if (!g_initialized || !g_audit_log) {
        IDCU_LOG_ERROR("[security_module] Module or audit log not initialized");
        return -1;
    }
    return idcu_audit_log_log_simple(g_audit_log, level, action, event_name, user_name, resource,
                                     details, success) == IDCU_ERR_OK
               ? 0
               : -1;
}

idcu_AuditLog *idcu_security_module_get_audit_log(void) {
    if (!g_initialized) {
        IDCU_LOG_ERROR("[security_module] Module not initialized");
        return NULL;
    }
    return g_audit_log;
}

IDCU_REGISTER_MODULE(security_module, IDCU_MODULE_VERSION(1, 0, 0), security_module_init,
                     security_module_run, security_module_stop);
