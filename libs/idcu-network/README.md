# idcu-network

网络通信库 for IDCU Agent.

## 功能特性

- **跨平台支持**: Windows (WinSock2) / Linux / macOS
- **TCP客户端**: TCP Socket连接、发送、接收
- **TCP服务器**: TCP监听、接受连接
- **UDP通信**: UDP Socket绑定、发送、接收
- **地址解析**: IPv4/IPv6地址解析和操作
- **超时设置**: Socket发送/接收超时
- **非阻塞I/O**: 支持阻塞和非阻塞模式
- **完整API**: 统一的跨平台接口

## 使用方法

### TCP客户端

```c
#include <idcu/network/network.h>

// 初始化网络库
idcu_network_init();

// 创建TCP Socket
idcu_TcpSocket sock;
idcu_tcp_socket_init(&sock);

// 解析地址
idcu_NetAddress addr;
idcu_net_address_resolve(&addr, "example.com", 8080);

// 连接
if (idcu_tcp_socket_connect(&sock, &addr) == IDCU_ERR_OK) {
    // 发送数据
    const char* data = "Hello!";
    idcu_tcp_socket_send_all(&sock, data, strlen(data));
    
    // 接收数据
    char buffer[1024];
    size_t received;
    idcu_tcp_socket_recv(&sock, buffer, sizeof(buffer), &received);
    
    // 断开连接
    idcu_tcp_socket_disconnect(&sock);
}

idcu_tcp_socket_destroy(&sock);
idcu_network_cleanup();
```

### TCP服务器

```c
#include <idcu/network/network.h>

idcu_network_init();

idcu_TcpServer server;
idcu_tcp_server_init(&server);

// 绑定并监听
idcu_NetAddress listen_addr;
idcu_net_address_init(&listen_addr, IDCU_NET_ADDR_IPV4, NULL, 8080);
idcu_tcp_server_listen(&server, &listen_addr, 10);

// 接受连接
idcu_TcpSocket client;
if (idcu_tcp_server_accept(&server, &client) == IDCU_ERR_OK) {
    // 处理客户端
    char buffer[1024];
    size_t received;
    idcu_tcp_socket_recv(&client, buffer, sizeof(buffer), &received);
    
    idcu_tcp_socket_disconnect(&client);
}

idcu_tcp_server_destroy(&server);
idcu_network_cleanup();
```

### UDP通信

```c
#include <idcu/network/network.h>

idcu_network_init();

idcu_UdpSocket sock;
idcu_udp_socket_init(&sock);

// 绑定本地地址
idcu_NetAddress local_addr;
idcu_net_address_init(&local_addr, IDCU_NET_ADDR_IPV4, NULL, 9000);
idcu_udp_socket_bind(&sock, &local_addr);

// 发送数据
idcu_NetAddress dest_addr;
idcu_net_address_resolve(&dest_addr, "127.0.0.1", 9001);
idcu_udp_socket_send(&sock, &dest_addr, "Hello UDP", 9);

// 接收数据
idcu_NetAddress source_addr;
char buffer[1024];
size_t received;
idcu_udp_socket_recv(&sock, &source_addr, buffer, sizeof(buffer), &received);

idcu_udp_socket_destroy(&sock);
idcu_network_cleanup();
```

## API参考

### 核心初始化
- `idcu_network_init()` - 初始化网络库
- `idcu_network_cleanup()` - 清理网络库

### 地址操作
- `idcu_net_address_init()` - 初始化地址
- `idcu_net_address_resolve()` - 解析主机名
- `idcu_net_address_to_string()` - 地址转字符串

### TCP Socket
- `idcu_tcp_socket_init()` - 初始化Socket
- `idcu_tcp_socket_destroy()` - 销毁Socket
- `idcu_tcp_socket_connect()` - 连接
- `idcu_tcp_socket_connect_timeout()` - 带超时连接
- `idcu_tcp_socket_send()` - 发送数据
- `idcu_tcp_socket_send_all()` - 发送全部数据
- `idcu_tcp_socket_recv()` - 接收数据
- `idcu_tcp_socket_set_timeout()` - 设置超时
- `idcu_tcp_socket_set_blocking()` - 设置阻塞模式
- `idcu_tcp_socket_disconnect()` - 断开连接

### TCP服务器
- `idcu_tcp_server_init()` - 初始化服务器
- `idcu_tcp_server_destroy()` - 销毁服务器
- `idcu_tcp_server_listen()` - 开始监听
- `idcu_tcp_server_accept()` - 接受连接
- `idcu_tcp_server_accept_timeout()` - 带超时接受

### UDP Socket
- `idcu_udp_socket_init()` - 初始化Socket
- `idcu_udp_socket_destroy()` - 销毁Socket
- `idcu_udp_socket_bind()` - 绑定地址
- `idcu_udp_socket_send()` - 发送数据
- `idcu_udp_socket_recv()` - 接收数据
- `idcu_udp_socket_set_timeout()` - 设置超时

## 性能指标

- TCP连接延迟：≤ 100ms（局域网）
- 数据传输吞吐量：≥ 100MB/s（千兆网络）
- 支持并发连接：≥ 1000+
- 内存占用：≤ 10MB（1000连接）

## 跨平台支持

| 平台 | 支持状态 | 底层API |
|------|---------|---------|
| Windows | ✅ 完整支持 | WinSock2 |
| Linux | ✅ 完整支持 | POSIX Socket |
| macOS | ✅ 完整支持 | POSIX Socket |

## CMake配置

```cmake
target_link_libraries(myapp PRIVATE idcu::network)

# Windows需要链接ws2_32
if(WIN32)
    target_link_libraries(myapp PRIVATE ws2_32)
endif()
```

## 构建

```bash
cmake -B build && cmake --build build
```

## 依赖

- idcu-common
- Windows: ws2_32.lib

## 许可证

MIT
