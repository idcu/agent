#ifndef IDCU_REST_API_REST_API_H
#define IDCU_REST_API_REST_API_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/rest_api/types.h>
#include <idcu/http_server/http_server.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_rest_api_init(idcu_RestApi** api);
void idcu_rest_api_destroy(idcu_RestApi* api);

int idcu_rest_api_listen(idcu_RestApi* api, const char* host, uint16_t port, int backlog);
void idcu_rest_api_stop(idcu_RestApi* api);

idcu_ApiEndpointId idcu_rest_api_add_endpoint(idcu_RestApi* api,
                                               const char* path,
                                               idcu_HttpMethod method,
                                               idcu_ApiHandler handler,
                                               void* user_data);

int idcu_rest_api_set_auth(idcu_RestApi* api, idcu_ApiEndpointId id, const idcu_ApiAuthConfig* config);
int idcu_rest_api_set_rate_limit(idcu_RestApi* api, idcu_ApiEndpointId id, const idcu_ApiRateLimitConfig* config);
int idcu_rest_api_set_cors(idcu_RestApi* api, const idcu_ApiCorsConfig* config);
int idcu_rest_api_set_description(idcu_RestApi* api, idcu_ApiEndpointId id, const char* description);
int idcu_rest_api_set_version(idcu_RestApi* api, idcu_ApiEndpointId id, const char* version);

int idcu_rest_api_get(idcu_RestApi* api, const char* path, idcu_ApiHandler handler, void* user_data);
int idcu_rest_api_post(idcu_RestApi* api, const char* path, idcu_ApiHandler handler, void* user_data);
int idcu_rest_api_put(idcu_RestApi* api, const char* path, idcu_ApiHandler handler, void* user_data);
int idcu_rest_api_delete(idcu_RestApi* api, const char* path, idcu_ApiHandler handler, void* user_data);

int idcu_rest_api_generate_openapi(idcu_RestApi* api, char* buffer, size_t buffer_size);
char* idcu_rest_api_generate_openapi_alloc(idcu_RestApi* api);

int idcu_rest_api_response_json(idcu_HttpResponse* response, const char* json);
int idcu_rest_api_response_error(idcu_HttpResponse* response, idcu_HttpStatus status, const char* message);
int idcu_rest_api_response_success(idcu_HttpResponse* response, const char* data);

#ifdef __cplusplus
}
#endif

#endif
