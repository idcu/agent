#ifndef IDCU_MANAGEMENT_HTTP_API_H
#define IDCU_MANAGEMENT_HTTP_API_H

#include "idcu/common/error_code.h"
#include "idcu/http_server/http_server.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    idcu_HttpServer server;
    int running;
    char address[256];
    uint16_t port;
} idcu_HttpManagementServer;

// 初始化 HTTP 运维服务器
int idcu_http_management_init(idcu_HttpManagementServer* server, const char* address, uint16_t port);

// 启动 HTTP 运维服务器
int idcu_http_management_start(idcu_HttpManagementServer* server);

// 停止 HTTP 运维服务器
int idcu_http_management_stop(idcu_HttpManagementServer* server);

// 销毁 HTTP 运维服务器
void idcu_http_management_destroy(idcu_HttpManagementServer* server);

// 轮询处理请求
int idcu_http_management_poll(idcu_HttpManagementServer* server, int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif // IDCU_MANAGEMENT_HTTP_API_H
