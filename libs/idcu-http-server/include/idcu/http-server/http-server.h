#ifndef IDCU_HTTP-SERVER_HTTP-SERVER_H
#define IDCU_HTTP-SERVER_HTTP-SERVER_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/http-server/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_http-server_init(idcu_Http-Server_Context** ctx);
void idcu_http-server_destroy(idcu_Http-Server_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
