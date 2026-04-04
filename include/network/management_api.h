#ifndef IDCU_NETWORK_MANAGEMENT_API_H
#define IDCU_NETWORK_MANAGEMENT_API_H

#include "network/http_server.h"
#include "common/error_code.h"

#ifdef __cplusplus
extern "C" {
#endif

int idcu_management_api_register_routes(idcu_HttpServer* server);

#ifdef __cplusplus
}
#endif

#endif // IDCU_NETWORK_MANAGEMENT_API_H
