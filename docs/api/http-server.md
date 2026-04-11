# idcu-http-server API 文档

HTTP 服务器框架库。

## 快速开始

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
    printf("服务器运行在 http://localhost:8080\n");
    
    // 等待关闭...
    
    idcu_http_server_stop(server);
    idcu_http_server_destroy(server);
    return 0;
}
```

## 服务器配置

### 服务器配置结构

```c
typedef struct {
    const char* host;
    uint16_t port;
    int max_connections;
    int timeout_ms;
    const char* static_dir;
} idcu_HttpServerConfig;
```

## 服务器生命周期

### 初始化服务器

```c
idcu_ErrorCode idcu_http_server_init(idcu_HttpServer** server, const idcu_HttpServerConfig* config);
```

初始化 HTTP 服务器。

### 启动服务器

```c
idcu_ErrorCode idcu_http_server_start(idcu_HttpServer* server);
```

启动 HTTP 服务器。

### 停止服务器

```c
idcu_ErrorCode idcu_http_server_stop(idcu_HttpServer* server);
```

停止 HTTP 服务器。

### 销毁服务器

```c
void idcu_http_server_destroy(idcu_HttpServer* server);
```

销毁 HTTP 服务器。

## 路由

### 添加路由

```c
idcu_ErrorCode idcu_http_server_add_route(idcu_HttpServer* server, idcu_HttpMethod method, const char* path, idcu_HttpHandler handler, void* user_data);
```

添加路由处理程序。

### 便捷方法

```c
idcu_ErrorCode idcu_http_server_get(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
idcu_ErrorCode idcu_http_server_post(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
idcu_ErrorCode idcu_http_server_put(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
idcu_ErrorCode idcu_http_server_delete(idcu_HttpServer* server, const char* path, idcu_HttpHandler handler, void* user_data);
```

## 请求处理

### 请求类型

```c
typedef struct idcu_HttpRequest idcu_HttpRequest;
```

### 请求方法

```c
idcu_HttpMethod idcu_http_request_get_method(idcu_HttpRequest* req);
const char* idcu_http_request_get_path(idcu_HttpRequest* req);
const char* idcu_http_request_get_query(idcu_HttpRequest* req);
const char* idcu_http_request_get_header(idcu_HttpRequest* req, const char* name);
const void* idcu_http_request_get_body(idcu_HttpRequest* req, size_t* out_size);
const char* idcu_http_request_get_query_param(idcu_HttpRequest* req, const char* name);
```

## 响应处理

### 响应类型

```c
typedef struct idcu_HttpResponse idcu_HttpResponse;
```

### 响应方法

```c
void idcu_http_response_set_status(idcu_HttpResponse* res, int status_code);
void idcu_http_response_set_header(idcu_HttpResponse* res, const char* name, const char* value);
void idcu_http_response_set_content_type(idcu_HttpResponse* res, const char* content_type);
idcu_ErrorCode idcu_http_response_write(idcu_HttpResponse* res, const void* data, size_t size);
idcu_ErrorCode idcu_http_response_write_string(idcu_HttpResponse* res, const char* str);
void idcu_http_response_send_file(idcu_HttpResponse* res, const char* file_path);
void idcu_http_response_redirect(idcu_HttpResponse* res, const char* url, int status_code);
```

## 错误响应

### 发送 404 未找到

```c
void idcu_http_response_not_found(idcu_HttpResponse* res);
```

### 发送 500 内部服务器错误

```c
void idcu_http_response_internal_error(idcu_HttpResponse* res);
```

### 发送 400 错误请求

```c
void idcu_http_response_bad_request(idcu_HttpResponse* res, const char* message);
```

## HTTP 方法

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

## 示例

### 基本服务器

```c
void home_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    idcu_http_response_set_content_type(res, "text/html");
    idcu_http_response_write_string(res, 
        "<html><body><h1>欢迎!</h1></body></html>");
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

### 查询参数

```c
void search_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    const char* query = idcu_http_request_get_query_param(req, "q");
    const char* page = idcu_http_request_get_query_param(req, "page");
    
    char response[256];
    snprintf(response, sizeof(response), 
        "搜索内容: %s (页码: %s)", 
        query ? query : "无", 
        page ? page : "1");
    
    idcu_http_response_write_string(res, response);
}
```

### POST 数据

```c
void submit_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    size_t body_size = 0;
    const void* body = idcu_http_request_get_body(req, &body_size);
    
    if (body && body_size > 0) {
        // 处理请求体...
        idcu_http_response_write_string(res, "数据已接收");
    } else {
        idcu_http_response_bad_request(res, "未接收到数据");
    }
}
```

### 静态文件

```c
idcu_HttpServerConfig config = {
    .host = "0.0.0.0",
    .port = 8080,
    .max_connections = 100,
    .static_dir = "./public"
};

idcu_http_server_init(&server, &config);
// ./public 目录下的文件将被自动提供
```

### 重定向

```c
void old_page_handler(idcu_HttpRequest* req, idcu_HttpResponse* res, void* user_data) {
    idcu_http_response_redirect(res, "/new-page", 301);
}
```
