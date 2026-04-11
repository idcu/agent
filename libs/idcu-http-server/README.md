# idcu-http-server

IDCU Agent 的 HTTP 服务器库。

## 功能特性
- 轻量级 HTTP 服务器
- 路由系统
- 请求和响应处理
- 中间件支持
- 静态文件服务
- 线程安全实现

## 使用方法

```c
#include <idcu/http-server/http-server.h>

idcu_HttpServer* server;
int ret = idcu_http_server_init(&server);
if (ret == IDCU_ERR_OK) {
    // 添加路由
    idcu_http_server_add_route(server, "GET", "/", handle_root);
    idcu_http_server_add_route(server, "GET", "/api", handle_api);
    
    // 启动服务器
    idcu_http_server_start(server, "0.0.0.0", 8080);
    
    // 运行
    idcu_http_server_run(server);
    
    // 停止
    idcu_http_server_stop(server);
    idcu_http_server_destroy(server);
}
```

## API 参考
- `idcu_http_server_init()` - 初始化 HTTP 服务器
- `idcu_http_server_destroy()` - 销毁 HTTP 服务器
- `idcu_http_server_is_initialized()` - 检查是否已初始化
- `idcu_http_server_add_route()` - 添加路由
- `idcu_http_server_start()` - 启动服务器
- `idcu_http_server_stop()` - 停止服务器
- `idcu_http_server_run()` - 运行服务器事件循环

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
