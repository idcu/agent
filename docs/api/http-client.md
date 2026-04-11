# idcu-http-client API Documentation

HTTP client library for making HTTP requests.

## HTTP Client

```c
typedef struct idcu_HttpClient idcu_HttpClient;
```

### Client Functions

```c
int idcu_http_client_init(idcu_HttpClient* client);
void idcu_http_client_destroy(idcu_HttpClient* client);

int idcu_http_client_connect(idcu_HttpClient* client, const char* host, uint16_t port);
int idcu_http_client_connect_timeout(idcu_HttpClient* client, const char* host, uint16_t port, int timeout_ms);
void idcu_http_client_disconnect(idcu_HttpClient* client);

int idcu_http_client_set_timeout(idcu_HttpClient* client, int timeout_ms);
```

## HTTP Request

```c
typedef struct idcu_HttpClientRequest idcu_HttpClientRequest;
```

### Request Functions

```c
int idcu_http_client_request_init(idcu_HttpClientRequest* request);
void idcu_http_client_request_destroy(idcu_HttpClientRequest* request);

int idcu_http_client_request_set_method(idcu_HttpClientRequest* request, idcu_HttpClientMethod method);
int idcu_http_client_request_set_url(idcu_HttpClientRequest* request, const char* url);
int idcu_http_client_request_set_header(idcu_HttpClientRequest* request, const char* name, const char* value);
int idcu_http_client_request_set_body(idcu_HttpClientRequest* request, const void* data, size_t length);
int idcu_http_client_request_set_json(idcu_HttpClientRequest* request, const char* json);
int idcu_http_client_request_set_form(idcu_HttpClientRequest* request, const char* form_data);
int idcu_http_client_request_add_query_param(idcu_HttpClientRequest* request, const char* name, const char* value);
int idcu_http_client_request_set_timeout(idcu_HttpClientRequest* request, int timeout_ms);
int idcu_http_client_request_set_follow_redirects(idcu_HttpClientRequest* request, int follow, int max_redirects);
int idcu_http_client_request_set_proxy(idcu_HttpClientRequest* request, const char* host, uint16_t port);
```

## HTTP Response

```c
typedef struct idcu_HttpClientResponse idcu_HttpClientResponse;
```

### Response Functions

```c
int idcu_http_client_response_init(idcu_HttpClientResponse* response);
void idcu_http_client_response_destroy(idcu_HttpClientResponse* response);

int idcu_http_client_response_get_header(const idcu_HttpClientResponse* response, const char* name, const char** value);
int idcu_http_client_response_get_status_code(const idcu_HttpClientResponse* response);
const char* idcu_http_client_response_get_body(const idcu_HttpClientResponse* response);
size_t idcu_http_client_response_get_body_length(const idcu_HttpClientResponse* response);
```

## HTTP Methods

```c
typedef enum {
    IDCU_HTTP_CLIENT_METHOD_GET,
    IDCU_HTTP_CLIENT_METHOD_POST,
    IDCU_HTTP_CLIENT_METHOD_PUT,
    IDCU_HTTP_CLIENT_METHOD_DELETE,
    IDCU_HTTP_CLIENT_METHOD_HEAD,
    IDCU_HTTP_CLIENT_METHOD_OPTIONS,
    IDCU_HTTP_CLIENT_METHOD_PATCH
} idcu_HttpClientMethod;
```

## Execute Request

```c
int idcu_http_client_execute(idcu_HttpClient* client, const idcu_HttpClientRequest* request, idcu_HttpClientResponse* response);

int idcu_http_client_get(idcu_HttpClient* client, const char* url, idcu_HttpClientResponse* response);
int idcu_http_client_post(idcu_HttpClient* client, const char* url, const void* data, size_t length, idcu_HttpClientResponse* response);
int idcu_http_client_post_json(idcu_HttpClient* client, const char* url, const char* json, idcu_HttpClientResponse* response);
int idcu_http_client_put(idcu_HttpClient* client, const char* url, const void* data, size_t length, idcu_HttpClientResponse* response);
int idcu_http_client_delete(idcu_HttpClient* client, const char* url, idcu_HttpClientResponse* response);
```

## URL Parsing

```c
int idcu_http_url_parse(const char* url, char* host, size_t host_size, uint16_t* port, char* path, size_t path_size, char* query, size_t query_size);
```

## Example

```c
#include <idcu/http_client/http_client.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    idcu_HttpClient client;
    idcu_http_client_init(&client);
    
    // Simple GET request
    idcu_HttpClientResponse response;
    idcu_http_client_response_init(&response);
    
    int ret = idcu_http_client_get(&client, "http://example.com", &response);
    if (ret == IDCU_ERR_OK) {
        int status = idcu_http_client_response_get_status_code(&response);
        printf("Status: %d\n", status);
        
        const char* body = idcu_http_client_response_get_body(&response);
        if (body) {
            printf("Body: %.*s\n", (int)idcu_http_client_response_get_body_length(&response), body);
        }
    }
    
    idcu_http_client_response_destroy(&response);
    
    // Custom POST request
    idcu_HttpClientRequest request;
    idcu_http_client_request_init(&request);
    
    idcu_http_client_request_set_method(&request, IDCU_HTTP_CLIENT_METHOD_POST);
    idcu_http_client_request_set_url(&request, "http://example.com/api");
    idcu_http_client_request_set_json(&request, "{\"key\":\"value\"}");
    
    idcu_http_client_response_init(&response);
    idcu_http_client_execute(&client, &request, &response);
    
    idcu_http_client_request_destroy(&request);
    idcu_http_client_response_destroy(&response);
    
    idcu_http_client_destroy(&client);
    return 0;
}
```
