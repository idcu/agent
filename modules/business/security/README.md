# Security Module

## 概述
安全业务模块，提供 TLS/SSL 加密通信支持。

## 功能特性
- TLS/SSL 加密通信
- 支持 TLS 1.0-1.3
- 证书验证
- 默认 TLS 上下文管理

## 使用示例

```c
#include "module_def.h"
#include "tls.h"

// 使用默认 TLS 上下文
idcu_TLSContext* ctx = idcu_security_module_get_default_tls_context();

// 创建 TLS 连接
idcu_TLSConnection* conn = NULL;
idcu_security_module_create_tls_connection(&conn, ctx, socket_fd);

// 执行握手
idcu_security_module_tls_handshake(conn);

// 发送/接收数据
idcu_security_module_tls_write(conn, data, size, &written);
idcu_security_module_tls_read(conn, buffer, size, &read);
```
