# 任务 4.1.5: REST API 模块

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建 REST API 模块，提供完整的 HTTP API 功能，支持：
- HTTP API 端点管理
- 请求路由和参数验证
- 多种认证方式（Basic、Bearer、API Key）
- API 版本控制
- 请求/响应日志
- 限流控制
- CORS 支持
- API 响应时间 ≤ 100ms，支持并发 1000 QPS

### 1.2 不做什么
- 不实现 WebSocket 支持
- 不实现 GraphQL 支持
- 不实现 API 网关功能

### 1.3 输入
- idcu-http-server 独立库
- idcu-json 独立库
- SDK 基础
- YAML 配置文件

### 1.4 输出
- REST API 模块
- 可以注册和访问 API 端点
- 支持多种认证方式

### 1.5 前置依赖
- phase3 11_idcu_http_server.md 任务已完成
- phase3 02_idcu_json.md 任务已完成
- phase2 SDK 基础已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- HTTP 服务器：idcu-http-server
- JSON 处理：idcu-json
- 认证方式：Basic Auth、Bearer Token、API Key
- 限流算法：令牌桶算法
- 配置格式：YAML（默认）

### 2.2 核心逻辑
1. 创建 REST API 目录结构
2. 实现 API 端点管理（注册、查找、删除）
3. 实现请求路由分发
4. 实现多种认证方式
5. 实现限流控制
6. 实现 CORS 支持
7. 实现 OpenAPI 文档生成

### 2.3 数据结构/接口
```c
typedef struct {
    idcu_ApiEndpointId id;
    char path[512];
    idcu_HttpMethod method;
    idcu_ApiHandler handler;
    void* user_data;
    idcu_ApiAuthConfig auth;
    idcu_ApiRateLimitConfig rate_limit;
} idcu_ApiEndpoint;

typedef struct {
    idcu_HttpServer http_server;
    idcu_Vector endpoints;
    idcu_HashMap endpoints_by_path;
    idcu_Mutex lock;
} idcu_RestApi;
```

### 2.4 跨平台适配
- Windows/Linux 使用相同的 socket API
- 路径分隔符处理一致
- 编码使用 UTF-8

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以添加和访问 API 端点
- [ ] JSON 响应正常工作
- [ ] 多种认证方式可以正常工作
- [ ] 限流功能正常工作
- [ ] CORS 支持正常工作

### 3.2 性能验收
- [ ] API 响应时间 ≤ 100ms（P99）
- [ ] 支持并发 1000 QPS
- [ ] 内存占用 ≤ 1MB
- [ ] 连接建立时间 ≤ 10ms

### 3.3 异常验收
- [ ] 认证失败返回 401/403 状态码
- [ ] 限流触发返回 429 状态码
- [ ] 无效请求返回 400 状态码
- [ ] 服务器错误返回 500 状态码

---

## 4. 执行计划

### 4.1 工期
1-2 天/人

### 4.2 里程碑
- D1：完成基础 API 端点和路由
- D2：完成认证、限流、CORS
- D2：完成 OpenAPI 文档生成

### 4.3 人力
1 人（技能要求：C 语言 + HTTP 协议 + REST API）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目的 .clang-format 规范
- 函数名小写+下划线，结构体前缀 Api_
- 所有头文件使用 include guard

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试 5 种异常场景
- 性能测试达到 QPS 要求

### 5.3 部署指引
- 编译命令：`cmake --build build --target rest-api`
- 部署路径：`modules/rest-api/`
- 配置使用 YAML 格式

---

## 6. 风险与应对

### 6.1 风险1
描述：高并发下性能下降  
应对：使用连接池和异步处理

### 6.2 风险2
描述：安全漏洞（SQL 注入、XSS）  
应对：输入验证和输出编码

---

## 7. 详细实现步骤

### 7.1 创建目录结构

```bash
mkdir -p modules/rest-api/include/idcu/rest_api
mkdir -p modules/rest-api/src/idcu/rest_api
mkdir -p modules/rest-api/tests
mkdir -p modules/rest-api/examples
```

### 7.2 创建头文件和实现
（详细代码省略，请参考原文档）

### 7.3 创建 CMakeLists.txt、module.yaml 和 README
（详细内容省略，请参考原文档）

---

## 8. 验证检查清单

- [ ] REST API 头文件已创建
- [ ] REST API 实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以添加和访问 API 端点
- [ ] JSON 响应正常工作
- [ ] 认证可以正常工作

---

## 9. Git 提交

```bash
git add modules/rest-api/
git commit -m "feat: add rest-api module

- Add HTTP API endpoints
- Add request routing
- Add parameter validation
- Add authentication and authorization
- Add API versioning
- Add request/response logging
- Add rate limiting
- Add CORS support
- Add OpenAPI documentation generation"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 路由不匹配 | 路径错误 | 确保路径完全匹配 |
| 认证失败 | 凭据错误 | 检查认证凭据 |
| 限流触发 | 请求太多 | 减少请求频率或增加限流阈值 |
