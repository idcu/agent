# 任务 3.12: idcu-http-client - HTTP 客户端库

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建轻量级 HTTP 客户端库，支持 HTTP/1.1 协议、GET/POST/PUT/DELETE 等方法、请求头设置、查询参数、请求体（表单、JSON）、响应解析、超时设置、代理支持、重定向跟随，满足请求延迟 ≤ 100ms（局域网）、支持 100+ 并发请求、吞吐量 ≥ 1000 QPS 的性能要求。

### 1.2 不做什么
- 不实现 HTTPS（由外部 SSL 库处理）
- 不实现 HTTP/2
- 不实现 WebSocket
- 不实现 Cookie 管理
- 不实现连接池（由上层处理）

### 1.3 输入
- URL
- HTTP 方法
- 请求头
- 请求体
- 超时设置
- 代理配置

### 1.4 输出
- HTTP 响应状态码
- 响应头
- 响应体
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
- **URL 解析**: 手动解析 URL 组件
- **请求构建**: 手动格式化 HTTP 请求
- **响应解析**: 逐行解析 HTTP 响应

### 2.2 核心逻辑
```
请求执行流程：
1. 解析 URL（主机、端口、路径、查询参数）
2. 建立 TCP 连接
3. 构建 HTTP 请求
4. 发送请求
5. 接收响应
6. 解析响应（状态码、头部、Body）
7. 关闭连接
8. 返回结果

URL 解析流程：
1. 提取协议（http://）
2. 提取主机名
3. 提取端口（默认 80）
4. 提取路径
5. 提取查询参数

重定向处理流程：
1. 检查响应状态码（3xx）
2. 提取 Location 头部
3. 解析新 URL
4. 递归执行请求（直到 max_redirects）
```

### 2.3 数据结构/接口
```c
// 主要头文件：idcu/http/http_client.h

typedef enum
{
    IDCU_HTTP_CLIENT_METHOD_GET = 0,
    IDCU_HTTP_CLIENT_METHOD_POST,
    IDCU_HTTP_CLIENT_METHOD_PUT,
    IDCU_HTTP_CLIENT_METHOD_DELETE,
    IDCU_HTTP_CLIENT_METHOD_HEAD,
    IDCU_HTTP_CLIENT_METHOD_OPTIONS,
    IDCU_HTTP_CLIENT_METHOD_PATCH
} idcu_HttpClientMethod;

typedef struct
{
    char key[256];
    char value[1024];
} idcu_HttpClientHeader;

typedef struct
{
    idcu_HttpClientHeader headers[64];
    size_t count;
} idcu_HttpClientHeaders;

typedef struct
{
    idcu_HttpClientMethod method;
    char url[2048];
    idcu_HttpClientHeaders headers;
    char* body;
    size_t body_length;
    int timeout_ms;
    int follow_redirects;
    int max_redirects;
    char proxy_host[256];
    uint16_t proxy_port;
    int use_proxy;
} idcu_HttpClientRequest;

typedef struct
{
    int status_code;
    char reason[256];
    char version[32];
    idcu_HttpClientHeaders headers;
    char* body;
    size_t body_length;
    size_t body_capacity;
    char final_url[2048];
    int redirect_count;
} idcu_HttpClientResponse;

typedef struct
{
    idcu_TcpSocket socket;
    int connected;
    int timeout_ms;
    char host[256];
    uint16_t port;
    int use_ssl;
} idcu_HttpClient;

// 核心 API
int  idcu_http_client_init(idcu_HttpClient* client);
void idcu_http_client_destroy(idcu_HttpClient* client);
int  idcu_http_client_connect(idcu_HttpClient* client, const char* host, uint16_t port);
int  idcu_http_client_connect_timeout(idcu_HttpClient* client, const char* host, uint16_t port, int timeout_ms);
void idcu_http_client_disconnect(idcu_HttpClient* client);
int  idcu_http_client_set_timeout(idcu_HttpClient* client, int timeout_ms);

int  idcu_http_client_request_init(idcu_HttpClientRequest* request);
void idcu_http_client_request_destroy(idcu_HttpClientRequest* request);
int  idcu_http_client_request_set_method(idcu_HttpClientRequest* request, idcu_HttpClientMethod method);
int  idcu_http_client_request_set_url(idcu_HttpClientRequest* request, const char* url);
int  idcu_http_client_request_set_header(idcu_HttpClientRequest* request, const char* name, const char* value);
int  idcu_http_client_request_set_body(idcu_HttpClientRequest* request, const void* data, size_t length);
int  idcu_http_client_request_set_json(idcu_HttpClientRequest* request, const char* json);
int  idcu_http_client_request_set_form(idcu_HttpClientRequest* request, const char* form_data);
int  idcu_http_client_request_add_query_param(idcu_HttpClientRequest* request, const char* name, const char* value);
int  idcu_http_client_request_set_timeout(idcu_HttpClientRequest* request, int timeout_ms);
int  idcu_http_client_request_set_follow_redirects(idcu_HttpClientRequest* request, int follow, int max_redirects);
int  idcu_http_client_request_set_proxy(idcu_HttpClientRequest* request, const char* host, uint16_t port);
const char* idcu_http_client_method_to_string(idcu_HttpClientMethod method);

int  idcu_http_client_response_init(idcu_HttpClientResponse* response);
void idcu_http_client_response_destroy(idcu_HttpClientResponse* response);
int  idcu_http_client_response_get_header(const idcu_HttpClientResponse* response, const char* name, const char** value);
int  idcu_http_client_response_get_status_code(const idcu_HttpClientResponse* response);
const char* idcu_http_client_response_get_body(const idcu_HttpClientResponse* response);
size_t idcu_http_client_response_get_body_length(const idcu_HttpClientResponse* response);

int idcu_http_client_execute(idcu_HttpClient* client, const idcu_HttpClientRequest* request, idcu_HttpClientResponse* response);
int idcu_http_client_get(idcu_HttpClient* client, const char* url, idcu_HttpClientResponse* response);
int idcu_http_client_post(idcu_HttpClient* client, const char* url, const void* data, size_t length, idcu_HttpClientResponse* response);
int idcu_http_client_post_json(idcu_HttpClient* client, const char* url, const char* json, idcu_HttpClientResponse* response);
int idcu_http_client_put(idcu_HttpClient* client, const char* url, const void* data, size_t length, idcu_HttpClientResponse* response);
int idcu_http_client_delete(idcu_HttpClient* client, const char* url, idcu_HttpClientResponse* response);

int idcu_http_url_parse(const char* url, char* host, size_t host_size, uint16_t* port, char* path, size_t path_size, char* query, size_t query_size);
```

