# 任务 3.9: idcu-network - 网络层库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建跨平台网络层库，提供 TCP 客户端/服务器、UDP 通信、Socket 管理、地址解析、超时设置、非阻塞 I/O、Windows/Linux/macOS 跨平台支持，满足 TCP 连接延迟 ≤ 100ms、数据传输吞吐量 ≥ 100MB/s、支持 1000+ 并发连接的性能要求。

### 1.2 不做什么
- 不实现 HTTP/HTTPS 协议栈（由上层模块处理）
- 不实现 TLS/SSL 加密
- 不实现 WebSocket
- 不实现高级网络拓扑（负载均衡等）

### 1.3 输入
- IP 地址/主机名和端口
- 要发送的数据
- 超时设置（毫秒）
- Socket 选项

### 1.4 输出
- 接收的数据
- Socket 连接状态
- 错误码
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **Socket API**: 封装 WinSock2（Windows）和 POSIX socket（Linux/macOS）
- **地址解析**: getaddrinfo（跨平台）
- **非阻塞 I/O**: fcntl（Linux）/ ioctlsocket（Windows）
- **超时设置**: setsockopt（SO_RCVTIMEO/SO_SNDTIMEO）或 select
- **Socket 集合**: select 模型

### 2.2 核心逻辑
```
网络初始化：
1. Windows：初始化 Winsock2
2. Linux/macOS：无需特殊初始化

TCP 连接流程：
1. 创建 Socket
2. 解析地址
3. 设置非阻塞/超时
4. 连接
5. 处理连接结果

数据发送：
1. 检查连接状态
2. 循环发送直到全部数据送出
3. 返回发送的字节数

数据接收：
1. 检查连接状态
2. 接收数据
3. 返回接收的字节数

TCP 服务器流程：
1. 创建监听 Socket
2. 绑定地址
3. 监听
4. 接受连接
5. 返回客户端 Socket
```

### 2.3 数据结构/接口
```c
// 主要头文件：idcu/network/network.h

#ifdef _WIN32
    typedef SOCKET idcu_Socket;
    #define IDCU_INVALID_SOCKET INVALID_SOCKET
#else
    typedef int idcu_Socket;
    #define IDCU_INVALID_SOCKET (-1)
#endif

typedef enum {
    IDCU_NET_PROTO_TCP = 0,
    IDCU_NET_PROTO_UDP
} idcu_NetProtocol;

typedef enum {
    IDCU_NET_ADDR_IPV4 = 0,
    IDCU_NET_ADDR_IPV6
} idcu_NetAddrType;

typedef struct {
    idcu_NetAddrType type;
    union {
        struct in_addr  v4;
        struct in6_addr v6;
    } addr;
    uint16_t port;
} idcu_NetAddress;

typedef struct {
    idcu_Socket socket;
    idcu_NetProtocol protocol;
    idcu_NetAddress local_addr;
    idcu_NetAddress remote_addr;
    int is_connected;
    int is_blocking;
    int timeout_ms;
} idcu_TcpSocket;

typedef struct {
    idcu_Socket socket;
    idcu_NetAddress local_addr;
    int is_bound;
    int timeout_ms;
} idcu_UdpSocket;

typedef struct {
    idcu_Socket listen_socket;
    idcu_NetAddress listen_addr;
    int is_listening;
    int backlog;
} idcu_TcpServer;

// 核心 API
int  idcu_network_init(void);
void idcu_network_cleanup(void);

// 地址操作
int idcu_net_address_init(idcu_NetAddress* addr, idcu_NetAddrType type, const char* ip, uint16_t port);
int idcu_net_address_resolve(idcu_NetAddress* addr, const char* hostname, uint16_t port);

// TCP Socket
int  idcu_tcp_socket_init(idcu_TcpSocket* sock);
void idcu_tcp_socket_destroy(idcu_TcpSocket* sock);
int  idcu_tcp_socket_connect(idcu_TcpSocket* sock, const idcu_NetAddress* addr);
int  idcu_tcp_socket_send_all(idcu_TcpSocket* sock, const void* data, size_t len);
int  idcu_tcp_socket_recv(idcu_TcpSocket* sock, void* buffer, size_t len, size_t* received);
void idcu_tcp_socket_disconnect(idcu_TcpSocket* sock);

// TCP 服务器
int  idcu_tcp_server_init(idcu_TcpServer* server);
void idcu_tcp_server_destroy(idcu_TcpServer* server);
int  idcu_tcp_server_listen(idcu_TcpServer* server, const idcu_NetAddress* addr, int backlog);
int  idcu_tcp_server_accept(idcu_TcpServer* server, idcu_TcpSocket* client);

// UDP Socket
int  idcu_udp_socket_init(idcu_UdpSocket* sock);
void idcu_udp_socket_destroy(idcu_UdpSocket* sock);
int  idcu_udp_socket_bind(idcu_UdpSocket* sock, const idcu_NetAddress* addr);
int  idcu_udp_socket_send(idcu_UdpSocket* sock, const idcu_NetAddress* dest, const void* data, size_t len);
int  idcu_udp_socket_recv(idcu_UdpSocket* sock, idcu_NetAddress* source, void* buffer, size_t len, size_t* received);
```

