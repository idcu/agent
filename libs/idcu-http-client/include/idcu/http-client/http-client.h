#ifndef IDCU_HTTP-CLIENT_HTTP-CLIENT_H
#define IDCU_HTTP-CLIENT_HTTP-CLIENT_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/http-client/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_http-client_init(idcu_Http-Client_Context** ctx);
void idcu_http-client_destroy(idcu_Http-Client_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
