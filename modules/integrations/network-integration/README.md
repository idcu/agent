# network-integration

网络集成模块，统一管理所有网络功能。

## 配置

使用 YAML 配置（默认）：

```yaml
network:
  http_server:
    enabled: true
    host: "0.0.0.0"
    port: 8080
  connection_pool:
    max_connections: 100
```

## 功能

- 统一的网络接口
- 连接池集成
- HTTP 客户端/服务器集成
- 消息总线集成
- 节点发现集成
- 网络监控和健康检查

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
