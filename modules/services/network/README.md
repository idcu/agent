# Network 服务模块

## 功能说明

网络服务模块，提供完整的网络和分布式功能：
- **网络层** - TCP/UDP Socket 封装，网络连接管理，数据收发接口
- **连接池** - 连接复用管理，连接生命周期管理，连接健康检查
- **HTTP 服务器** - 轻量级 HTTP 服务实现，请求路由，响应处理
- **管理 API** - RESTful 管理接口，模块管理 API，状态查询 API
- **分布式节点** - 多节点通信，节点状态同步，数据同步机制
- **节点发现** - 节点自动发现，节点广播，发现协议实现

## 目录结构

```
network/
├── include/              # 头文件
│   ├── README.md
│   ├── connection_pool.h
│   ├── distributed_node.h
│   ├── http_server.h
│   ├── management_api.h
│   ├── network_layer.h
│   └── node_discovery.h
├── src/                  # 源代码
│   ├── connection_pool.c
│   ├── distributed_node.c
│   ├── http_server.c
│   ├── management_api.c
│   ├── network_layer.c
│   └── node_discovery.c
├── tests/                # 测试
│   ├── CMakeLists.txt
│   ├── test_distributed_node.c
│   ├── test_network_layer.c
│   └── test_node_discovery.c
├── CMakeLists.txt
├── README.md
└── module.json
```

## 依赖关系

- 基础公共库
- 工具库
- 模块系统

## 使用说明

业务模块可以通过 include 目录下的头文件使用网络服务的功能。
