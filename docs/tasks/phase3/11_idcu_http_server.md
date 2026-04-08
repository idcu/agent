# 任务 3.11: idcu-http-server - HTTP 服务器库

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建轻量级 HTTP 服务器库，支持 HTTP/1.1 协议、路由处理、GET/POST/PUT/DELETE 等方法、请求响应处理、查询参数解析、JSON 支持、静态文件服务，满足请求处理延迟 ≤ 10ms、支持 1000+ 并发连接、QPS ≥ 5000 的性能要求。

### 1.2 不做什么
- 不实现 HTTPS（由外部 SSL 库处理）
- 不实现 HTTP/2
- 不实现 WebSocket
- 不实现缓存机制
- 不实现认证/授权

### 1.3 输入
- HTTP 请求（原始字节流）
- 路由注册信息
- 静态文件目录
- 监听地址和端口

### 1.4 输出
- HTTP 响应（原始字节流）
- 路由处理器回调
- 错误码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用
- idcu-network 网络库已可用
- idcu-log 日志库已可用
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **协议**: HTTP/1.1
- **网络层**: 基于 idcu-network
- **路由**: Vector 存储路由，线性匹配
- **解析**: 手动解析 HTTP 请求
- **并发模型**: 每个连接一个线程或事件循环

### 2.2 核心逻辑
```
服务器启动流程：
1. 创建 TCP 服务器 Socket
2. 绑定地址和端口
3. 开始监听
4. 进入接受连接循环

连接处理流程：
1. 接受新连接
2. 读取 HTTP 请求
3. 解析请求行、头部、Body
4. 匹配路由
5. 调用路由处理器
6. 生成响应
7. 发送响应
8. 关闭连接或保持 Keep-Alive

请求解析流程：
1. 读取请求行（Method Path Version）
2. 读取头部直到空行
3. 根据 Content-Length 读取 Body
4. 解析查询参数

响应生成流程：
1. 设置状态码
2. 设置响应头部
3. 设置响应 Body
4. 格式化 HTTP 响应
```

### 2.3 数据结构/接口
```c
// 主要头文件：idcu/http/http_server.h

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

typedef struct
{
    char key[256];
    char value[1024];
} idcu_HttpHeader;

typedef struct
{
    idcu_HttpHeader headers[64];
    size_t count;
} idcu_HttpHeaders;

typedef struct
{
    char key[128];
    char value[512];
} idcu_HttpQueryParam;

typedef struct
{
    idcu_HttpQueryParam params[32];
    size_t count;
} idcu_HttpQueryParams;

typedef struct
{
    idcu_HttpMethod method;
    char path[1024];
    char version[32];
    idcu_HttpHeaders headers;
    idcu_HttpQueryParams query_params;
    char* body;
    size_t body_length;
    char client_ip[64];
    uint16_t client_port;
} idcu_HttpRequest;

typedef struct
{
    idcu_HttpStatus status;
    char reason[256];
    char version[32];
    idcu_HttpHeaders headers;
    char* body;
    size_t body_length;
    size_t body_capacity;
} idcu_HttpResponse;

typedef void (*idcu_HttpHandler)(const idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data);

typedef struct
{
    idcu_Vector routes;
    idcu_TcpServer server;
    idcu_Mutex lock;
    int running;
    int initialized;
    char static_dir[1024];
    int serve_static;
} idcu_HttpServer;

// 核心 API
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
```

### 2.4 跨平台适配
- **网络层**: 基于 idcu-network 统一封装
- **文件操作**: 使用标准 C 文件 API 或跨平台封装
- **路径分隔符**: Windows 用 \，Linux/macOS 用 /，内部统一处理

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 服务器可以监听端口
- [ ] 可以注册和访问路由
- [ ] GET/POST/PUT/DELETE 方法正常工作
- [ ] 查询参数正确解析
- [ ] 请求头正确解析
- [ ] 响应头正确设置
- [ ] 可以返回 JSON 响应
- [ ] 静态文件服务正常工作
- [ ] 错误响应正确返回
- [ ] 跨平台正常运行（Windows + Linux）

### 3.2 性能验收
- 请求处理延迟 ≤ 10ms（简单响应）
- 支持 1000+ 并发连接
- QPS ≥ 5000
- 内存占用 ≤ 20MB（1000 连接）

### 3.3 异常验收
- [ ] 无效请求返回 400
- [ ] 未找到路由返回 404
- [ ] 方法不允许返回 405
- [ ] 服务器错误返回 500
- [ ] NULL 指针检查正确
- [ ] 超大请求体正确处理

---

## 4. 执行计划

### 4.1 工期
5 小时/人

### 4.2 里程碑
- D1-00: 完成数据结构和头文件定义（1 小时）
- D1-60: 完成请求解析（1 小时）
- D1-120: 完成响应生成（45 分钟）
- D1-165: 完成路由匹配（45 分钟）
- D1-210: 完成静态文件服务（30 分钟）
- D1-240: 完成单元测试（60 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + HTTP 协议知识）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，前缀 idcu_
- 枚举前缀 IDCU_HTTP_

### 5.2 测试要求
- 单元测试覆盖率 ≥ 75%
- 性能测试验证 QPS 和延迟
- 跨平台测试（Windows + Linux）

### 5.3 部署指引
- 编译命令：`cmake -B build &amp;&amp; cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::http-server)`

---

## 6. 风险与应对

### 6.1 风险1
描述：HTTP 解析存在安全漏洞  
应对：严格限制输入大小，进行边界检查，模糊测试

### 6.2 风险2
描述：高并发下性能瓶颈  
应对：使用事件循环或线程池，优化内存分配

---

## 7. 详细实现步骤

### 1. 创建目录结构
```bash
mkdir -p libs/idcu-http-server/include/idcu/http
mkdir -p libs/idcu-http-server/src/idcu/http
mkdir -p libs/idcu-http-server/tests
mkdir -p libs/idcu-http-server/examples
```

### 2. 创建 HTTP 服务器头文件 (http_server.h)

### 3. 创建 CMakeLists.txt

### 4. 创建模块配置文件 (module.yaml)

### 5. 创建 README.md

---

## 8. 验证检查清单

- [ ] HTTP 服务器头文件已创建
- [ ] HTTP 服务器实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 服务器可以监听端口
- [ ] 可以注册和访问路由
- [ ] 可以返回 JSON 响应
- [ ] 单元测试通过
- [ ] 跨平台测试通过
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-http-server/
git commit -m "feat: add idcu-http-server library

- Add HTTP/1.1 protocol support
- Add route-based request handling
- Add GET/POST/PUT/DELETE/HEAD/OPTIONS/PATCH methods
- Add HTTP header parsing and setting
- Add query parameter parsing
- Add JSON request/response support
- Add static file serving
- Add form data parsing
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 端口被占用 | 端口已被其他程序使用 | 更换端口或关闭占用程序 |
| 路由不匹配 | 路径错误 | 确保路径完全匹配 |
| 响应错误 | 未设置 Content-Type | 确保设置正确的 Content-Type |
| 静态文件 404 | 目录路径错误 | 检查 static_dir 配置 |
