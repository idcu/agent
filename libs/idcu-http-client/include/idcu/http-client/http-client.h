#ifndef IDCU_HTTP-CLIENT_HTTP-CLIENT_H
#define IDCU_HTTP-CLIENT_HTTP-CLIENT_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/http-client/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_http-client_init(idcu_Http-Client_Context** ctx);
void idcu_http-client_destroy(idcu_Http-Client_Context* ctx);
int idcu_http-client_is_initialized(idcu_Http-Client_Context* ctx);
uint64_t idcu_http-client_get_operation_count(idcu_Http-Client_Context* ctx);
uint64_t idcu_http-client_get_error_count(idcu_Http-Client_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
