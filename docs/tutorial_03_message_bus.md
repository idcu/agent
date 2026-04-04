# 教程三：消息总线使用

消息总线是 IDCU Agent 的核心组件之一，它让模块之间可以高效地通信。本教程将教你如何使用消息总线实现模块间的数据交换。

---

## 学习目标

完成本教程后，你将学会：
- 消息总线的基本概念
- 如何发送和接收消息
- 如何使用广播消息
- 零拷贝消息的使用
- 优先级队列

---

## 一、消息总线概述

### 1.1 什么是消息总线？

消息总线就像一个"邮局"，模块可以：
- 给特定模块发送消息（一对一）
- 给所有模块广播消息（一对多）
- 接收发给自己的消息

### 1.2 核心特性

- **优先级队列**：支持 4 种优先级（低、普通、高、实时）
- **零拷贝传输**：大数据传输时避免内存拷贝
- **批量处理**：支持批量发送和接收
- **线程安全**：多模块并发访问安全

---

## 二、消息基础

### 2.1 消息优先级

```c
#include "scheduler/msg_bus.h"

// 四种优先级
idcu_MsgPriority prio;
prio = IDCU_MSG_PRIO_LOW;      // 低优先级 - 后台任务
prio = IDCU_MSG_PRIO_NORMAL;   // 普通优先级 - 默认
prio = IDCU_MSG_PRIO_HIGH;     // 高优先级 - 重要任务
prio = IDCU_MSG_PRIO_REALTIME; // 实时优先级 - 紧急任务
```

### 2.2 消息上下文 (StackContext)

消息数据存储在 `idcu_StackContext` 中：

```c
#include "scheduler/context.h"

idcu_StackContext ctx;

// 设置消息类型
ctx.type = 1;  // 自定义消息类型

// 存储整数
ctx.int_val = 42;

// 存储浮点数
ctx.float_val = 3.14f;

// 存储小数据（最多 64 字节）
strcpy(ctx.buf, "Hello, World!");
```

---

## 三、发送和接收消息

### 3.1 简单示例：发送者和接收者

让我们创建两个模块：一个发送消息，一个接收消息。

**sender_module.c - 消息发送者：**
```c
#include "module/module_def.h"
#include "scheduler/msg_bus.h"
#include "kernel/micro_kernel.h"
#include "utils/log.h"
#include <time.h>

static int sender_init(void)
{
    idcu_log_info("[sender] Sender module initialized");
    return 0;
}

static int sender_run(void)
{
    static time_t last_send = 0;
    time_t now = time(NULL);
    
    // 每 2 秒发送一条消息
    if (now - last_send >= 2) {
        idcu_StackContext ctx;
        ctx.type = 100;  // 自定义消息类型
        ctx.int_val = now;
        strcpy(ctx.buf, "Hello from sender!");
        
        // 获取消息总线
        idcu_MessageBus* bus = idcu_kernel_get_msg_bus();
        
        // 发送给 ID 为 2 的模块（假设 receiver 模块 ID 是 2）
        // 注意：实际使用时需要通过模块管理器获取正确的模块 ID
        idcu_msg_send(bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
        
        idcu_log_info("[sender] Sent message: %s", ctx.buf);
        last_send = now;
    }
    
    return 0;
}

static int sender_stop(void)
{
    idcu_log_info("[sender] Sender module stopped");
    return 0;
}

IDCU_REGISTER_MODULE(sender, sender_init, sender_run, sender_stop);
```

**receiver_module.c - 消息接收者：**
```c
#include "module/module_def.h"
#include "scheduler/msg_bus.h"
#include "kernel/micro_kernel.h"
#include "utils/log.h"

static int receiver_init(void)
{
    idcu_log_info("[receiver] Receiver module initialized");
    return 0;
}

static int receiver_run(void)
{
    idcu_MessageBus* bus = idcu_kernel_get_msg_bus();
    idcu_Message msg;
    
    // 尝试接收消息
    while (idcu_msg_recv(bus, 2, &msg) == 0) {
        // 处理接收到的消息
        if (msg.data.type == 100) {
            idcu_log_info("[receiver] Got message from %d: %s", 
                          msg.source_mod_id, msg.data.buf);
            idcu_log_info("[receiver] Integer value: %d", msg.data.int_val);
        }
        
        // 如果是零拷贝消息，需要释放 payload
        if (msg.payload) {
            idcu_msg_release_payload(bus, msg.payload);
        }
    }
    
    return 0;
}

static int receiver_stop(void)
{
    idcu_log_info("[receiver] Receiver module stopped");
    return 0;
}

IDCU_REGISTER_MODULE(receiver, receiver_init, receiver_run, receiver_stop);
```

