# idcu-http-server API Documentation

HTTP server framework library.

## Quick Start

```c
#include <idcu/http_server/http_server.h>

void hello_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    idcu_http_response_set_status(res, 200);
    idcu_http_response_set_content_type(res, "text/plain");
    idcu_http_response_write(res, "Hello, World!", 13);
}

int main() {
    idcu_HttpServer* server = NULL;
    idcu_HttpServerConfig config = {
        .host = "0.0.0.0",
        .port = 8080,
        .max_connections = 100
    };
    
    idcu_http_server_init(&server, &config);
    idcu_http_server_add_route(server, IDCU_HTTP_GET, "/", hello_handler, NULL);
    
    idcu_http_server_start(server);
    printf("Server running on http://localhost:8080\n");
    
    // Wait for shutdown...
    
    idcu_http_server_stop(server);
    idcu_http_server_destroy(server);
    return 0;
}
```

## Server Configuration

### Server Config Structure

```c
typedef struct {
    const char* host;
    uint16_t port;
    int max_connections;
    int timeout_ms;
    const char* static_dir;
} idcu_HttpServerConfig;
```

## Server Lifecycle

### Initialize Server

```c
idcu_ErrorCode idcu_http_server_init(idcu_HttpServer** server, const idcu_HttpServerConfig* config);
```

Initialize an HTTP server.

### Start Server

```c
idcu_ErrorCode idcu_http_server_start(idcu_HttpServer* server);
```

Start the HTTP server.

### Stop Server

```c
idcu_ErrorCode idcu_http_server_stop(idcu_HttpServer* server);
```

Stop the HTTP server.

### Destroy Server

```c
void idcu_http_server_destroy(idcu_HttpServer* server);
```

Destroy the HTTP server.

## Routing

### Add Route

```c
idcu_ErrorCode idcu_http_server_add_route(idcu_HttpServer* server, idcu_HttpMethod method, const char* path, idcu_HttpHandler handler, void* user_data);
```

Add a route handler.

### Convenience Methods

```c
idcu_ErrorCode idcu_http_server_get(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
idcu_ErrorCode idcu_http_server_post(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
idcu_ErrorCode idcu_http_server_put(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
idcu_ErrorCode idcu_http_server_delete(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
```

## Request Handling

### Request Type

```c
typedef struct idcu_HttpRequest idcu_HttpRequest;
```

### Request Methods

```c
idcu_HttpMethod idcu_http_request_get_method(idcu_HttpRequest* req);
const char* idcu_http_request_get_path(idcu_HttpRequest* req);
const char* idcu_http_request_get_query(idcu_HttpRequest* req);
const char* idcu_http_request_get_header(idcu_HttpRequest* req, const char* name);
const void* idcu_http_request_get_body(idcu_HttpRequest* req, size_t* out_size);
const char* idcu_http_request_get_query_param(idcu_HttpRequest* req, const char* name);
```

## Response Handling

### Response Type

```c
typedef struct idcu_HttpResponse idcu_HttpResponse;
```

### Response Methods

```c
void idcu_http_response_set_status(idcu_HttpResponse* res, int status_code);
void idcu_http_response_set_header(idcu_HttpResponse* res, const char* name, const char* value);
void idcu_http_response_set_content_type(idcu_HttpResponse* res, const char* content_type);
idcu_ErrorCode idcu_http_response_write(idcu_HttpResponse* res, const void* data, size_t size);
idcu_ErrorCode idcu_http_response_write_string(idcu_HttpResponse* res, const char* str);
void idcu_http_response_send_file(idcu_HttpResponse* res, const char* file_path);
void idcu_http_response_redirect(idcu_HttpResponse* res, const char* url, int status_code);
```

## Error Responses

### Send 404 Not Found

```c
void idcu_http_response_not_found(idcu_HttpResponse* res);
```

### Send 500 Internal Server Error

```c
void idcu_http_response_internal_error(idcu_HttpResponse* res);
```

### Send 400 Bad Request

```c
void idcu_http_response_bad_request(idcu_HttpResponse* res, const char* message);
```

## HTTP Methods

```c
typedef enum {
    IDCU_HTTP_GET,
    IDCU_HTTP_POST,
    IDCU_HTTP_PUT,
    IDCU_HTTP_DELETE,
    IDCU_HTTP_HEAD,
    IDCU_HTTP_OPTIONS,
    IDCU_HTTP_PATCH
} idcu_HttpMethod;
```

## Examples

### Basic Server

```c
void home_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    idcu_http_response_set_content_type(res, "text/html");
    idcu_http_response_write_string(res, 
        "<html><body><h1>Welcome!</h1></body></html>");
}

idcu_HttpServer* server = NULL;
idcu_HttpServerConfig config = {
    .host = "127.0.0.1",
    .port = 3000,
    .max_connections = 50
};

idcu_http_server_init(&server, &config);
idcu_http_server_get(server, "/", home_handler, NULL);
idcu_http_server_start(server);
```

### JSON API

```c
#include <idcu/json/json.h>

void api_user_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    idcu_JsonValue* user = idcu_json_object_create();
    idcu_json_object_set_string(user, "name", "John Doe");
    idcu_json_object_set_int(user, "age", 30);
    
    char* json_str = idcu_json_serialize(user);
    idcu_http_response_set_content_type(res, "application/json");
    idcu_http_response_write_string(res, json_str);
    
    free(json_str);
    idcu_json_value_destroy(user);
}
```

### Query Parameters

```c
void search_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    const char* query = idcu_http_request_get_query_param(req, "q");
    const char* page = idcu_http_request_get_query_param(req, "page");
    
    char response[256];
    snprintf(response, sizeof(response), 
        "Searching for: %s (page: %s)", 
        query ? query : "none", 
        page ? page : "1");
    
    idcu_http_response_write_string(res, response);
}
```

### POST Data

```c
void submit_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    size_t body_size = 0;
    const void* body = idcu_http_request_get_body(req, &body_size);
    
    if (body && body_size > 0) {
        // Process body...
        idcu_http_response_write_string(res, "Data received");
    } else {
        idcu_http_response_bad_request(res, "No data received");
    }
}
```

### Static Files

```c
idcu_HttpServerConfig config = {
    .host = "0.0.0.0",
    .port = 8080,
    .max_connections = 100,
    .static_dir = "./public"
};

idcu_http_server_init(&server, &config);
// Files in ./public will be served automatically
```

### Redirect

```c
void old_page_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    idcu_http_response_redirect(res, "/new-page", 301);
}
```
