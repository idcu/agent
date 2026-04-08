# 任务 3.13: idcu-msgbus - 消息总线库

## 目标

创建完整的消息总线库，支持：
- 发布/订阅模式
- 消息队列
- 主题订阅
- 消息过滤
- 异步处理
- 线程安全

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-msgbus/include/idcu/msgbus
mkdir -p libs/idcu-msgbus/src/idcu/msgbus
mkdir -p libs/idcu-msgbus/tests
mkdir -p libs/idcu-msgbus/examples
```

### 2. 创建消息总线头文件 (msg_bus.h)

创建 `libs/idcu-msgbus/include/idcu/msgbus/msg_bus.h`：

```c
#ifndef IDCU_MSGBUS_MSG_BUS_H
#define IDCU_MSGBUS_MSG_BUS_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/common/condition.h"
#include "idcu/common/thread.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_MessageId;
typedef uint32_t idcu_MessageType;

typedef struct
{
    idcu_MessageId id;
    idcu_MessageType type;
    char topic[256];
    void* data;
    size_t data_size;
    uint64_t timestamp;
    uint32_t priority;
} idcu_Message;

typedef void (*idcu_MessageHandler)(const idcu_Message* message, void* user_data);

typedef struct idcu_Subscription
{
    idcu_MessageHandler handler;
    void* user_data;
    char topic_filter[256];
    idcu_MessageType type_filter;
    int use_type_filter;
} idcu_Subscription;

typedef struct idcu_MessageQueue
{
    idcu_Vector messages;
    idcu_Mutex lock;
    idcu_Condition not_empty;
    idcu_Condition not_full;
    size_t capacity;
    int closed;
} idcu_MessageQueue;

typedef struct
{
    idcu_HashMap subscriptions;
    idcu_MessageQueue queue;
    idcu_Thread worker_thread;
    idcu_Mutex lock;
    int running;
    int initialized;
} idcu_MsgBus;

int  idcu_msgbus_init(idcu_MsgBus* bus, size_t queue_capacity);
void idcu_msgbus_destroy(idcu_MsgBus* bus);
int  idcu_msgbus_start(idcu_MsgBus* bus);
void idcu_msgbus_stop(idcu_MsgBus* bus);

int idcu_msgbus_subscribe(idcu_MsgBus* bus, const char* topic, idcu_MessageHandler handler, void* user_data);
int idcu_msgbus_subscribe_type(idcu_MsgBus* bus, idcu_MessageType type, idcu_MessageHandler handler, void* user_data);
int idcu_msgbus_unsubscribe(idcu_MsgBus* bus, const char* topic, idcu_MessageHandler handler);
int idcu_msgbus_unsubscribe_all(idcu_MsgBus* bus, const char* topic);

int idcu_msgbus_publish(idcu_MsgBus* bus, const idcu_Message* message);
int idcu_msgbus_publish_simple(idcu_MsgBus* bus, const char* topic, const void* data, size_t data_size);
int idcu_msgbus_publish_with_type(idcu_MsgBus* bus, idcu_MessageType type, const char* topic, const void* data, size_t data_size);

idcu_Message* idcu_msgbus_message_create(const char* topic, const void* data, size_t data_size);
idcu_Message* idcu_msgbus_message_create_with_type(idcu_MessageType type, const char* topic, const void* data, size_t data_size);
void idcu_msgbus_message_destroy(idcu_Message* message);
int  idcu_msgbus_message_copy(idcu_Message* dest, const idcu_Message* src);

int idcu_msgbus_set_priority(idcu_Message* message, uint32_t priority);

int  idcu_message_queue_init(idcu_MessageQueue* queue, size_t capacity);
void idcu_message_queue_destroy(idcu_MessageQueue* queue);
int  idcu_message_queue_push(idcu_MessageQueue* queue, const idcu_Message* message);
int  idcu_message_queue_pop(idcu_MessageQueue* queue, idcu_Message* message, int timeout_ms);
int  idcu_message_queue_try_pop(idcu_MessageQueue* queue, idcu_Message* message);
size_t idcu_message_queue_size(idcu_MessageQueue* queue);
int  idcu_message_queue_is_empty(idcu_MessageQueue* queue);
int  idcu_message_queue_is_full(idcu_MessageQueue* queue);
void idcu_message_queue_close(idcu_MessageQueue* queue);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-msgbus/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-msgbus VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-msgbus STATIC
    src/idcu/msgbus/msg_bus.c
)