---

## 四、广播消息

### 4.1 广播示例

广播消息会发送给所有订阅的模块：

```c
#include "scheduler/msg_bus.h"
#include "kernel/micro_kernel.h"

void broadcast_example(void)
{
    idcu_MessageBus* bus = idcu_kernel_get_msg_bus();
    
    idcu_StackContext ctx;
    ctx.type = 200;
    strcpy(ctx.buf, "Broadcast message!");
    
    // 广播消息给所有模块
    idcu_msg_broadcast(bus, 1, IDCU_MSG_PRIO_NORMAL, &ctx);
    
    idcu_log_info("Broadcast sent");
}
```

---

## 五、零拷贝消息

对于大数据（超过 64 字节），使用零拷贝消息更高效。

### 5.1 零拷贝发送

```c
#include "scheduler/msg_bus.h"
#include "kernel/micro_kernel.h"

void send_large_data(void)
{
    idcu_MessageBus* bus = idcu_kernel_get_msg_bus();
    
    // 假设有 1KB 的数据
    uint8_t large_data[1024];
    for (int i = 0; i < 1024; i++) {
        large_data[i] = (uint8_t)i;
    }
    
    // 发送零拷贝消息
    int ret = idcu_msg_send_zerocopy(
        bus, 
        1,              // 源模块 ID
        2,              // 目标模块 ID
        IDCU_MSG_PRIO_NORMAL,
        large_data,
        1024
    );
    
    if (ret == 0) {
        idcu_log_info("Zero-copy message sent");
    } else {
        idcu_log_error("Failed to send zero-copy message");
    }
}
```

### 5.2 零拷贝接收

```c
void receive_large_data(void)
{
    idcu_MessageBus* bus = idcu_kernel_get_msg_bus();
    idcu_Message msg;
    
    if (idcu_msg_recv_zerocopy(bus, 2, &msg) == 0) {
        if (msg.payload) {
            // 访问数据
            uint8_t* data = msg.payload->data;
            uint32_t size = msg.payload->size;
            
            idcu_log_info("Received %u bytes of data", size);
            
            // 处理数据...
            for (uint32_t i = 0; i < size && i < 10; i++) {
                idcu_log_debug("data[%u] = %u", i, data[i]);
            }
            
            // 重要：使用完后必须释放 payload！
            idcu_msg_release_payload(bus, msg.payload);
        }
    }
}
```

---

## 六、批量消息

### 6.1 批量发送

```c
#include "scheduler/msg_bus.h"

void send_batch_messages(void)
{
    idcu_MessageBus* bus = idcu_kernel_get_msg_bus();
    idcu_MessageBatch batch;
    batch.count = 0;
    
    // 准备多条消息
    for (int i = 0; i < 5; i++) {
        idcu_Message* msg = &batch.msgs[batch.count];
        msg->data.type = 300;
        msg->data.int_val = i;
        msg->source_mod_id = 1;
        msg->target_mod_id = 2;
        msg->priority = IDCU_MSG_PRIO_NORMAL;
        msg->payload = NULL;
        batch.count++;
    }
    
    // 批量发送
    idcu_msg_send_batch(bus, &batch);
    idcu_log_info("Sent %u messages in batch", batch.count);
}
```

### 6.2 批量接收

```c
void receive_batch_messages(void)
{
    idcu_MessageBus* bus = idcu_kernel_get_msg_bus();
    idcu_MessageBatch batch;
    
    // 批量接收（最多 10 条）
    int count = idcu_msg_recv_batch(bus, 2, &batch, 10);
    
    if (count > 0) {
        idcu_log_info("Received %d messages", count);
        
        for (uint32_t i = 0; i < batch.count; i++) {
            idcu_Message* msg = &batch.msgs[i];
            idcu_log_info("  Message %u: type=%d, int_val=%d", 
                          i, msg->data.type, msg->data.int_val);
            
            // 释放 payload（如果有）
            if (msg->payload) {
                idcu_msg_release_payload(bus, msg->payload);
            }
        }
    }
}
```

---

## 七、实用工具函数

### 7.1 获取队列消息数

```c
void check_queue_status(void)
{
    idcu_MessageBus* bus = idcu_kernel_get_msg_bus();
    uint32_t count = idcu_msg_get_count(bus);
    idcu_log_info("Total messages in queue: %u", count);
}
```

---

## 八、完整示例：Ping-Pong

