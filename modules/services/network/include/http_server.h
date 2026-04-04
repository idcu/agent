#ifndef IDCU_NETWORK_HTTP_SERVER_H
#define IDCU_NETWORK_HTTP_SERVER_H

#include "error_code.h"
#include "network_layer.h"
#include "micro_kernel.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_HTTP_MAX_PATH 256
#define IDCU_HTTP_MAX_METHOD 16
#define IDCU_HTTP_MAX_HEADER 64
#define IDCU_HTTP_MAX_HEADER_VALUE 256
#define IDCU_HTTP_MAX_BODY 4096
#define IDCU_HTTP_MAX_ROUTES 32

typedef enum {
    IDCU_HTTP_METHOD_GET = 0,
    IDCU_HTTP_METHOD_POST,
    IDCU_HTTP_METHOD_PUT,
    IDCU_HTTP_METHOD_DELETE,
    IDCU_HTTP_METHOD_UNKNOWN
} idcu_HttpMethod;

typedef struct {
    char name[IDCU_HTTP_MAX_HEADER];
    char value[IDCU_HTTP_MAX_HEADER_VALUE];
} idcu_HttpHeader;

typedef struct {
    idcu_HttpMethod method;
    char path[IDCU_HTTP_MAX_PATH];
    char version[16];
    idcu_HttpHeader headers[IDCU_HTTP_MAX_HEADER];
    int header_count;
    char body[IDCU_HTTP_MAX_BODY];
    size_t body_length;
} idcu_HttpRequest;

typedef struct {
    int status_code;
    const char* status_text;
    idcu_HttpHeader headers[IDCU_HTTP_MAX_HEADER];
    int header_count;
    char body[IDCU_HTTP_MAX_BODY];
    size_t body_length;
} idcu_HttpResponse;

typedef int (*idcu_HttpHandler)(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data);

typedef struct {
    idcu_HttpMethod method;
    char path[IDCU_HTTP_MAX_PATH];
    idcu_HttpHandler handler;
    void* user_data;
} idcu_HttpRoute;

typedef struct {
    idcu_NetworkServer server;
    idcu_HttpRoute routes[IDCU_HTTP_MAX_ROUTES];
    int route_count;
    idcu_MicroKernel* kernel;
    int running;
} idcu_HttpServer;

int idcu_http_server_init(idcu_HttpServer* server, idcu_MicroKernel* kernel, const char* address, uint16_t port);
void idcu_http_server_destroy(idcu_HttpServer* server);

int idcu_http_server_register_route(idcu_HttpServer* server, idcu_HttpMethod method,
                                    const char* path, idcu_HttpHandler handler, void* user_data);

int idcu_http_server_start(idcu_HttpServer* server);
int idcu_http_server_stop(idcu_HttpServer* server);
int idcu_http_server_poll(idcu_HttpServer* server, int timeout_ms);

void idcu_http_response_init(idcu_HttpResponse* response);
int idcu_http_response_set_status(idcu_HttpResponse* response, int status_code);
int idcu_http_response_add_header(idcu_HttpResponse* response, const char* name, const char* value);
int idcu_http_response_set_body(idcu_HttpResponse* response, const char* body, size_t length);
int idcu_http_response_set_json_body(idcu_HttpResponse* response, const char* json);

idcu_HttpMethod idcu_http_method_from_string(const char* method_str);
const char* idcu_http_status_text(int status_code);

#endif // IDCU_NETWORK_HTTP_SERVER_H
