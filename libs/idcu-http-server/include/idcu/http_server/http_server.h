#ifndef IDCU_HTTP_SERVER_HTTP_SERVER_H
#define IDCU_HTTP_SERVER_HTTP_SERVER_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/http_server/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int  idcu_http_server_init(idcu_HttpServer* server);
void idcu_http_server_destroy(idcu_HttpServer* server);
int  idcu_http_server_listen(idcu_HttpServer* server, const char* host, uint16_t port, int backlog);
void idcu_http_server_stop(idcu_HttpServer* server);
int  idcu_http_server_set_static_dir(idcu_HttpServer* server, const char* dir);

int idcu_http_server_add_route(idcu_HttpServer* server, idcu_HttpMethod method, const char* path, idcu_HttpHandler handler, void* user_data);
int idcu_http_server_get(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
int idcu_http_server_post(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
int idcu_http_server_put(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
int idcu_http_server_delete(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);

int  idcu_http_request_init(idcu_HttpRequest* request);
void idcu_http_request_destroy(idcu_HttpRequest* request);
int  idcu_http_request_get_header(const idcu_HttpRequest* request, const char* name, const char** value);
int  idcu_http_request_get_query_param(const idcu_HttpRequest* request, const char* name, const char** value);
const char* idcu_http_method_to_string(idcu_HttpMethod method);

int  idcu_http_response_init(idcu_HttpResponse* response);
void idcu_http_response_destroy(idcu_HttpResponse* response);
void idcu_http_response_set_status(idcu_HttpResponse* response, idcu_HttpStatus status);
int  idcu_http_response_set_header(idcu_HttpResponse* response, const char* name, const char* value);
int  idcu_http_response_set_body(idcu_HttpResponse* response, const void* data, size_t length);
int  idcu_http_response_append_body(idcu_HttpResponse* response, const void* data, size_t length);
int  idcu_http_response_set_string(idcu_HttpResponse* response, const char* str);
int  idcu_http_response_set_json(idcu_HttpResponse* response, const char* json);
void idcu_http_response_send_error(idcu_HttpResponse* response, idcu_HttpStatus status, const char* message);
const char* idcu_http_status_to_string(idcu_HttpStatus status);

#ifdef __cplusplus
}
#endif

#endif
