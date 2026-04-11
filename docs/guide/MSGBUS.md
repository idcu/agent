# 消息总线设计

本文档描述了 IDCU Agent 消息总线的设计。

## 概述

消息总线为模块间通信提供了一个发布-订阅通信系统。

## 核心概念

### 主题

消息按主题组织。主题是 32 位整数：

```c
typedef uint32_t idcu_MsgTopic;
```

主题约定：
- 使用高位表示类别
- 使用低位表示特定消息类型

示例主题定义：

```c
#define TOPIC_SYSTEM_BASE     0x00010000
#define TOPIC_SYSTEM_STARTUP  (TOPIC_SYSTEM_BASE | 0x0001)
#define TOPIC_SYSTEM_SHUTDOWN (TOPIC_SYSTEM_BASE | 0x0002)

#define TOPIC_METRICS_BASE    0x00020000
#define TOPIC_METRICS_UPDATE  (TOPIC_METRICS_BASE | 0x0001)

#define TOPIC_LOG_BASE        0x00030000
#define TOPIC_LOG_ENTRY       (TOPIC_LOG_BASE | 0x0001)
```

### 消息优先级

消息可以有优先级级别：

```c
typedef enum {
    IDCU_MSG_PRIORITY_LOW,      // 低优先级（批处理）
    IDCU_MSG_PRIORITY_NORMAL,    // 正常优先级（默认）
    IDCU_MSG_PRIORITY_HIGH,      // 高优先级（时间敏感）
    IDCU_MSG_PRIORITY_CRITICAL   // 关键优先级（紧急）
} idcu_MsgPriority;
```

更高优先级的消息在较低优先级消息之前被传递。

### 消息处理器

```c
typedef void (*idcu_MsgHandler)(idcu_MsgTopic topic,
                                 const void* data,
                                 size_t data_size,
                                 void* user_data);
```

## 消息总线 API

### 初始化

```c
// 创建消息总线
int idcu_msgbus_init(idcu_MsgBus** out_bus);

// 销毁消息总线
void idcu_msgbus_destroy(idcu_MsgBus* bus);
```

### 发布

```c
// 发布一条消息
int idcu_msgbus_publish(idcu_MsgBus* bus,
                         idcu_MsgTopic topic,
                         const void* data,
                         size_t data_size,
                         idcu_MsgPriority priority);
```

### 订阅

```c
// 订阅一个主题
int idcu_msgbus_subscribe(idcu_MsgBus* bus,
                           idcu_MsgTopic topic,
                           idcu_MsgHandler handler,
                           void* user_data,
                           idcu_MsgSubscriber** out_subscriber);

// 使用模式匹配订阅
int idcu_msgbus_subscribe_pattern(idcu_MsgBus* bus,
                                    const char* pattern,
                                    idcu_MsgHandler handler,
                                    void* user_data,
                                    idcu_MsgSubscriber** out_subscriber);

// 取消订阅
int idcu_msgbus_unsubscribe(idcu_MsgBus* bus,
                             idcu_MsgSubscriber* subscriber);
```

### 消息处理

```c
// 处理所有待处理消息
int idcu_msgbus_process(idcu_MsgBus* bus);

// 处理一条消息
int idcu_msgbus_process_one(idcu_MsgBus* bus);

// 获取待处理消息数量
int idcu_msgbus_get_pending_count(idcu_MsgBus* bus);

// 清除所有消息
void idcu_msgbus_clear(idcu_MsgBus* bus);
```

## 架构

### 消息流程

```
发布者
    ↓
[消息队列（按优先级排序）]
    ↓
[分发]
    ↓
订阅者 1 ── 处理器 1
    ↓
订阅者 2 ── 处理器 2
    ↓
    ...
```

### 线程安全

消息总线是线程安全的：
- 多个线程可以并发发布
- 多个线程可以并发订阅
- 消息分发是单线程的（必须调用 process()）

## 使用模式

### 基本发布-订阅

```c
#include <idcu/msgbus/msgbus.h>

#define TOPIC_EXAMPLE 0x1000

void my_handler(idcu_MsgTopic topic, const void* data,
                size_t data_size, void* user_data) {
    printf("在主题 %u 上收到消息\n", topic);
    if (data && data_size > 0) {
        printf("数据：%s\n", (const char*)data);
    }
}

int main(void) {
    idcu_MsgBus* bus = NULL;
    idcu_msgbus_init(&bus);

    // 订阅
    idcu_MsgSubscriber* sub = NULL;
    idcu_msgbus_subscribe(bus, TOPIC_EXAMPLE, my_handler, NULL, &sub);

    // 发布
    const char* msg = "你好，总线！";
    idcu_msgbus_publish(bus, TOPIC_EXAMPLE, msg, strlen(msg) + 1,
                         IDCU_MSG_PRIORITY_NORMAL);

    // 处理
    idcu_msgbus_process(bus);

    // 清理
    idcu_msgbus_unsubscribe(bus, sub);
    idcu_msgbus_destroy(bus);
    return 0;
}
```

### 请求-响应模式

```c
typedef struct {
    idcu_MsgTopic reply_topic;
    int request_id;
} RequestHeader;

typedef struct {
    int request_id;
    int result;
} ResponseHeader;

#define TOPIC_REQUEST  0x2000
#define TOPIC_RESPONSE 0x2001

void request_handler(idcu_MsgTopic topic, const void* data,
                     size_t data_size, void* user_data) {
    idcu_MsgBus* bus = (idcu_MsgBus*)user_data;
    const RequestHeader* header = (const RequestHeader*)data;

    // 处理请求...
    ResponseHeader resp = {
        .request_id = header->request_id,
        .result = 42
    };

    // 发送响应
    idcu_msgbus_publish(bus, header->reply_topic, &resp,
                         sizeof(resp), IDCU_MSG_PRIORITY_HIGH);
}
```

### 模式匹配

模式匹配允许订阅多个主题：

```c
// 订阅 0x1000-0x1FFF 范围内的所有主题
idcu_msgbus_subscribe_pattern(bus, "0x1*", handler, NULL, &sub);
```

## 最佳实践

1. **定义清晰的主题约定** - 记录主题范围和含义
2. **保持消息小** - 避免大的有效载荷
3. **适当使用优先级** - 为紧急情况保留 CRITICAL
4. **定期处理** - 从主循环调用 process()
5. **处理 NULL 数据** - 检查 data 和 data_size
6. **不要在处理器中阻塞** - 保持处理器快速和异步
7. **使用 user_data 传递上下文** - 传递必要的状态
