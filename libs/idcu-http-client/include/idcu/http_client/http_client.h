#ifndef IDCU_HTTP_CLIENT_H
#define IDCU_HTTP_CLIENT_H

#include "idcu/common/error_code.h"
#include "idcu/network/network_layer.h"

#include <stddef.h>
#include <stdint.h>

#define IDCU_HTTPC_URL_MAX           512
#define IDCU_HTTPC_HEADER_KEY_MAX    128
#define IDCU_HTTPC_HEADER_VALUE_MAX  512
#define IDCU_HTTPC_MAX_HEADERS       32
#define IDCU_HTTPC_BODY_MAX          65536
#define IDCU_HTTPC_METHOD_MAX        16
#define IDCU_HTTPC_RESPONSE_BUF_SIZE 8192

typedef enum
{
    IDCU_HTTPC_METHOD_GET = 0,
    IDCU_HTTPC_METHOD_POST,
    IDCU_HTTPC_METHOD_PUT,
    IDCU_HTTPC_METHOD_DELETE,
    IDCU_HTTPC_METHOD_PATCH,
    IDCU_HTTPC_METHOD_HEAD,
    IDCU_HTTPC_METHOD_OPTIONS
} idcu_HttpClientMethod;

typedef struct
{
    char key[IDCU_HTTPC_HEADER_KEY_MAX];
    char value[IDCU_HTTPC_HEADER_VALUE_MAX];
} idcu_HttpClientHeader;

typedef struct
{
    idcu_HttpClientMethod method;
    char                  url[IDCU_HTTPC_URL_MAX];
    idcu_HttpClientHeader headers[IDCU_HTTPC_MAX_HEADERS];
    size_t                header_count;
    char*                 body;
    size_t                body_length;
} idcu_HttpClientRequest;

typedef struct
{
    int                   status_code;
    char                  status_text[128];
    idcu_HttpClientHeader headers[IDCU_HTTPC_MAX_HEADERS];
    size_t                header_count;
    char                  body[IDCU_HTTPC_BODY_MAX];
    size_t                body_length;
} idcu_HttpClientResponse;

typedef struct
{
    idcu_NetworkSocket socket;
    char               host[IDCU_ADDR_MAX];
    uint16_t           port;
    int                connected;
    int                timeout_ms;
} idcu_HttpClient;

int  idcu_http_client_init(idcu_HttpClient* client, int timeout_ms);
void idcu_http_client_destroy(idcu_HttpClient* client);

int  idcu_http_client_request_init(idcu_HttpClientRequest* req, idcu_HttpClientMethod method,
                                   const char* url);
void idcu_http_client_request_destroy(idcu_HttpClientRequest* req);
int  idcu_http_client_request_add_header(idcu_HttpClientRequest* req, const char* key,
                                         const char* value);
int idcu_http_client_request_set_body(idcu_HttpClientRequest* req, const char* body, size_t length);

int         idcu_http_client_response_init(idcu_HttpClientResponse* resp);
void        idcu_http_client_response_destroy(idcu_HttpClientResponse* resp);
const char* idcu_http_client_response_get_header(idcu_HttpClientResponse* resp, const char* key);

int idcu_http_client_execute(idcu_HttpClient* client, idcu_HttpClientRequest* req,
                             idcu_HttpClientResponse* resp);

int idcu_http_client_get(idcu_HttpClient* client, const char* url, idcu_HttpClientResponse* resp);
int idcu_http_client_post(idcu_HttpClient* client, const char* url, const char* body,
                          size_t body_len, idcu_HttpClientResponse* resp);

const char* idcu_http_client_method_to_string(idcu_HttpClientMethod method);

#endif
