#ifndef IDCU_HTTP-SERVER_HTTP-SERVER_H
#define IDCU_HTTP-SERVER_HTTP-SERVER_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/http-server/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_http-server_init(idcu_Http-Server_Context** ctx);
void idcu_http-server_destroy(idcu_Http-Server_Context* ctx);
int idcu_http-server_is_initialized(idcu_Http-Server_Context* ctx);
uint64_t idcu_http-server_get_operation_count(idcu_Http-Server_Context* ctx);
uint64_t idcu_http-server_get_error_count(idcu_Http-Server_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