target_include_directories(idcu-msgbus PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-msgbus PRIVATE
    idcu::common
    idcu::log
)

add_library(idcu::msgbus ALIAS idcu-msgbus)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-msgbus/module.yaml`：

```yaml
name: idcu-msgbus
version: 1.0.0
description: Message bus library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-msgbus

headers:
  - idcu/msgbus/msg_bus.h

features:
  - pubsub: Publish/Subscribe pattern
  - topics: Topic-based subscription
  - message_queue: Message queue with capacity
  - async: Asynchronous message processing
  - thread_safe: Thread-safe operations
  - priority: Message priority support
  - filtering: Message type and topic filtering

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-msgbus/README.md`：

```markdown
# idcu-msgbus

IDCU Agent 的消息总线库。

## 功能特性

- **发布/订阅模式**: 经典的 Pub/Sub 消息模式
- **主题订阅**: 基于主题的订阅机制
- **消息队列**: 带容量限制的消息队列
- **异步处理**: 异步消息处理
- **线程安全**: 多线程环境安全使用
- **优先级支持**: 消息优先级
- **消息过滤**: 支持类型和主题过滤

## 快速开始

### 初始化消息总线

```c
#include "idcu/msgbus/msg_bus.h"

idcu_MsgBus bus;
idcu_msgbus_init(&bus, 1024);
idcu_msgbus_start(&bus);
```

### 定义消息处理器

```c
void my_handler(const idcu_Message* message, void* user_data)
{
    printf("Received message on topic: %s\n", message->topic);
    printf("Data: %.*s\n", (int)message->data_size, (const char*)message->data);
}
```

### 订阅主题

```c
idcu_msgbus_subscribe(&bus, "my/topic", my_handler, NULL);
```

### 发布消息

```c
const char* data = "Hello, World!";
idcu_msgbus_publish_simple(&bus, "my/topic", data, strlen(data));
```

### 停止消息总线

```c
idcu_msgbus_stop(&bus);
idcu_msgbus_destroy(&bus);
```

### 使用消息队列

```c
idcu_MessageQueue queue;
idcu_message_queue_init(&queue, 100);

idcu_Message msg;
idcu_msgbus_message_copy(&msg, some_message);

idcu_message_queue_push(&queue, &msg);

idcu_Message received;
idcu_message_queue_pop(&queue, &received, 1000);

idcu_message_queue_destroy(&queue);
```

## 消息主题

消息主题支持简单的层级结构，例如：

```
system/status
module/log/error
user/event/login
```

## 消息类型

可以为消息指定类型，用于更精确的过滤：

```c
#define MSG_TYPE_LOG 1
#define MSG_TYPE_EVENT 2

idcu_msgbus_publish_with_type(&bus, MSG_TYPE_LOG, "log/info", data, size);
```

## API 文档

详见 [include/idcu/msgbus/msg_bus.h](include/idcu/msgbus/msg_bus.h)
```

## 验证检查清单

- [ ] 消息总线头文件已创建
- [ ] 消息总线实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以发布和订阅消息
- [ ] 消息队列正常工作
- [ ] 支持按类型和主题过滤

## Git 提交

```bash
git add libs/idcu-msgbus/
git commit -m "feat: add idcu-msgbus library

- Add Publish/Subscribe pattern
- Add topic-based subscription
- Add message queue with capacity
- Add asynchronous message processing
- Add thread-safe operations
- Add message priority support
- Add message type and topic filtering
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 消息丢失 | 队列满 | 增加队列容量或加快消费速度 |
| 处理器不调用 | 订阅主题不匹配 | 确保订阅和发布的主题一致 |
| 死锁 | 未正确使用锁 | 确保在回调中不再次获取锁 |
