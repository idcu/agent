#ifndef IDCU_REST_API_TYPES_H
#define IDCU_REST_API_TYPES_H

#include <idcu/common/config.h>
#include <idcu/http_server/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_API_AUTH_NONE = 0,
    IDCU_API_AUTH_BASIC,
    IDCU_API_AUTH_BEARER,
    IDCU_API_AUTH_API_KEY
} idcu_ApiAuthType;

typedef struct {
    idcu_ApiAuthType type;
    char username[256];
    char password[256];
    char bearer_token[512];
    char api_key[512];
    char api_key_header[128];
} idcu_ApiAuthConfig;

typedef struct {
    bool enabled;
    uint64_t requests_per_second;
    uint64_t burst_size;
} idcu_ApiRateLimitConfig;

typedef struct {
    bool enabled;
    char allowed_origins[1024];
    char allowed_methods[256];
    char allowed_headers[512];
    bool allow_credentials;
    uint64_t max_age;
} idcu_ApiCorsConfig;

typedef uint64_t idcu_ApiEndpointId;

typedef void (*idcu_ApiHandler)(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data);

typedef struct {
    idcu_ApiEndpointId id;
    char path[512];
    idcu_HttpMethod method;
    idcu_ApiHandler handler;
    void* user_data;
    idcu_ApiAuthConfig auth;
    idcu_ApiRateLimitConfig rate_limit;
    char description[1024];
    char version[32];
} idcu_ApiEndpoint;

typedef struct idcu_RestApi idcu_RestApi;

#ifdef __cplusplus
}
#endif

#endif
