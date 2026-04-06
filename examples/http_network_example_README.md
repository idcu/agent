# HTTP客户端和服务端网络交互示例

## 功能说明

本示例演示如何使用IDCU的HTTP服务端和客户端库进行网络交互，包括创建RESTful API、处理HTTP请求、发送HTTP请求等功能。

### 主要功能
- HTTP服务端创建和配置
- 多路由注册和处理
- JSON请求/响应处理
- HTTP客户端GET/POST请求
- 自定义请求头
- 多种内容类型支持

## 编译方式

### 编译服务端和客户端示例

```bash
# 使用CMake编译
mkdir -p build && cd build
cmake ..
make http_network_example

# 或直接使用gcc
gcc -o http_network_example http_network_example.c \
    -I./libs/idcu-http-server/include \
    -I./libs/idcu-http-client/include \
    -I./libs/idcu-json/include \
    -I./libs/idcu-log/include \
    -L./build/libs \
    -lidcu-http-server -lidcu-http-client -lidcu-json -lidcu-log
```

## 运行步骤

### 1. 启动HTTP服务端

```bash
# Windows
http_network_example.exe server

# Linux
./http_network_example server
```

服务端会在 `0.0.0.0:8080` 启动，并显示可用路由：
```
Starting HTTP server on port 8080...
Available routes:
  GET  /          - Hello World
  GET  /status    - Server status (JSON)
  POST /echo      - Echo request body
  GET  /api/data  - API data (JSON)

Press Ctrl+C to stop the server
```

### 2. 运行HTTP客户端（在另一个终端）

```bash
# Windows
http_network_example.exe client

# Linux
./http_network_example client
```

客户端会依次测试所有API端点。

### 3. 使用curl手动测试

```bash
# 测试首页
curl http://localhost:8080/

# 测试状态接口
curl http://localhost:8080/status

# 测试echo接口
curl -X POST -d "Hello IDCU" http://localhost:8080/echo

# 测试JSON API
curl http://localhost:8080/api/data
```

## 核心代码解读

### 1. HTTP请求处理函数

```c
int hello_handler(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    idcu_http_response_set_body(response, "Hello, IDCU HTTP Server!", 23);
    idcu_http_response_set_header(response, "Content-Type", "text/plain");
    return IDCU_ERR_OK;
}
```

- 接收HTTP请求
- 设置响应内容
- 设置响应头
- 返回处理结果

### 2. JSON响应处理

```c
int api_data_handler(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    idcu_JsonObject* root = idcu_json_object_create();
    idcu_json_object_add_string(root, "message", "Success");
    idcu_json_object_add_number(root, "status", 200);
    
    idcu_JsonArray* data = idcu_json_array_create();
    idcu_json_array_add_string(data, "item1");
    idcu_json_object_add_array(root, "data", data);
    
    char* json_str = idcu_json_serialize(root);
    idcu_http_response_set_json_body(response, json_str);
    
    free(json_str);
    idcu_json_object_destroy(root);
    return IDCU_ERR_OK;
}
```

- 使用JSON库构建响应对象
- 添加字符串、数字、数组等类型
- 序列化为JSON字符串
- 设置为HTTP响应体

### 3. HTTP服务端初始化

```c
idcu_HttpServer server;
int ret = idcu_http_server_init(&server, "0.0.0.0", SERVER_PORT);

idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_GET, "/", hello_handler, NULL);
idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_GET, "/status", status_handler, NULL);
idcu_http_server_register_route(&server, IDCU_HTTP_METHOD_POST, "/echo", echo_handler, NULL);

ret = idcu_http_server_start(&server);
```

- 初始化服务端，指定监听地址和端口
- 注册多个路由，指定HTTP方法、路径和处理函数
- 启动服务端

### 4. HTTP客户端请求

```c
idcu_HttpClient client;
int ret = idcu_http_client_init(&client);

char response_buffer[MAX_RESPONSE_SIZE];
idcu_HttpResponse* response;

ret = idcu_http_client_get(&client, "http://localhost:8080/", 
                           response_buffer, sizeof(response_buffer), &response);

printf("Status: %d\n", response->status_code);
printf("Body: %.*s\n", (int)response->body_length, response->body);

idcu_http_client_destroy(&client);
```

- 初始化HTTP客户端
- 发送GET请求
- 获取响应状态码和响应体
- 清理资源

### 5. POST请求发送

```c
const char* post_data = "{\"name\":\"IDCU\",\"version\":\"1.0.0\"}";
ret = idcu_http_client_post(&client, "http://localhost:8080/echo", 
                             post_data, strlen(post_data),
                             response_buffer, sizeof(response_buffer), &response);
```

- 准备POST数据
- 发送POST请求
- 获取响应

## 可用API端点

| 方法 | 路径 | 描述 | 返回类型 |
|------|------|------|----------|
| GET | `/` | Hello World | text/plain |
| GET | `/status` | 服务状态 | application/json |
| POST | `/echo` | 回显请求体 | text/plain |
| GET | `/api/data` | 示例数据 | application/json |

## 扩展建议

1. **认证授权**: 添加JWT或API Key认证
2. **HTTPS支持**: 启用TLS加密
3. **限流**: 添加请求频率限制
4. **文件上传**: 支持multipart/form-data文件上传
5. **WebSocket**: 添加WebSocket支持实时通信
6. **Swagger文档**: 自动生成API文档

## 依赖关系

- `idcu-http-server`: HTTP服务端库
- `idcu-http-client`: HTTP客户端库
- `idcu-json`: JSON解析和序列化
- `idcu-log`: 日志系统
- `idcu-common`: 通用工具和错误码
