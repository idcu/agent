# 消息通信与网络示例

## 功能说明

本目录包含IDCU Agent 的消息通信和网络交互相关示例，演示如何使用消息总线实现模块间通信，以及如何使用HTTP服务端和客户端进行网络交互。

## 包含示例

### 1. 消息总线通信示例

文件：`msgbus_communication_example.c`

演示如何使用IDCU的消息总线功能实现模块间的通信，包括点对点消息发送、广播消息、消息接收和处理等功能。

**主要功能：**
- 点对点消息发送
- 广播消息
- 消息接收和处理
- 消息优先级管理
- 通信统计

### 2. HTTP网络交互示例

文件：`http_network_example.c`

演示如何使用IDCU的HTTP服务端和客户端库进行网络交互。

**主要功能：**
- HTTP服务端创建和配置
- 多路由注册和处理
- JSON请求/响应处理
- HTTP客户端GET/POST请求
- 自定义请求头

## 使用说明

### 消息总线通信示例

为了演示模块间通信，建议创建两个实例：
1. **sender_module**: 负责发送消息
2. **receiver_module**: 负责接收消息

在 `config/agent.cfg` 中启用两个模块：

```ini
[modules]
business = ..., msgbus_sender, msgbus_receiver

enable_msgbus_sender = true
msgbus_sender.priority = normal

enable_msgbus_receiver = true
msgbus_receiver.priority = normal
```

### HTTP网络交互示例

1. 启动HTTP服务端：
```bash
# Windows
http_network_example.exe server

# Linux
./http_network_example server
```

2. 运行HTTP客户端（在另一个终端）：
```bash
# Windows
http_network_example.exe client

# Linux
./http_network_example client
```

## 依赖关系

- `idcu-core-sdk`: SDK核心功能
- `idcu-msgbus`: 消息总线库
- `idcu-http-server`: HTTP服务端库
- `idcu-http-client`: HTTP客户端库
- `idcu-json`: JSON解析和序列化
- `idcu-log`: 日志系统
