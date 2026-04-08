# 任务 2.5: 消息总线

## 目标

创建高效的消息总线，为 IDCU Agent 提供模块间通信支持，包括：
- 消息发送和接收
- 消息队列管理
- 消息优先级
- 消息路由
- 消息持久化（可选）

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-msgbus/include/idcu/msgbus
mkdir -p libs/idcu-msgbus/src/idcu/msgbus
mkdir -p libs/idcu-msgbus/tests
mkdir -p libs/idcu-msgbus/examples
```

### 2. 创建消息总线头文件 (msgbus.h)

创建 `libs/idcu-msgbus/include/idcu/msgbus/msgbus.h`：

```c
#ifndef IDCU_MSGBUS_MSGBUS_H
#define IDCU_MSGBUS_MSGBUS_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_MSGBUS_MAX_TOPICS 64
#define IDCU_MSGBUS_MAX_MSG_SIZE 4096
#define IDCU_MSGBUS_MAX_QUEUE_SIZE 256

typedef enum {
    IDCU_MSG_PRIORITY_LOW = 0,
    IDCU_MSG_PRIORITY_NORMAL = 1,
    IDCU_MSG_PRIORITY_HIGH = 2,
    IDCU_MSG_PRIORITY_CRITICAL = 3
} idcu_MsgPriority;

typedef uint32_t idcu_MsgTopic;
typedef uint32_t idcu_MsgId;

typedef struct {
    idcu_MsgId id;
    idcu_MsgTopic topic;
    idcu_MsgPriority priority;
    uint8_t* data;
    size_t data_size;
    uint64_t timestamp;
    uint32_t sender_id;
} idcu_Message;

typedef struct idcu_MsgBus idcu_MsgBus;
typedef struct idcu_MsgSubscriber idcu_MsgSubscriber;

typedef void (*idcu_MsgHandler)(const idcu_Message* msg, void* user_data);

// 消息总线
int  idcu_msgbus_init(idcu_MsgBus** bus);
void idcu_msgbus_destroy(idcu_MsgBus* bus);

// 消息发布
int  idcu_msgbus_publish(idcu_MsgBus* bus, idcu_MsgTopic topic,
                         const void* data, size_t data_size,
                         idcu_MsgPriority priority);

// 消息订阅
int  idcu_msgbus_subscribe(idcu_MsgBus* bus, idcu_MsgTopic topic,
                           idcu_MsgHandler handler, void* user_data,
                           idcu_MsgSubscriber** subscriber);
int  idcu_msgbus_unsubscribe(idcu_MsgSubscriber* subscriber);

// 消息查询
bool idcu_msgbus_has_topic(idcu_MsgBus* bus, idcu_MsgTopic topic);
size_t idcu_msgbus_queue_size(idcu_MsgBus* bus, idcu_MsgTopic topic);

// 消息处理
int  idcu_msgbus_process(idcu_MsgBus* bus);
int  idcu_msgbus_process_all(idcu_MsgBus* bus);

// 消息创建和释放
idcu_Message* idcu_message_create(idcu_MsgTopic topic, const void* data,
                                  size_t data_size, idcu_MsgPriority priority);
void idcu_message_destroy(idcu_Message* msg);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-msgbus/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-msgbus C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-msgbus STATIC
    src/idcu/msgbus/msgbus.c
)

target_include_directories(idcu-msgbus PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-msgbus PUBLIC
    idcu::common
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

build:
  type: cmake
  targets:
    - idcu-msgbus

headers:
  - idcu/msgbus/msgbus.h

features:
  - message_publish: Message publishing with priority
  - message_subscribe: Topic-based message subscription
  - message_queue: Priority-based message queuing
  - message_routing: Flexible message routing

testing:
  enabled: true
  framework: internal

security:
  notes: |
    - Message size limits prevent DoS attacks
    - Topic validation prevents invalid topic access
    - Queue size limits prevent memory exhaustion
```

## 验证检查清单

- [ ] 头文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建（YAML 默认格式）
- [ ] README.md 已创建
- [ ] 代码可以成功编译
- [ ] 基本消息发布和订阅功能正常
- [ ] 消息优先级功能正常
- [ ] 安全检查清单已通过

## Git 提交

```bash
git add libs/idcu-msgbus/
git commit -m "feat: add idcu-msgbus library

- Add message publishing and subscription
- Add priority-based message queuing
- Add topic-based message routing
- Add CMake build configuration
- Add module.yaml metadata with YAML format"
```
