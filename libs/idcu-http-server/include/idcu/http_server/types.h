#ifndef IDCU_HTTP_SERVER_TYPES_H
#define IDCU_HTTP_SERVER_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/vector.h>
#include <idcu/common/lock.h>
#include <idcu/network/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_HTTP_METHOD_GET = 0,
    IDCU_HTTP_METHOD_POST,
    IDCU_HTTP_METHOD_PUT,
    IDCU_HTTP_METHOD_DELETE,
    IDCU_HTTP_METHOD_HEAD,
    IDCU_HTTP_METHOD_OPTIONS,
    IDCU_HTTP_METHOD_PATCH
} idcu_HttpMethod;

typedef enum
{
    IDCU_HTTP_STATUS_OK = 200,
    IDCU_HTTP_STATUS_CREATED = 201,
    IDCU_HTTP_STATUS_ACCEPTED = 202,
    IDCU_HTTP_STATUS_NO_CONTENT = 204,
    IDCU_HTTP_STATUS_BAD_REQUEST = 400,
    IDCU_HTTP_STATUS_UNAUTHORIZED = 401,
    IDCU_HTTP_STATUS_FORBIDDEN = 403,
    IDCU_HTTP_STATUS_NOT_FOUND = 404,
    IDCU_HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
    IDCU_HTTP_STATUS_INTERNAL_SERVER_ERROR = 500,
    IDCU_HTTP_STATUS_NOT_IMPLEMENTED = 501,
    IDCU_HTTP_STATUS_SERVICE_UNAVAILABLE = 503
} idcu_HttpStatus;

#define IDCU_HTTP_HEADER_KEY_MAX    256
#define IDCU_HTTP_HEADER_VALUE_MAX  1024
#define IDCU_HTTP_HEADERS_MAX       64
#define IDCU_HTTP_QUERY_KEY_MAX     128
#define IDCU_HTTP_QUERY_VALUE_MAX   512
#define IDCU_HTTP_QUERY_PARAMS_MAX  32
#define IDCU_HTTP_PATH_MAX           1024
#define IDCU_HTTP_VERSION_MAX        32
#define IDCU_HTTP_REASON_MAX         256
#define IDCU_HTTP_BODY_MAX           (64 * 1024)
#define IDCU_HTTP_CLIENT_IP_MAX      64

typedef struct
{
    char key[IDCU_HTTP_HEADER_KEY_MAX];
    char value[IDCU_HTTP_HEADER_VALUE_MAX];
} idcu_HttpHeader;

typedef struct
{
    idcu_HttpHeader headers[IDCU_HTTP_HEADERS_MAX];
    size_t count;
} idcu_HttpHeaders;

typedef struct
{
    char key[IDCU_HTTP_QUERY_KEY_MAX];
    char value[IDCU_HTTP_QUERY_VALUE_MAX];
} idcu_HttpQueryParam;

typedef struct
{
    idcu_HttpQueryParam params[IDCU_HTTP_QUERY_PARAMS_MAX];
    size_t count;
} idcu_HttpQueryParams;

typedef struct
{
    idcu_HttpMethod method;
    char path[IDCU_HTTP_PATH_MAX];
    char version[IDCU_HTTP_VERSION_MAX];
    idcu_HttpHeaders headers;
    idcu_HttpQueryParams query_params;
    char* body;
    size_t body_length;
    char client_ip[IDCU_HTTP_CLIENT_IP_MAX];
    uint16_t client_port;
} idcu_HttpRequest;

typedef struct
{
    idcu_HttpStatus status;
    char reason[IDCU_HTTP_REASON_MAX];
    char version[IDCU_HTTP_VERSION_MAX];
    idcu_HttpHeaders headers;
    char* body;
    size_t body_length;
    size_t body_capacity;
} idcu_HttpResponse;

typedef void (*idcu_HttpHandler)(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data);

typedef struct
{
    idcu_HttpMethod method;
    char path[IDCU_HTTP_PATH_MAX];
    idcu_HttpHandler handler;
    void* user_data;
} idcu_HttpRoute;

typedef struct
{
    idcu_Vector routes;
    idcu_TcpServer server;
    idcu_Mutex lock;
    int running;
    int initialized;
    char static_dir[IDCU_HTTP_PATH_MAX];
    int serve_static;
} idcu_HttpServer;

#ifdef __cplusplus
}
#endif

#endif
