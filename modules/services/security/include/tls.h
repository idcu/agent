#ifndef IDCU_SECURITY_TLS_H
#define IDCU_SECURITY_TLS_H

#include "idcu/common/error_code.h"

#include <stddef.h>
#include <stdint.h>

#define IDCU_TLS_CERT_PATH_MAX 512
#define IDCU_TLS_KEY_PATH_MAX  512
#define IDCU_TLS_CA_PATH_MAX   512

typedef enum
{
    IDCU_TLS_VERSION_TLS_1_0 = 0,
    IDCU_TLS_VERSION_TLS_1_1,
    IDCU_TLS_VERSION_TLS_1_2,
    IDCU_TLS_VERSION_TLS_1_3
} idcu_TLSVersion;

typedef enum
{
    IDCU_TLS_VERIFY_NONE = 0,
    IDCU_TLS_VERIFY_PEER,
    IDCU_TLS_VERIFY_PEER_STRICT
} idcu_TLSVerifyMode;

typedef struct idcu_TLSContext    idcu_TLSContext;
typedef struct idcu_TLSConnection idcu_TLSConnection;

int  idcu_tls_init(void);
void idcu_tls_cleanup(void);

int  idcu_tls_context_create(idcu_TLSContext** ctx);
void idcu_tls_context_destroy(idcu_TLSContext* ctx);

int idcu_tls_context_set_version(idcu_TLSContext* ctx, idcu_TLSVersion version);
int idcu_tls_context_set_verify_mode(idcu_TLSContext* ctx, idcu_TLSVerifyMode mode);

int idcu_tls_context_load_ca_cert(idcu_TLSContext* ctx, const char* ca_path);
int idcu_tls_context_load_cert(idcu_TLSContext* ctx, const char* cert_path);
int idcu_tls_context_load_private_key(idcu_TLSContext* ctx, const char* key_path);

int  idcu_tls_connection_create(idcu_TLSConnection** conn, idcu_TLSContext* ctx, int socket_fd);
void idcu_tls_connection_destroy(idcu_TLSConnection* conn);

int idcu_tls_connection_handshake(idcu_TLSConnection* conn);
int idcu_tls_connection_write(idcu_TLSConnection* conn, const void* data, size_t size,
                              size_t* written);
int idcu_tls_connection_read(idcu_TLSConnection* conn, void* buffer, size_t size, size_t* read);
int idcu_tls_connection_shutdown(idcu_TLSConnection* conn);

const char* idcu_tls_get_error(void);

#endif  // IDCU_SECURITY_TLS_H
