#ifndef IDCU_HTTP_CLIENT_TYPES_H
#define IDCU_HTTP_CLIENT_TYPES_H

#include <idcu/common/config.h>
#include <idcu/network/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_HTTP_CLIENT_METHOD_GET = 0,
    IDCU_HTTP_CLIENT_METHOD_POST,
    IDCU_HTTP_CLIENT_METHOD_PUT,
    IDCU_HTTP_CLIENT_METHOD_DELETE,
    IDCU_HTTP_CLIENT_METHOD_HEAD,
    IDCU_HTTP_CLIENT_METHOD_OPTIONS,
    IDCU_HTTP_CLIENT_METHOD_PATCH
} idcu_HttpClientMethod;

#define IDCU_HTTP_CLIENT_HEADER_KEY_MAX    256
#define IDCU_HTTP_CLIENT_HEADER_VALUE_MAX  1024
#define IDCU_HTTP_CLIENT_HEADERS_MAX       64
#define IDCU_HTTP_CLIENT_URL_MAX           2048
#define IDCU_HTTP_CLIENT_VERSION_MAX        32
#define IDCU_HTTP_CLIENT_REASON_MAX         256
#define IDCU_HTTP_CLIENT_HOST_MAX           256
#define IDCU_HTTP_CLIENT_PATH_MAX           1024
#define IDCU_HTTP_CLIENT_QUERY_MAX          1024
#define IDCU_HTTP_CLIENT_BODY_MAX           (64 * 1024)

typedef struct
{
    char key[IDCU_HTTP_CLIENT_HEADER_KEY_MAX];
    char value[IDCU_HTTP_CLIENT_HEADER_VALUE_MAX];
} idcu_HttpClientHeader;

typedef struct
{
    idcu_HttpClientHeader headers[IDCU_HTTP_CLIENT_HEADERS_MAX];
    size_t count;
} idcu_HttpClientHeaders;

typedef struct
{
    idcu_HttpClientMethod method;
    char url[IDCU_HTTP_CLIENT_URL_MAX];
    idcu_HttpClientHeaders headers;
    char* body;
    size_t body_length;
    int timeout_ms;
    int follow_redirects;
    int max_redirects;
    char proxy_host[IDCU_HTTP_CLIENT_HOST_MAX];
    uint16_t proxy_port;
    int use_proxy;
} idcu_HttpClientRequest;

typedef struct
{
    int status_code;
    char reason[IDCU_HTTP_CLIENT_REASON_MAX];
    char version[IDCU_HTTP_CLIENT_VERSION_MAX];
    idcu_HttpClientHeaders headers;
    char* body;
    size_t body_length;
    size_t body_capacity;
    char final_url[IDCU_HTTP_CLIENT_URL_MAX];
    int redirect_count;
} idcu_HttpClientResponse;

typedef struct
{
    idcu_TcpSocket socket;
    int connected;
    int timeout_ms;
    char host[IDCU_HTTP_CLIENT_HOST_MAX];
    uint16_t port;
    int use_ssl;
} idcu_HttpClient;

#ifdef __cplusplus
}
#endif

#endif