### 2.4 跨平台适配
- **网络层**: 基于 idcu-network 统一封装
- **Socket 操作**: 统一接口
- **超时处理**: select 模型跨平台

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以发送 GET 请求
- [ ] 可以发送 POST 请求
- [ ] 可以设置请求头
- [ ] 可以解析响应头
- [ ] 可以解析响应体
- [ ] 可以添加查询参数
- [ ] 可以跟随重定向
- [ ] 可以使用代理
- [ ] 超时设置生效
- [ ] 跨平台正常运行（Windows + Linux）

### 3.2 性能验收
- 请求延迟 ≤ 100ms（局域网）
- 支持 100+ 并发请求
- 吞吐量 ≥ 1000 QPS
- 内存占用 ≤ 10MB（100 并发）

### 3.3 异常验收
- [ ] 连接失败返回正确错误码
- [ ] 超时返回正确错误码
- [ ] 无效 URL 返回正确错误
- [ ] NULL 指针检查正确
- [ ] 超大响应正确处理

---

## 4. 执行计划

### 4.1 工期
4 小时/人

### 4.2 里程碑
- D1-00: 完成数据结构和头文件（45 分钟）
- D1-45: 完成 URL 解析（45 分钟）
- D1-90: 完成请求构建和发送（1 小时）
- D1-150: 完成响应解析（45 分钟）
- D1-195: 完成重定向和代理（30 分钟）
- D1-225: 完成单元测试（60 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + HTTP 协议知识）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，前缀 idcu_
- 枚举前缀 IDCU_HTTP_CLIENT_

### 5.2 测试要求
- 单元测试覆盖率 ≥ 75%
- 性能测试验证 QPS 和延迟
- 跨平台测试（Windows + Linux）

### 5.3 部署指引
- 编译命令：`cmake -B build &amp;&amp; cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::http-client)`

### 5.4 相关参考文档
- 详细工程化标准请参考：[工程化标准与CI/CD指南](../reference/engineering_standards.md)
- 性能与可靠性要求请参考：[性能指标与可靠性要求](../reference/performance_reliability.md)
- 模块依赖关系请参考：[模块依赖关系图](../reference/module_dependencies.md)
- 技术决策请参考：[技术决策记录](../reference/technical_decisions.md)

---

## 6. 风险与应对

### 6.1 风险1
描述：HTTP 解析存在边界情况  
应对：充分测试各种边界情况，添加模糊测试

### 6.2 风险2
描述：重定向循环导致无限递归  
应对：限制 max_redirects，检测循环

---

## 7. 详细实现步骤

### 1. 创建目录结构
```bash
mkdir -p libs/idcu-http-client/include/idcu/http
mkdir -p libs/idcu-http-client/src/idcu/http
mkdir -p libs/idcu-http-client/tests
mkdir -p libs/idcu-http-client/examples
```

### 2. 创建 HTTP 客户端头文件 (http_client.h)

### 3. 创建 CMakeLists.txt

### 4. 创建模块配置文件 (module.yaml)

### 5. 创建 README.md

---

## 8. 验证检查清单

- [ ] HTTP 客户端头文件已创建
- [ ] HTTP 客户端实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以发送 GET 请求
- [ ] 可以发送 POST 请求
- [ ] 可以解析响应
- [ ] 单元测试通过
- [ ] 跨平台测试通过
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-http-client/
git commit -m "feat: add idcu-http-client library

- Add HTTP/1.1 protocol support
- Add GET/POST/PUT/DELETE/HEAD/OPTIONS/PATCH methods
- Add HTTP header support
- Add query parameter support
- Add request body support (form, JSON)
- Add response parsing
- Add timeout settings
- Add follow redirects support
- Add proxy support
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 连接超时 | 网络问题或服务器无响应 | 增加超时时间或检查网络 |
| SSL 错误 | 未配置 SSL | 使用 HTTPS 时需要 SSL 支持 |
| 重定向循环 | max_redirects 太小 | 增加 max_redirects 值 |
| 代理连接失败 | 代理配置错误 | 检查代理主机和端口 |
