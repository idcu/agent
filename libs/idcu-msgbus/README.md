# idcu-msgbus

IDCU 项目的消息总线库，提供多优先级消息传递、广播和零拷贝传输功能。

## 特性

- 多优先级消息队列（支持 4 个优先级：低、正常、高、实时）
- 点对点消息传递
- 广播消息
- 零拷贝消息传输
- 批量消息发送和接收
- 线程安全
- 跨平台支持（Windows、Linux、macOS）

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-msgbus REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::msgbus)
```

## API 文档

### 消息总线初始化和销毁

```c
#include <idcu/msgbus/msg_bus.h>

void idcu_msg_bus_init(idcu_MessageBus *bus);
void idcu_msg_bus_destroy(idcu_MessageBus *bus);
```

### 消息发送和接收

```c
int idcu_msg_send(idcu_MessageBus *bus, uint32_t src_mod, uint32_t dst_mod, 
                   idcu_MsgPriority prio, const idcu_StackContext *ctx);
int idcu_msg_recv(idcu_MessageBus *bus, uint32_t mod_id, idcu_Message *msg);
int idcu_msg_broadcast(idcu_MessageBus *bus, uint32_t src_mod, 
                        idcu_MsgPriority prio, const idcu_StackContext *ctx);
uint32_t idcu_msg_get_count(idcu_MessageBus *bus);
```

### 零拷贝消息

```c
int idcu_msg_send_zerocopy(idcu_MessageBus *bus, uint32_t src_mod, uint32_t dst_mod, 
                            idcu_MsgPriority prio, const uint8_t *data, uint32_t size);
int idcu_msg_recv_zerocopy(idcu_MessageBus *bus, uint32_t mod_id, idcu_Message *msg);
void idcu_msg_release_payload(idcu_MessageBus *bus, idcu_ZeroCopyPayload *payload);
```

### 批量消息

```c
int idcu_msg_send_batch(idcu_MessageBus *bus, idcu_MessageBatch *batch);
int idcu_msg_recv_batch(idcu_MessageBus *bus, uint32_t mod_id, 
                         idcu_MessageBatch *batch, uint32_t max_count);
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [基础消息示例](examples/example_msg_basic.c) - 演示基本的消息发送和接收
- [优先级消息示例](examples/example_msg_priority.c) - 演示多优先级消息调度

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

# 运行基础消息示例
./example_msg_basic

# 运行优先级消息示例
./example_msg_priority
```

## 许可证

详见项目根目录的 LICENSE 文件。