让我们创建一个完整的 Ping-Pong 示例，两个模块互相发送消息。

**ping_module.c:**
```c
#include "module/module_def.h"
#include "scheduler/msg_bus.h"
#include "kernel/micro_kernel.h"
#include "utils/log.h"
#include <time.h>

static int ping_count = 0;
static time_t last_ping = 0;

static int ping_init(void)
{
    idcu_log_info("[ping] Ping module initialized");
    ping_count = 0;
    last_ping = time(NULL);
    return 0;
}

static int ping_run(void)
{
    idcu_MessageBus* bus = idcu_kernel_get_msg_bus();
    idcu_Message msg;
    
    // 检查是否收到 pong
    while (idcu_msg_recv(bus, 1, &msg) == 0) {
        if (msg.data.type == 101) {
            idcu_log_info("[ping] Got pong #%d", msg.data.int_val);
            
            if (msg.payload) {
                idcu_msg_release_payload(bus, msg.payload);
            }
        }
    }
    
    // 每秒发送一个 ping
    time_t now = time(NULL);
    if (now - last_ping >= 1) {
        idcu_StackContext ctx;
        ctx.type = 100;
        ctx.int_val = ++ping_count;
        
        idcu_msg_send(bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
        idcu_log_info("[ping] Sent ping #%d", ping_count);
        
        last_ping = now;
    }
    
    return 0;
}

static int ping_stop(void)
{
    idcu_log_info("[ping] Ping module stopped, total: %d", ping_count);
    return 0;
}

IDCU_REGISTER_MODULE(ping, ping_init, ping_run, ping_stop);
```

**pong_module.c:**
```c
#include "module/module_def.h"
#include "scheduler/msg_bus.h"
#include "kernel/micro_kernel.h"
#include "utils/log.h"

static int pong_count = 0;

static int pong_init(void)
{
    idcu_log_info("[pong] Pong module initialized");
    pong_count = 0;
    return 0;
}

static int pong_run(void)
{
    idcu_MessageBus* bus = idcu_kernel_get_msg_bus();
    idcu_Message msg;
    
    // 检查是否收到 ping
    while (idcu_msg_recv(bus, 2, &msg) == 0) {
        if (msg.data.type == 100) {
            idcu_log_info("[pong] Got ping #%d", msg.data.int_val);
            
            // 回复 pong
            idcu_StackContext ctx;
            ctx.type = 101;
            ctx.int_val = ++pong_count;
            
            idcu_msg_send(bus, 2, 1, IDCU_MSG_PRIO_NORMAL, &ctx);
            idcu_log_info("[pong] Sent pong #%d", pong_count);
            
            if (msg.payload) {
                idcu_msg_release_payload(bus, msg.payload);
            }
        }
    }
    
    return 0;
}

static int pong_stop(void)
{
    idcu_log_info("[pong] Pong module stopped, total: %d", pong_count);
    return 0;
}

IDCU_REGISTER_MODULE(pong, pong_init, pong_run, pong_stop);
```

---

## 九、最佳实践

### 9.1 消息设计

```c
// ✅ 好的做法：定义消息类型常量
#define MSG_TYPE_DATA 100
#define MSG_TYPE_COMMAND 101
#define MSG_TYPE_RESPONSE 102

// 使用枚举
typedef enum {
    MY_MSG_DATA = 100,
    MY_MSG_COMMAND,
    MY_MSG_RESPONSE
} MyMessageType;
```

### 9.2 错误处理

```c
void safe_send(void)
{
    idcu_MessageBus* bus = idcu_kernel_get_msg_bus();
    idcu_StackContext ctx;
    
    int ret = idcu_msg_send(bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
    if (ret != 0) {
        idcu_log_warn("Failed to send message, queue may be full");
    }
}
```

### 9.3 避免消息风暴

```c
// ✅ 限流发送
static int message_count = 0;
static time_t last_reset = 0;

void rate_limited_send(void)
{
    time_t now = time(NULL);
    
    // 每秒重置计数
    if (now - last_reset >= 1) {
        message_count = 0;
        last_reset = now;
    }
    
    // 限制每秒最多 10 条消息
    if (message_count < 10) {
        // 发送消息...
        message_count++;
    }
}
```

---

## 下一步

恭喜你完成了消息总线教程！接下来你可以：

- 阅读 [教程四：协程调度详解](./tutorial_04_coroutine.md)，学习如何使用协程
- 查看现有模块中如何使用消息总线
- 尝试实现更复杂的模块间通信场景

---

祝你编码愉快！💬
