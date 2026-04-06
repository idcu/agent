# idcu-http-client

HTTP 客户端库，提供请求发送和响应处理功能。

## 功能特性

- HTTP 请求发送
- 响应解析和处理
- 支持 GET、POST、PUT、DELETE、PATCH、HEAD、OPTIONS 等方法
- 自定义请求头和请求体
- 跨平台支持（Windows/Linux）

## 快速开始

```c
#include "idcu/http_client/http_client.h"

int main() {
    idcu_HttpClient client;
    idcu_http_client_init(&client, 30000);
    
    idcu_HttpClientResponse resp;
    idcu_http_client_response_init(&resp);
    
    int ret = idcu_http_client_get(&client, "http://example.com", &resp);
    if (ret == IDCU_ERR_OK) {
        printf("Status: %d\n", resp.status_code);
        printf("Body: %s\n", resp.body);
    }
    
    idcu_http_client_response_destroy(&resp);
    idcu_http_client_destroy(&client);
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
