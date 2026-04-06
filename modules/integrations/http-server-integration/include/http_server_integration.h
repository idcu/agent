#ifndef IDCU_INTEGRATIONS_HTTP_SERVER_INTEGRATION_H
#define IDCU_INTEGRATIONS_HTTP_SERVER_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/http_server/http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int initialized;
    idcu_HttpServer server;
    char bind_address[256];
    uint16_t port;
} idcu_HttpServerIntegration;

int idcu_http_server_integration_init(idcu_HttpServerIntegration* integration,
                                        const char* bind_address,
                                        uint16_t port);
void idcu_http_server_integration_destroy(idcu_HttpServerIntegration* integration);

int idcu_http_server_integration_start(idcu_HttpServerIntegration* integration);
int idcu_http_server_integration_stop(idcu_HttpServerIntegration* integration);
int idcu_http_server_integration_poll(idcu_HttpServerIntegration* integration, int timeout_ms);

idcu_HttpServer* idcu_http_server_integration_get_server(idcu_HttpServerIntegration* integration);

#ifdef __cplusplus
}
#endif

#endif // IDCU_INTEGRATIONS_HTTP_SERVER_INTEGRATION_H
