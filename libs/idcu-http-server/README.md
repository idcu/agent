# idcu-http-server

HTTP 服务器库，提供请求路由和响应处理功能。

## 功能特性

- HTTP 请求解析和响应构建
- 路由注册和匹配（支持通配符）
- 支持 GET、POST、PUT、DELETE 等 HTTP 方法
- JSON 响应支持
- 跨平台支持（Windows/Linux）

## 快速开始

```c
#include "idcu/http_server/http_server.h"

int hello_handler(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    idcu_http_response_set_body(response, "Hello, World!", 13);
    return IDCU_ERR_OK;
}

int main() {
    idcu_HttpServer server;
    idcu_http_server_init(&server, "0.0.0.0", 8080);
    
    idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_GET, "/", hello_handler, NULL);
    
    idcu_http_server_start(&server);
    
    while (1) {
        idcu_http_server_poll(&server, 100);
    }
    
    idcu_http_server_destroy(&server);
    return 0;
}
```

## 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## 安装

```bash
cmake --install .
```

## 依赖

- idcu-common
- idcu-log
- idcu-network