### 2.4 跨平台适配
- **Windows**: 使用 WinSock2，需链接 ws2_32.lib
- **Linux/macOS**: 使用 POSIX socket
- **统一 API**: 封装差异，提供一致接口

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] TCP 客户端可以连接和发送数据
- [ ] TCP 服务器可以监听和接受连接
- [ ] UDP Socket 可以发送和接收数据
- [ ] 地址解析（域名 → IP）正常工作
- [ ] 超时设置生效
- [ ] 非阻塞 I/O 正常工作
- [ ] select 模型支持多路复用
- [ ] 跨平台正常运行（Windows + Linux）

### 3.2 性能验收
- TCP 连接延迟 ≤ 100ms（局域网）
- 数据传输吞吐量 ≥ 100MB/s（千兆网络）
- 支持 1000+ 并发连接
- 内存占用 ≤ 10MB（1000 连接）

### 3.3 异常验收
- [ ] 连接失败返回明确错误码
- [ ] 超时返回明确错误码
- [ ] 网络断开正确检测
- [ ] NULL 指针检查正确

---

## 4. 执行计划

### 4.1 工期
5 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和跨平台封装（1 小时）
- D1-60: 完成 TCP 客户端（1 小时）
- D1-120: 完成 TCP 服务器（1 小时）
- D1-180: 完成 UDP 和地址解析（1 小时）
- D1-240: 完成 select 和单元测试（30 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 网络编程 + 跨平台）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，前缀 idcu_

### 5.2 测试要求
- 单元测试覆盖率 ≥ 75%
- 跨平台测试（Windows + Linux）

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::network)`
- Windows 需链接 ws2_32

---

## 6. 风险与应对

### 6.1 风险1
描述：跨平台行为差异导致兼容性问题  
应对：持续集成测试覆盖 Windows 和 Linux

### 6.2 风险2
描述：高并发时性能下降  
应对：提供 epoll（Linux）/ IOCP（Windows）选项

---

## 7. 详细实现步骤

### 1. 创建目录结构
```bash
mkdir -p libs/idcu-network/include/idcu/network
mkdir -p libs/idcu-network/src/idcu/network
mkdir -p libs/idcu-network/tests
mkdir -p libs/idcu-network/examples
```

### 2. 创建头文件和实现
- network.h: 头文件定义
- tcp.c: TCP 实现
- udp.c: UDP 实现
- address.c: 地址解析
- select.c: Socket 集合

### 3. 创建 CMakeLists.txt 和 module.yaml

### 4. 创建 README.md

---

## 8. 验证检查清单

- [ ] 网络头文件已创建
- [ ] 网络实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 已创建
- [ ] README.md 已创建
- [ ] TCP 客户端测试通过
- [ ] TCP 服务器测试通过
- [ ] UDP 测试通过
- [ ] 跨平台测试通过
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-network/
git commit -m "feat: add idcu-network library

- Add TCP client and server socket
- Add UDP socket support
- Add cross-platform (Windows/Linux/macOS) support
- Add address resolution and manipulation
- Add non-blocking I/O support
- Add timeout settings
- Add socket set and select operations
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 连接失败 | 地址或端口错误 | 检查服务器地址和端口 |
| 发送超时 | 网络拥堵 | 增加超时时间或检查网络 |
| Windows 初始化失败 | 未调用 idcu_network_init | 确保首先初始化网络库 |
